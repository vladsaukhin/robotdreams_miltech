#pragma once

#include "Coords.hpp"

double NormalizeAngle360(double angle);  // [0, 2π)

double NormalizeAngle180(double angle);  // (-π, π]

double AngleDiff(double from, double to);

double GetDistance(const Coord& a, const Coord& b);