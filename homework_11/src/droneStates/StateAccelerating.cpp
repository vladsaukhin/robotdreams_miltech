#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateDecelerating.h"
#include "droneStates/StateMoving.h"

#include "Utils.h"

IDroneStatePtr StateAccelerating::Execute(DroneStateContext& ctx)
{
  m_stopTime = ctx.telemetry.speed / ctx.acceleration;
  m_control.accel = ctx.GetSpeedAfterTime();

  // Select next state
  const double deltaAngle = std::fabs(AngleDiff(ctx.telemetry.dir, ctx.targetAngle));
  if (std::fabs(deltaAngle) > ctx.droneCfg.turnThreshold) {
    return std::make_unique<StateDecelerating>();
  }

  if (ctx.telemetry.speed >= ctx.droneCfg.attackSpeed) {
    return std::make_unique<StateMoving>();
  }

  return nullptr;
}
