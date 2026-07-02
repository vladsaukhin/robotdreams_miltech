#include "interfaces/DroneStateContext.h"

#include "droneStates/StateAccelerating.h"
#include "droneStates/StateTurning.h"

#include "Utils.h"

namespace {

constexpr double gEps{1e-6f};
}

IDroneStatePtr StateTurning::Execute(DroneStateContext& ctx)
{
  const double deltaAngle = AngleDiff(ctx.telemetry.dir, ctx.targetAngle);
  const double deltaAngleAbs = std::fabs(deltaAngle);

  const double maxTurn = ctx.droneCfg.angularSpeed * ctx.droneCfg.timeStep;
  if (deltaAngleAbs <= maxTurn + gEps) {
    m_control.accel = 0.0f;
    m_control.turnRate = 0.0f;
    m_stopTime = 0.0;

    return std::make_unique<StateAccelerating>();
  }

  m_control.turnRate = (deltaAngle - maxTurn) / ctx.droneCfg.angularSpeed;
  m_stopTime = (deltaAngleAbs - maxTurn) / ctx.droneCfg.angularSpeed;

  return nullptr;
}
