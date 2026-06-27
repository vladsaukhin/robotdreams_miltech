#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateTurning.h"
#include "droneStates/StateStopped.h"

#include "Utils.h"

IDroneStatePtr StateStopped::Execute(DroneStateContext& ctx)
{
  const double deltaAngle = std::fabs(AngleDiff(ctx.telemetry.diraction, ctx.telemetry.targetDir));
  if (std::fabs(deltaAngle) > ctx.cfg.turnThreshold) {
    ctx.telemetry.turnRemaining = deltaAngle / ctx.cfg.angularSpeed;
    return std::make_unique<StateTurning>();
  }

  ctx.telemetry.diraction = ctx.telemetry.targetDir;
  return std::make_unique<StateAccelerating>();
}
