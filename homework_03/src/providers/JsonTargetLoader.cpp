#include "providers/JsonTargetLoader.h"

#include <format>
#include <fstream>

#include <nlohmann/json.hpp>

void JsonTargetLoader::readTargets(std::string_view dataFolderPath)
{
  try {
    const auto targetsPath = dataFolderPath.data() + std::string("/targets.json");
    std::ifstream input(targetsPath);
    if (!input) {
      throw std::runtime_error(std::format("Can't open {}", targetsPath));
    }

    nlohmann::json j = nlohmann::json::parse(input);

    if (!j.is_object()) {
      throw std::runtime_error("Root must be an object");
    }

    targetCount = j.at("targetCount").get<size_t>();
    targetTimeStepsCount = j.at("timeSteps").get<size_t>();

    const auto& targets = j.at("targets");

    if (!targets.is_array()) {
      throw std::runtime_error("'targets' must be array");
    }

    if (targets.size() != targetCount) {
      throw std::runtime_error("targets.size != targetCount");
    }

    targetsInTime.reserve(targetCount);  // preallocate memory for targets

    ListOfCoords targetPositions;
    targetPositions.reserve(targetTimeStepsCount);

    for (size_t i = 0; i < targets.size(); ++i) {
      const auto& target = targets.at(i);

      // validate
      if (!target.is_object()) {
        throw std::runtime_error("target must be object");
      }

      const auto& positions = target.at("positions");

      if (!positions.is_array()) {
        throw std::runtime_error("positions must be array");
      }

      if (positions.size() != targetTimeStepsCount) {
        throw std::runtime_error("positions.size != timeSteps");
      }

      // read
      for (size_t k = 0; k < positions.size(); ++k) {
        const auto& position = positions.at(k);

        if (!position.is_object()) {
          throw std::runtime_error("position must be object");
        }

        const double x = position.at("x").get<double>();
        const double y = position.at("y").get<double>();

        if (x < 0 || y < 0) {
          throw std::runtime_error("Coords must be non-negative");
        }

        targetPositions.push_back({x, y});
      }

      targetsInTime.push_back(std::move(targetPositions));  // resets targetPositions
    }
  }
  catch (const std::exception& e) {
    throw std::runtime_error("JsonTargetLoader error: " + std::string(e.what()));
  }
}