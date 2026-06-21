#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateDecelerating.h"
#include "droneStates/StateMoving.h"

#include "Utils.h"

IDroneStatePtr StateAccelerating::Execute(DroneStateContext& ctx)
{
  // Update speed and position
  const double prevSpeed = ctx.drone.speed;
  ctx.drone.speed += ctx.GetSpeedAfterTime();
  ctx.MoveDrone(prevSpeed);
  m_stopTime = ctx.drone.speed / ctx.acceleration;

  // Select next state
  const double deltaAngle = std::fabs(AngleDiff(ctx.drone.diraction, ctx.drone.targetDir));
  if (std::fabs(deltaAngle) > ctx.cfg.turnThreshold) {
    return std::make_unique<StateDecelerating>();
  }

  if (ctx.drone.speed >= ctx.cfg.attackSpeed) {
    ctx.drone.speed = ctx.cfg.attackSpeed;
    return std::make_unique<StateMoving>();
  }

  return nullptr;
}
