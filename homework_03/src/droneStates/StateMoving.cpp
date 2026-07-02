#include "interfaces/DroneStateContext.h"

#include "droneStates/StateDecelerating.h"
#include "droneStates/StateMoving.h"

#include "Utils.h"

IDroneStatePtr StateMoving::Execute(DroneStateContext& ctx)
{
  // Update position
  ctx.MoveDrone(ctx.drone.speed);
  m_stopTime = ctx.drone.speed / ctx.acceleration;

  const double deltaAngle = std::fabs(AngleDiff(ctx.drone.diraction, ctx.drone.targetDir));
  if (std::fabs(deltaAngle) > ctx.cfg.turnThreshold) {
    return std::make_unique<StateDecelerating>();
  }

  return nullptr;
}
