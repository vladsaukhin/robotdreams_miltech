#include "interfaces/DroneStateContext.h"

#include "droneStates/StateDecelerating.h"
#include "droneStates/StateStopped.h"

IDroneStatePtr StateDecelerating::Execute(DroneStateContext& ctx)
{
  // Update speed and position
  const double prevSpeed = ctx.telemetry.speed;
  ctx.telemetry.speed -= ctx.GetSpeedAfterTime();
  ctx.MoveDrone(prevSpeed);
  m_stopTime = ctx.telemetry.speed / ctx.acceleration;

  // Select next state
  if (ctx.telemetry.speed <= 0.01) {
    ctx.telemetry.speed = 0.0;
    return std::make_unique<StateStopped>();
  }

  return nullptr;
}
