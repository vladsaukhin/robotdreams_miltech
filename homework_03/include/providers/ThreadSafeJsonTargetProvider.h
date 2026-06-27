#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>

#include "interfaces/ITargetProvider.h"
#include "JsonTargetLoader.h"

struct ThreadSafeJsonTargetProviderConfig {
  std::string dataFolderPath;
  double arrayTimeStep;
  double targetTimeStep;
};

class ThreadSafeJsonTargetProvider : public ITargetProvider {
public:
  ThreadSafeJsonTargetProvider(ThreadSafeJsonTargetProviderConfig config)
    : m_targetLoader(config.dataFolderPath)
    , m_arrayTimeStep(config.arrayTimeStep)
    , m_targetTimeStep(config.targetTimeStep)
  {
    updateTargetsState();
  }

  ~ThreadSafeJsonTargetProvider();

private:
  ThreadSafeJsonTargetProvider(ThreadSafeJsonTargetProvider&&) = delete;
  ThreadSafeJsonTargetProvider& operator=(ThreadSafeJsonTargetProvider&&) = delete;

  ThreadSafeJsonTargetProvider(const ThreadSafeJsonTargetProvider&) = delete;
  ThreadSafeJsonTargetProvider& operator=(const ThreadSafeJsonTargetProvider&) = delete;

public:
  size_t GetTargetCount() const override { return m_targetLoader.targetCount; }

  void Start(std::optional<double>) override;
  void Stop() override;

  TargetState GetTargetState(size_t targetIdx) const override;

private:
  void runWorker();

  void updateTargetsState();

private:
  JsonTargetLoader m_targetLoader;
  double m_arrayTimeStep;
  double m_targetTimeStep;

  std::atomic_bool m_started{false};
  std::thread m_worker;

  mutable std::mutex m_mutex;
  std::condition_variable m_waitCv;
  std::vector<TargetState> m_targetsState{};
  double m_currentTime{};
};