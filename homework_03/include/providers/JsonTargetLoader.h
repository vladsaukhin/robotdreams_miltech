#pragma once

#include "Coords.hpp"

class JsonTargetLoader {
public:
  JsonTargetLoader(std::string_view dataFolderPath) { readTargets(dataFolderPath); }

  JsonTargetLoader(JsonTargetLoader&&) = default;
  JsonTargetLoader& operator=(JsonTargetLoader&&) = default;

  ~JsonTargetLoader() = default;

private:
  JsonTargetLoader(const JsonTargetLoader&) = delete;
  JsonTargetLoader& operator=(const JsonTargetLoader&) = delete;

private:
  void readTargets(std::string_view dataFolderPath);

public:
  size_t targetCount{};
  size_t targetTimeStepsCount{};
  std::vector<ListOfCoords> targetsInTime{};
};