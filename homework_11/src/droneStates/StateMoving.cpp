#include "interfaces/DroneStateContext.h"

#include "droneStates/StateDecelerating.h"
#include "droneStates/StateMoving.h"

#include "Utils.h"

IDroneStatePtr StateMoving::Execute(DroneStateContext& ctx)
{
  m_stopTime = ctx.telemetry.speed / ctx.acceleration;
  m_control.accel = 0.0f;

  const double deltaAngle = std::fabs(AngleDiff(ctx.telemetry.dir, ctx.targetAngle));
  if (std::fabs(deltaAngle) > ctx.droneCfg.turnThreshold) {
    return std::make_unique<StateDecelerating>();
  }

  return nullptr;
}
