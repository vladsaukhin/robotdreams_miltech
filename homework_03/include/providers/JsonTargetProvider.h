#pragma once

#include "interfaces/ITargetProvider.h"
#include "JsonTargetLoader.h"

struct JsonTargetProviderConfig {
  std::string dataFolderPath;
  double arrayTimeStep;
  double simTimeStep;
};

class JsonTargetProvider : public ITargetProvider {
public:
  JsonTargetProvider(JsonTargetProviderConfig config)
    : m_targetLoader(config.dataFolderPath)
    , m_arrayTimeStep(config.arrayTimeStep)
    , m_simTimeStep(config.simTimeStep)
  {
  }

private:
  JsonTargetProvider(JsonTargetProvider&&) = delete;
  JsonTargetProvider& operator=(JsonTargetProvider&&) = delete;

  JsonTargetProvider(const JsonTargetProvider&) = delete;
  JsonTargetProvider& operator=(const JsonTargetProvider&) = delete;

public:
  size_t GetTargetCount() const override { return m_targetLoader.targetCount; }

  void Start(std::optional<double> currentTime) override { m_time = currentTime.value_or(0.0); }
  void Stop() override {}

  TargetState GetTargetState(size_t targetIdx) const override;

private:
  JsonTargetLoader m_targetLoader;
  double m_arrayTimeStep;
  double m_simTimeStep;

  double m_time{};
};