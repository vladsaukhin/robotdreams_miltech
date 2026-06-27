#pragma once

#include "DroneAbstractions.h"
#include "DroneConfig.h"

struct DroneStateContext {
  DroneTelemetry& telemetry;
  const DroneConfig& cfg;
  double acceleration{};

  void MoveDrone(double prevSpeed)
  {
    const double dt = (prevSpeed + telemetry.speed) / 2.0 * cfg.simTimeStep;
    const Coord positionToAdd = {std::cos(telemetry.diraction) * dt, std::sin(telemetry.diraction) * dt};
    telemetry.position += positionToAdd;
  }

  double GetSpeedAfterTime() const { return acceleration * cfg.simTimeStep; };
};