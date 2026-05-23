#include "ballistics/ballistics.h"

#include <cmath>
#include "ballistics/coord.h"

namespace {

constexpr double g_GravitationalConstant{9.81f};

}  // namespace

std::optional<double> GetTimeOfFlight(const AmmoParams& ammo, double zd, double speed)
{
  const double V0 = speed;
  const double sq_m = std::pow(ammo.mass, 2);
  const double sq_d = std::pow(ammo.drag, 2);

  const double a = ammo.drag * g_GravitationalConstant * ammo.mass - 2 * sq_d * ammo.lift * V0;
  const double b = -3 * g_GravitationalConstant * sq_m + 3 * ammo.drag * ammo.lift * ammo.mass * V0;
  const double c = 6 * sq_m * zd;

  const double p = -std::pow(b, 2) / (3 * std::pow(a, 2));
  const double q = 2 * std::pow(b, 3) / (27 * std::pow(a, 3)) + c / a;

  if (p >= 0) {
    return std::nullopt;
  }

  const double fi_arg = 3 * q * std::sqrt(-3.0 / p) / (2 * p);
  if (fi_arg < -1 || fi_arg > 1) {
    return std::nullopt;
  }

  const double fi = std::acos(fi_arg);
  const double t = 2 * std::sqrt(-p / 3.0) * std::cos((fi + 4 * M_PI) / 3.0) - b / (3 * a);

  if (t < 0) {
    return std::nullopt;
  }

  return t;
}

std::optional<double> GetHorizontalFlightDistance(const AmmoParams& ammo, double speed, double timeOfFlight)
{
  const double V0 = speed;
  const double sq_m = std::pow(ammo.mass, 2);
  const double sq_d = std::pow(ammo.drag, 2);
  const double cu_d = std::pow(ammo.drag, 3);
  const double sq_l = std::pow(ammo.lift, 2);
  const double cu_l = std::pow(ammo.lift, 3);

  const double h_part1 = V0 * timeOfFlight;
  const double h_part2 = std::pow(timeOfFlight, 2) * ammo.drag * V0 / (2 * ammo.mass);
  const double h_part3 = std::pow(timeOfFlight, 3) *
                         (6 * ammo.drag * g_GravitationalConstant * ammo.lift * ammo.mass - 6 * sq_d * (sq_l - 1) * V0) / (36 * sq_m);

  const double h_part4 = std::pow(timeOfFlight, 4) *
                         (-6 * sq_d * g_GravitationalConstant * ammo.lift * (1 + sq_l + sq_l * sq_l) * ammo.mass +
                          3 * cu_d * sq_l * (1 + sq_l) * V0 + 6 * cu_d * sq_l * sq_l * (1 + sq_l) * V0) /
                         (36 * std::pow(1 + sq_l, 2) * std::pow(ammo.mass, 3));

  const double h_part5 = std::pow(timeOfFlight, 5) *
                         (3 * cu_d * g_GravitationalConstant * cu_l * ammo.mass - 3 * sq_d * sq_d * sq_l * (1 + sq_l) * V0) /
                         (36 * (1 + sq_l) * sq_m * sq_m);

  const double h = h_part1 - h_part2 + h_part3 + h_part4 + h_part5;

  if (h < 0) {
    return std::nullopt;
  }

  return h;
}

BallisticsSolution GetBallistics(const InputParams& params, double timeOfFlight, double horizontalFlightDistance)
{
  BallisticsSolution solution{};

  double distanceToTarget = std::sqrt(std::pow(params.target.x - params.position.x, 2) + std::pow(params.target.y - params.position.y, 2));

  Coord maneuverPoint = params.position;

  if (horizontalFlightDistance + params.accelerationPath > distanceToTarget) {
    if (std::fabs(distanceToTarget) < 1e-6) {
      distanceToTarget = horizontalFlightDistance + params.accelerationPath;
      maneuverPoint = params.target - distanceToTarget;
    }
    else {
      maneuverPoint =
        params.target - (params.target - params.position) * (horizontalFlightDistance + params.accelerationPath) / distanceToTarget;

      distanceToTarget = std::sqrt(std::pow(params.target.x - maneuverPoint.x, 2) + std::pow(params.target.y - maneuverPoint.y, 2));
    }

    solution.maneuverPoint = maneuverPoint;
  }

  const double ratio = (distanceToTarget - horizontalFlightDistance) / distanceToTarget;
  solution.firePoint = maneuverPoint + (params.target - maneuverPoint) * ratio;

  return solution;
}
