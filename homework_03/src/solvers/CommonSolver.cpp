#include "solvers/CommonSolver.h"

Target CommonSolver::solveTargetFireParams(const BallisticsSolverContext& context)
{
  const auto& conf = context.conf.GetConfig();

  Target target{.idx = context.targetIdx};

  const auto& drone = context.drone;

  const auto targetVelocity = getTargetVelocity(target.idx, conf, context.targetLoader, context.currentTime);

  // get current position from targets file
  const auto currentPos = getInterpolatedTarget(context.targetLoader, target.idx, conf.arrayTimeStep, context.currentTime);

  // get fire point based on current target position
  const auto currentFirePoint = getFirePoint(drone.position, currentPos);

  double totalTime =
    computeTravelTime((currentFirePoint - drone.position).Length(), context.acceleration, drone.speed, conf.attackSpeed) + m_timeOfFlight;

  const auto predictedPos = currentPos + (targetVelocity * totalTime);

  const auto predictedFirePoint = getFirePoint(drone.position, predictedPos);

  // get time and position based on predicted coordinates

  target.totalTime =
    computeTravelTime((predictedFirePoint - drone.position).Length(), context.acceleration, drone.speed, conf.attackSpeed) + m_timeOfFlight;

  target.releasePoint = predictedFirePoint;
  target.predictedPosition = predictedPos;

  target.aimPoint = getAimPoint(drone);

  return target;
}

Coord CommonSolver::getInterpolatedTarget(const ITargetLoader& targetsLoader, size_t targetIdx, double arrayTimeStep, double time)
{
  const double samplePos = time / arrayTimeStep;
  const int rawIdx = static_cast<int>(std::floor(samplePos));
  const int idx = rawIdx % targetsLoader.GetTargetTimeStepsCount();
  const int next = (idx + 1) % targetsLoader.GetTargetTimeStepsCount();
  const double frac = samplePos - std::floor(samplePos);

  const auto& targetTimes = targetsLoader.GetTargetTimes(targetIdx);
  const double x = targetTimes[idx].x + (targetTimes[next].x - targetTimes[idx].x) * frac;
  const double y = targetTimes[idx].y + (targetTimes[next].y - targetTimes[idx].y) * frac;
  return {x, y};
}

Coord CommonSolver::getTargetVelocity(size_t targetIdx, const DroneConfig& conf, const ITargetLoader& targetsLoader, double currentTime)
{
  const double dt = conf.simTimeStep;
  const auto p0 = getInterpolatedTarget(targetsLoader, targetIdx, conf.arrayTimeStep, currentTime);
  const auto p1 = getInterpolatedTarget(targetsLoader, targetIdx, conf.arrayTimeStep, currentTime + dt);
  return {(p1.x - p0.x) / dt, (p1.y - p0.y) / dt};
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

Coord CommonSolver::getAimPoint(const Drone& drone)
{
  Coord dir{std::cos(drone.diraction), std::sin(drone.diraction)};

  return drone.position + dir * m_horizontalFlightDistance;
}