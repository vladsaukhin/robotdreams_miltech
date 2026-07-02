#include "interfaces/DroneStateContext.h"

#include "droneStates/StateDecelerating.h"
#include "droneStates/StateStopped.h"

IDroneStatePtr StateDecelerating::Execute(DroneStateContext& ctx)
{
  m_stopTime = ctx.telemetry.speed / ctx.acceleration;
  m_control.accel = -ctx.GetSpeedAfterTime();

  if (ctx.telemetry.speed <= 0.01) {
    return std::make_unique<StateStopped>();
  }

  return nullptr;
}
