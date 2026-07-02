#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateTurning.h"
#include "droneStates/StateStopped.h"

#include "Utils.h"

IDroneStatePtr StateStopped::Execute(DroneStateContext& ctx)
{
  const double deltaAngle = std::fabs(AngleDiff(ctx.telemetry.dir, ctx.targetAngle));
  if (std::fabs(deltaAngle) > ctx.droneCfg.turnThreshold) {
    // ctx.telemetry.turnRemaining = deltaAngle / ctx.droneCfg.angularSpeed;
    return std::make_unique<StateTurning>();
  }

  return std::make_unique<StateAccelerating>();
}
