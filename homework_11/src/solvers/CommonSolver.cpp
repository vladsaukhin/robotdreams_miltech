#include "solvers/CommonSolver.h"
#include "Coords.hpp"
#include "DroneAbstractions.h"

TargetFireParams CommonSolver::solveTargetFireParams(const BallisticsSolverContext& context)
{
  const auto attackSpeed = context.droneCfg.attackSpeed;

  TargetFireParams target{.idx = context.targetIdx};

  const auto& drone = context.telemetry;

  Coord targetPos(context.targetPos.x, context.targetPos.y);
  Coord dronePos(context.telemetry.x, context.telemetry.y);

  // get fire point based on current target position
  const auto currentFirePoint = getFirePoint(dronePos, targetPos);

  target.totalTime =
    computeTravelTime((currentFirePoint - dronePos).Length(), context.acceleration, drone.speed, attackSpeed) + m_timeOfFlight;

  target.releasePoint = currentFirePoint;

  return target;
}

Coord CommonSolver::getFirePoint(const Coord& dronPos, const Coord& targetPos)
{
  const Coord delta = targetPos - dronPos;
  const double distanceToTarget = delta.Length();
  const double ratio = (distanceToTarget - m_horizontalFlightDistance) / distanceToTarget;

  return dronPos + (targetPos - dronPos) * ratio;
}

double CommonSolver::computeTravelTime(double distance, double acceleration, double currentSpeed, double maxSpeed)
{
  if (distance <= 0.0f) {
    return 0.0f;
  }

  if (currentSpeed >= maxSpeed) {
    return distance / maxSpeed;
  }

  const double distanceToMaxSpeed = (maxSpeed * maxSpeed - currentSpeed * currentSpeed) / (2.0f * acceleration);

  if (distance <= distanceToMaxSpeed) {
    return (-currentSpeed + std::sqrt(currentSpeed * currentSpeed + 2.0f * acceleration * distance)) / acceleration;
  }

  const double timeToMaxSpeed = (maxSpeed - currentSpeed) / acceleration;
  const double cruiseDistance = distance - distanceToMaxSpeed;
  const double cruiseTime = cruiseDistance / maxSpeed;

  return timeToMaxSpeed + cruiseTime;
}