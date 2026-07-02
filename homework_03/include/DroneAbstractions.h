#pragma once

#include "Coords.hpp"
#include "interfaces/IDroneState.h"

constexpr int UNDEFINED_TARGET_ID{-1};

struct Drone {
  Coord position{};
  double diraction{};

  IDroneStatePtr state;

  int currentTarget{UNDEFINED_TARGET_ID};
  double targetDir{};

  double speed{};
  double turnRemaining{};
};

struct Target {
  int idx{UNDEFINED_TARGET_ID};

  double totalTime{std::numeric_limits<double>::max()};
  Coord releasePoint{};

  Coord predictedPosition{};
  Coord aimPoint{};
};