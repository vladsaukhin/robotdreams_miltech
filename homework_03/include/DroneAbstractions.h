#pragma once

#include "Coords.hpp"

constexpr int UNDEFINED_TARGET_ID{-1};

enum DroneState : uint8_t { STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING };

struct Drone {
  Coord position{};
  double diraction{};

  int currentTarget{UNDEFINED_TARGET_ID};
  double targetDir{};

  DroneState state{STOPPED};
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