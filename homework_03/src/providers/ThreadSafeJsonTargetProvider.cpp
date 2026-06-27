#include "providers/ThreadSafeJsonTargetProvider.h"

#include <chrono>

namespace {

Coord getInterpolatedTarget(const JsonTargetLoader& targetsLoader, size_t targetIdx, double time, double arrayTimeStep)
{
  const double samplePos = time / arrayTimeStep;
  const int rawIdx = static_cast<int>(std::floor(samplePos));
  const int idx = rawIdx % targetsLoader.targetTimeStepsCount;
  const int next = (idx + 1) % targetsLoader.targetTimeStepsCount;
  const double frac = samplePos - std::floor(samplePos);

  const auto& targetTimes = targetsLoader.targetsInTime.at(targetIdx);
  const double x = targetTimes[idx].x + (targetTimes[next].x - targetTimes[idx].x) * frac;
  const double y = targetTimes[idx].y + (targetTimes[next].y - targetTimes[idx].y) * frac;
  return {x, y};
}

Coord getTargetVelocity(const JsonTargetLoader& targetsLoader, size_t targetIdx, double time, double arrayTimeStep)
{
  const double dt = arrayTimeStep;
  const auto p0 = getInterpolatedTarget(targetsLoader, targetIdx, time, arrayTimeStep);
  const auto p1 = getInterpolatedTarget(targetsLoader, targetIdx, time + dt, arrayTimeStep);
  return {(p1.x - p0.x) / dt, (p1.y - p0.y) / dt};
}
}  // namespace

ThreadSafeJsonTargetProvider::~ThreadSafeJsonTargetProvider()
{
  Stop();
}

TargetState ThreadSafeJsonTargetProvider::GetTargetState(size_t targetIdx) const
{
  if (targetIdx >= m_targetLoader.targetCount) {
    throw std::out_of_range("ThreadSafeJsonTargetProvider::GetTargetState: targetIdx is out of range");
  };

  std::scoped_lock lock(m_mutex);
  return m_targetsState[targetIdx];
}

void ThreadSafeJsonTargetProvider::Start(std::optional<double>)
{
  if (m_worker.joinable()) {
    return;  // already started
  }

  m_started = true;
  m_worker = std::thread(&ThreadSafeJsonTargetProvider::runWorker, this);
}
void ThreadSafeJsonTargetProvider::Stop()
{
  if (!m_worker.joinable()) {
    return;
  }

  m_started = false;

  m_waitCv.notify_one();
  m_worker.join();
}

void ThreadSafeJsonTargetProvider::runWorker()
{
  while (m_started.load()) {
    updateTargetsState();

    std::unique_lock lock(m_mutex);
    m_waitCv.wait_for(lock, std::chrono::duration<double>(m_targetTimeStep), [this] { return !m_started.load(); });
  }
}

void ThreadSafeJsonTargetProvider::updateTargetsState()
{
  double currentTime{};
  {
    std::scoped_lock lock(m_mutex);
    currentTime = m_currentTime;
  }

  std::vector<TargetState> newStates;
  newStates.reserve(m_targetLoader.targetCount);
  for (size_t targetIdx = 0; targetIdx < m_targetLoader.targetCount; ++targetIdx) {
    newStates.push_back(TargetState{.position = getInterpolatedTarget(m_targetLoader, targetIdx, currentTime, m_arrayTimeStep),
                                    .velocity = getTargetVelocity(m_targetLoader, targetIdx, currentTime, m_arrayTimeStep)});
  }

  std::scoped_lock lock(m_mutex);
  m_targetsState = std::move(newStates);
  m_currentTime += m_targetTimeStep;
}