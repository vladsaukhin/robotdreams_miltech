#include "solvers/CommonSolver.h"
#include "DroneAbstractions.h"

TargetFireParams CommonSolver::solveTargetFireParams(const BallisticsSolverContext& context)
{
  const auto& conf = context.conf.GetConfig();

  TargetFireParams target{.idx = context.targetIdx};

  const auto& drone = context.telemetry;

  const auto targetState = context.targetProvider.GetTargetState(target.idx);

  // get fire point based on current target position
  const auto currentFirePoint = getFirePoint(drone.position, targetState.position);

  double totalTime =
    computeTravelTime((currentFirePoint - drone.position).Length(), context.acceleration, drone.speed, conf.attackSpeed) + m_timeOfFlight;

  const auto predictedPos = targetState.position + (targetState.velocity * totalTime);

  const auto predictedFirePoint = getFirePoint(drone.position, predictedPos);

  // get time and position based on predicted coordinates

  target.totalTime =
    computeTravelTime((predictedFirePoint - drone.position).Length(), context.acceleration, drone.speed, conf.attackSpeed) + m_timeOfFlight;

  target.releasePoint = predictedFirePoint;
  target.predictedPosition = predictedPos;

  target.aimPoint = getAimPoint(drone);

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

Coord CommonSolver::getAimPoint(const DroneTelemetry& drone)
{
  Coord dir{std::cos(drone.diraction), std::sin(drone.diraction)};

  return drone.position + dir * m_horizontalFlightDistance;
}