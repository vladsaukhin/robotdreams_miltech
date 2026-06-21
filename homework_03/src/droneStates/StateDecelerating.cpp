#include "interfaces/DroneStateContext.h"

#include "droneStates/StateDecelerating.h"
#include "droneStates/StateStopped.h"

IDroneStatePtr StateDecelerating::Execute(DroneStateContext& ctx)
{
  // Update speed and position
  const double prevSpeed = ctx.drone.speed;
  ctx.drone.speed -= ctx.GetSpeedAfterTime();
  ctx.MoveDrone(prevSpeed);
  m_stopTime = ctx.drone.speed / ctx.acceleration;

  // Select next state
  if (ctx.drone.speed <= 0.01) {
    ctx.drone.speed = 0.0;
    return std::make_unique<StateStopped>();
  }

  return nullptr;
}
