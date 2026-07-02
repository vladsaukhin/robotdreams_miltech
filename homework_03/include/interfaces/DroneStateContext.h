#pragma once

#include "DroneAbstractions.h"
#include "DroneConfig.h"

struct DroneStateContext {
  Drone& drone;
  const DroneConfig& cfg;
  double acceleration{};

  void MoveDrone(double prevSpeed)
  {
    const double dt = (prevSpeed + drone.speed) / 2.0 * cfg.simTimeStep;
    const Coord positionToAdd = {std::cos(drone.diraction) * dt, std::sin(drone.diraction) * dt};
    drone.position += positionToAdd;
  }

  double GetSpeedAfterTime() const { return acceleration * cfg.simTimeStep; };
};