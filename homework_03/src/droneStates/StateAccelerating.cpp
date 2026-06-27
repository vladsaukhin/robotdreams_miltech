#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateDecelerating.h"
#include "droneStates/StateMoving.h"

#include "Utils.h"

IDroneStatePtr StateAccelerating::Execute(DroneStateContext& ctx)
{
  // Update speed and position
  const double prevSpeed = ctx.telemetry.speed;
  ctx.telemetry.speed += ctx.GetSpeedAfterTime();
  ctx.MoveDrone(prevSpeed);
  m_stopTime = ctx.telemetry.speed / ctx.acceleration;

  // Select next state
  const double deltaAngle = std::fabs(AngleDiff(ctx.telemetry.diraction, ctx.telemetry.targetDir));
  if (std::fabs(deltaAngle) > ctx.cfg.turnThreshold) {
    return std::make_unique<StateDecelerating>();
  }

  if (ctx.telemetry.speed >= ctx.cfg.attackSpeed) {
    ctx.telemetry.speed = ctx.cfg.attackSpeed;
    return std::make_unique<StateMoving>();
  }

  return nullptr;
}
