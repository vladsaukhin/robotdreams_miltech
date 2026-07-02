
#include "loggers/JsonLogger.h"

#include <fstream>

#include <nlohmann/json.hpp>

void JsonLogger::RecordStep(const dlink::Telemetry& telemetry, const TargetFireParams& target, int droneStateIdx)
{
  SimStep step;
  step.pos = {telemetry.x, telemetry.y};
  step.direction = telemetry.dir;
  // step.stateName = drone.state ? drone.state->GetName() : "Unknown";
  step.state = droneStateIdx;
  step.targetIdx = target.idx;

  step.dropPoint = target.releasePoint;

  m_simSteps.push_back(std::move(step));
}

void JsonLogger::DumpLog(std::string_view dataFolderPath, size_t lastStepIdx)
{
  try {
    if (lastStepIdx > m_simSteps.size()) {
      throw std::runtime_error("lastStepIdx is greater than log size.");
    }

    const auto logPath = dataFolderPath.data() + std::string("/simulation.json");
    std::ofstream output(logPath);
    if (!output) {
      throw std::runtime_error("Failed to open simulation.json for writing");
    }

    nlohmann::json out;
    out["totalSteps"] = lastStepIdx;
    out["steps"] = nlohmann::json::array();
    for (const auto& logStep : m_simSteps) {
      nlohmann::json step;
      step["position"] = {{"x", logStep.pos.x}, {"y", logStep.pos.y}};
      step["direction"] = logStep.direction;
      step["state"] = logStep.state;
      step["targetIndex"] = logStep.targetIdx;
      step["dropPoint"] = {{"x", logStep.dropPoint.x}, {"y", logStep.dropPoint.y}};
      out["steps"].push_back(std::move(step));
    }

    output << out.dump(2);
  }
  catch (const std::exception& e) {
    std::cerr << "JsonLogger: " << e.what() << '\n';
  }
}

void JsonLogger::Reset()
{
  m_simSteps.clear();
}