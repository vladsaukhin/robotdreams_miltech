#include "solvers/AnalyticalSolver.h"

#include <format>

Target AnalyticalSolver::Solve(const BallisticsSolverContext& context)
{
  const auto& conf = context.conf.GetConfig();
  solveCommonBallistics(conf);

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

bool AnalyticalSolver::setTimeOfFlight(const AmmoParams& ammo, double altitude, double speed)
{
  constexpr double g{9.81f};
  const double V0 = speed;
  const double sq_m = std::pow(ammo.mass, 2);
  const double sq_d = std::pow(ammo.drag, 2);

  const double a = ammo.drag * g * ammo.mass - 2 * sq_d * ammo.lift * V0;
  const double b = -3 * g * sq_m + 3 * ammo.drag * ammo.lift * ammo.mass * V0;
  const double c = 6 * sq_m * altitude;

  const double p = -std::pow(b, 2) / (3 * std::pow(a, 2));
  const double q = 2 * std::pow(b, 3) / (27 * std::pow(a, 3)) + c / a;

  if (p >= 0) {
    std::cerr << "No real solution for time of flight" << std::endl;
    return false;
  }

  const double fi_arg = 3 * q * std::sqrt(-3.0f / p) / (2 * p);
  if (fi_arg < -1 || fi_arg > 1) {
    std::cerr << "Arccos arg has to be in the range (-1;1)" << std::endl;
    return false;
  }

  const double fi = std::acos(fi_arg);
  m_timeOfFlight = 2 * std::sqrt(-p / 3.0f) * std::cos((fi + 4 * M_PI) / 3.0f) - b / (3 * a);

  if (m_timeOfFlight < 0) {
    std::cerr << "timeOfFlight < 0" << std::endl;
    return false;
  }

  return true;
}

bool AnalyticalSolver::setHorizontalFlightDistance(const AmmoParams& ammo, double speed)
{
  constexpr double g{9.81f};
  const double V0 = speed;
  const double sq_m = std::pow(ammo.mass, 2);
  const double sq_d = std::pow(ammo.drag, 2);
  const double cu_d = std::pow(ammo.drag, 3);
  const double sq_l = std::pow(ammo.lift, 2);
  const double cu_l = std::pow(ammo.lift, 3);

  const double h_part1 = V0 * m_timeOfFlight;
  const double h_part2 = std::pow(m_timeOfFlight, 2) * ammo.drag * V0 / (2 * ammo.mass);
  const double h_part3 =
    std::pow(m_timeOfFlight, 3) * (6 * ammo.drag * g * ammo.lift * ammo.mass - 6 * sq_d * (sq_l - 1) * V0) / (36 * sq_m);

  // clang-format off
   const double h_part4 = std::pow(m_timeOfFlight, 4)
      * (-6 * sq_d * g * ammo.lift * (1 + sq_l + sq_l * sq_l) * ammo.mass
         + 3 * cu_d * sq_l * (1 + sq_l) * V0
         + 6 * cu_d * sq_l * sq_l * (1 + sq_l) * V0)
      / (36 * std::pow(1 + sq_l, 2) * std::pow(ammo.mass, 3));

   const double h_part5 = std::pow(m_timeOfFlight, 5)
      * (3 * cu_d * g * cu_l * ammo.mass
         - 3 * sq_d * sq_d * sq_l * (1 + sq_l) * V0)
      / (36 * (1 + sq_l) * sq_m * sq_m);
  // clang-format on

  m_horizontalFlightDistance = h_part1 - h_part2 + h_part3 + h_part4 + h_part5;

  if (m_horizontalFlightDistance < 0) {
    std::cerr << "horizontalFlightDistance < 0" << std::endl;
    return false;
  }

  return true;
}

void AnalyticalSolver::solveCommonBallistics(const DroneConfig& conf)
{
  if (!m_commonBallisticsSolved) {
    if (setTimeOfFlight(conf.ammoParams, conf.altitude, conf.attackSpeed) &&
        setHorizontalFlightDistance(conf.ammoParams, conf.attackSpeed)) {
      m_commonBallisticsSolved = true;
    }
    else {
      throw std::logic_error(std::format("Connot solve balistics for given ammo {}", conf.ammoName));
    }
  }
}

Coord AnalyticalSolver::getInterpolatedTarget(const ITargetLoader& targetsLoader, size_t targetIdx, double arrayTimeStep, double time)
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

Coord AnalyticalSolver::getTargetVelocity(size_t targetIdx, const DroneConfig& conf, const ITargetLoader& targetsLoader, double currentTime)
{
  const double dt = conf.simTimeStep;
  const auto p0 = getInterpolatedTarget(targetsLoader, targetIdx, conf.arrayTimeStep, currentTime);
  const auto p1 = getInterpolatedTarget(targetsLoader, targetIdx, conf.arrayTimeStep, currentTime + dt);
  return {(p1.x - p0.x) / dt, (p1.y - p0.y) / dt};
}

Coord AnalyticalSolver::getFirePoint(const Coord& dronPos, const Coord& targetPos)
{
  const Coord delta = targetPos - dronPos;
  const double distanceToTarget = delta.Length();
  const double ratio = (distanceToTarget - m_horizontalFlightDistance) / distanceToTarget;

  return dronPos + (targetPos - dronPos) * ratio;
}

double AnalyticalSolver::computeTravelTime(double distance, double acceleration, double currentSpeed, double maxSpeed)
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

Coord AnalyticalSolver::getAimPoint(const Drone& drone)
{
  Coord dir{std::cos(drone.diraction), std::sin(drone.diraction)};

  return drone.position + dir * m_horizontalFlightDistance;
}