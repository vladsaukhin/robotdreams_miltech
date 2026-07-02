#pragma once

#include "DroneAbstractions.h"
#include "interfaces/ILogger.h"

class JsonLogger : public ILogger {
public:
  JsonLogger() = default;

  JsonLogger(JsonLogger&&) = default;
  JsonLogger& operator=(JsonLogger&&) = default;

private:
  JsonLogger(const JsonLogger&) = delete;
  JsonLogger& operator=(const JsonLogger&) = delete;

public:
  void RecordStep(const dlink::Telemetry&, const TargetFireParams&, int droneStateIdx) override;

  void DumpLog(std::string_view dataFolderPath, size_t lastStepIdx) override;

  void Reset() override;

private:
  struct SimStep {
    Coord pos{};         // позиція дрона
    double direction{};  // напрямок (рад)
    // std::string stateName{};  // стан автомата (0-4) або повне ім'я стану
    int state{};              // стан автомата (0-4) або повне ім'я стану
    int targetIdx{};          // індекс поточної цілі
    Coord dropPoint{};        // точка скиду (куди летить дрон)
    Coord aimPoint{};         // куди впаде бомба (якщо скинути зараз)
    Coord predictedTarget{};  // прогнозована позиція цілі
  };

private:
  std::vector<SimStep> m_simSteps{};
};
