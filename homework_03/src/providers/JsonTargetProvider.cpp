#include "providers/JsonTargetProvider.h"

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

Coord getTargetVelocity(const JsonTargetLoader& targetsLoader, size_t targetIdx, double time, double arrayTimeStep, double simTimeStep)
{
  const double dt = simTimeStep;
  const auto p0 = getInterpolatedTarget(targetsLoader, targetIdx, time, arrayTimeStep);
  const auto p1 = getInterpolatedTarget(targetsLoader, targetIdx, time + dt, arrayTimeStep);
  return {(p1.x - p0.x) / dt, (p1.y - p0.y) / dt};
}
}  // namespace

TargetState JsonTargetProvider::GetTargetState(size_t targetIdx) const
{
  if (targetIdx >= m_targetLoader.targetCount || targetIdx < 0) {
    throw std::out_of_range("JsonTargetProvider::GetTargetState: targetIdx is out of range");
  };

  TargetState state;
  state.position = getInterpolatedTarget(m_targetLoader, targetIdx, m_time, m_arrayTimeStep);
  state.velocity = getTargetVelocity(m_targetLoader, targetIdx, m_time, m_arrayTimeStep, m_simTimeStep);
  return state;
}