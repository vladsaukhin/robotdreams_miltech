#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateTurning.h"
#include "droneStates/StateStopped.h"

#include "Utils.h"

IDroneStatePtr StateStopped::Execute(DroneStateContext& ctx)
{
  const double deltaAngle = std::fabs(AngleDiff(ctx.drone.diraction, ctx.drone.targetDir));
  if (std::fabs(deltaAngle) > ctx.cfg.turnThreshold) {
    ctx.drone.turnRemaining = deltaAngle / ctx.cfg.angularSpeed;
    return std::make_unique<StateTurning>();
  }

  ctx.drone.diraction = ctx.drone.targetDir;
  return std::make_unique<StateAccelerating>();
}
