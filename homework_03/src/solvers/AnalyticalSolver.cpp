#include "solvers/AnalyticalSolver.h"

#include <format>

TargetFireParams AnalyticalSolver::Solve(const BallisticsSolverContext& context)
{
  const auto& conf = context.conf.GetConfig();

  if (!solveCommonBallistics(conf)) {
    throw std::logic_error(std::format("Connot solve balistics for given ammo {}", conf.ammoName));
  }

  return solveTargetFireParams(context);
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

bool AnalyticalSolver::solveCommonBallistics(const DroneConfig& conf)
{
  if (setTimeOfFlight(conf.ammoParams, conf.altitude, conf.attackSpeed) && setHorizontalFlightDistance(conf.ammoParams, conf.attackSpeed)) {
    return true;
  }
  else {
    return false;
  }
}
