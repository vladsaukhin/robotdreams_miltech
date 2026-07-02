#pragma once

#include "drone_link.h"

struct DroneStateContext {
  const dlink::DroneCfg& droneCfg;
  const dlink::Telemetry& telemetry;
  double targetAngle{};
  double acceleration{};

  double GetSpeedAfterTime() const { return acceleration * droneCfg.timeStep; };
};