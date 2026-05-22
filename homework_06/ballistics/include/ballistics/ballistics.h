#pragma once

#include "input_params.h"

std::optional<double> GetTimeOfFlight(const AmmoParams&, double zd, double speed);
std::optional<double> GetHorizontalFlightDistance(const AmmoParams&, double speed, double timeOfFlight);

struct BallisticsSolution {
  Coord firePoint{};
  std::optional<Coord> maneuverPoint{};
};

BallisticsSolution GetBallistics(const InputParams&, double timeOfFlight, double horizontalFlightDistance);
