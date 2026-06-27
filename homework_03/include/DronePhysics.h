#pragma once

#include <mutex>
#include <functional>
#include <deque>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "DroneAbstractions.h"
#include "interfaces/IConfigLoader.h"

using DronePhysicsUpdater = std::function<void(DroneTelemetry&)>;
class DronePhysics {
public:
  DronePhysics(IConfigLoader& configLoader);
  ~DronePhysics();

private:
  DronePhysics(const DronePhysics&) = delete;
  DronePhysics& operator=(const DronePhysics&) = delete;

  DronePhysics(DronePhysics&&) = delete;
  DronePhysics& operator=(DronePhysics&&) = delete;

public:
  void Start();
  void Stop();

  void PostUpdater(DronePhysicsUpdater&&);

  DroneTelemetry GetTelemetry() const
  {
    std::scoped_lock lock(m_mutex);
    return m_telemetry;
  }

  void Reset();

private:
  void runWorker();
  void processUpdater();

private:
  IConfigLoader& m_configLoader;

  std::atomic_bool m_started{false};
  std::thread m_worker;

  mutable std::mutex m_mutex;
  std::condition_variable m_waitCv;
  std::deque<DronePhysicsUpdater> m_updaters;
  DroneTelemetry m_telemetry{};
};