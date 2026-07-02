#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateTurning.h"
#include <iostream>

#include "Utils.h"

namespace {

constexpr double gEps{1e-6f};
}

IDroneStatePtr StateTurning::Execute(DroneStateContext& ctx)
{
  if (ctx.drone.speed > 0.01) {
    std::cerr << "StateTurning::Execute: Drone is moving while turning. This should not happen.\n";
  }

  const double deltaAngle = AngleDiff(ctx.drone.diraction, ctx.drone.targetDir);
  const double deltaAngleAbs = std::fabs(deltaAngle);

  const double maxTurn = ctx.cfg.angularSpeed * ctx.cfg.simTimeStep;
  if (deltaAngleAbs <= maxTurn + gEps) {
    ctx.drone.diraction = ctx.drone.targetDir;
    ctx.drone.turnRemaining = 0.0;
    m_stopTime = 0.0;

    return std::make_unique<StateAccelerating>();
  }

  const double turnStep = (deltaAngle > 0.0 ? maxTurn : -maxTurn);
  ctx.drone.diraction = NormalizeAngle180(ctx.drone.diraction + turnStep);
  ctx.drone.turnRemaining = (deltaAngleAbs - maxTurn) / ctx.cfg.angularSpeed;
  m_stopTime = ctx.drone.turnRemaining;

  return nullptr;
}
