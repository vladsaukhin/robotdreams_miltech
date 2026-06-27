#pragma once

#include "Coords.hpp"

constexpr int UNDEFINED_TARGET_ID{-1};

struct Drone {
  Coord position{};
  double diraction{};

  int currentTarget{UNDEFINED_TARGET_ID};
  double targetDir{};

  double speed{};
  double turnRemaining{};
};

struct DroneTelemetry {
  Coord position{};
  double diraction{};
  double speed{};

  int currentTarget{UNDEFINED_TARGET_ID};
  double targetDir{};
  double turnRemaining{};
};

struct TargetFireParams {
  int idx{UNDEFINED_TARGET_ID};

  double totalTime{std::numeric_limits<double>::max()};
  Coord releasePoint{};

  Coord predictedPosition{};
  Coord aimPoint{};
};