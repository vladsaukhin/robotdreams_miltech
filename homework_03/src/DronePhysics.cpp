#include "DronePhysics.h"

#include <chrono>

DronePhysics::DronePhysics(IConfigLoader& confLoader)
  : m_configLoader(confLoader)
{
  Reset();
}

DronePhysics::~DronePhysics()
{
  Stop();
}

void DronePhysics::PostUpdater(DronePhysicsUpdater&& updater)
{
  std::scoped_lock lock(m_mutex);
  m_updaters.push_back(std::move(updater));
}

void DronePhysics::Reset()
{
  std::scoped_lock lock(m_mutex);
  m_telemetry.position = m_configLoader.GetConfig().startPos;
  m_telemetry.diraction = m_configLoader.GetConfig().initialDir;
  m_telemetry.speed = 0.0;

  m_telemetry.currentTarget = UNDEFINED_TARGET_ID;
  m_telemetry.targetDir = 0.0;
  m_telemetry.turnRemaining = 0.0;

  m_updaters.clear();
}

void DronePhysics::Start()
{
  if (m_worker.joinable()) {
    return;  // already started
  }

  m_started = true;
  m_worker = std::thread(&DronePhysics::runWorker, this);
}

void DronePhysics::Stop()
{
  if (!m_worker.joinable()) {
    return;
  }

  m_started = false;

  m_waitCv.notify_one();
  m_worker.join();
}

void DronePhysics::runWorker()
{
  while (m_started.load()) {
    processUpdater();

    std::unique_lock lock(m_mutex);
    m_waitCv.wait_for(
      lock, std::chrono::duration<double>(m_configLoader.GetConfig().physicsTimeStep), [this] { return !m_started.load(); });
  }
}

void DronePhysics::processUpdater()
{
  std::scoped_lock lock(m_mutex);
  if (!m_updaters.empty()) {
    auto updater = m_updaters.front();
    m_updaters.pop_front();
    updater(m_telemetry);
  }
}