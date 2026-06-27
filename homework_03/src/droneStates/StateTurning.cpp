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
  if (ctx.telemetry.speed > 0.01) {
    std::cerr << "StateTurning::Execute: Drone is moving while turning. This should not happen.\n";
  }

  const double deltaAngle = AngleDiff(ctx.telemetry.diraction, ctx.telemetry.targetDir);
  const double deltaAngleAbs = std::fabs(deltaAngle);

  const double maxTurn = ctx.cfg.angularSpeed * ctx.cfg.simTimeStep;
  if (deltaAngleAbs <= maxTurn + gEps) {
    ctx.telemetry.diraction = ctx.telemetry.targetDir;
    ctx.telemetry.turnRemaining = 0.0;
    m_stopTime = 0.0;

    return std::make_unique<StateAccelerating>();
  }

  const double turnStep = (deltaAngle > 0.0 ? maxTurn : -maxTurn);
  ctx.telemetry.diraction = NormalizeAngle180(ctx.telemetry.diraction + turnStep);
  ctx.telemetry.turnRemaining = (deltaAngleAbs - maxTurn) / ctx.cfg.angularSpeed;
  m_stopTime = ctx.telemetry.turnRemaining;

  return nullptr;
}
