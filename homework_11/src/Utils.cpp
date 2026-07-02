#include "Utils.h"

double NormalizeAngle360(double angle)  // [0, 2π)
{
  double a = std::fmod(angle, 2.0f * M_PI);
  if (a < 0.0f) {
    a += 2.0f * M_PI;
  }
  return a;
}

double NormalizeAngle180(double angle)  // (-π, π]
{
  constexpr double TWO_PI = 2.0f * M_PI;
  while (angle > M_PI) {
    angle -= TWO_PI;
  }
  while (angle < -M_PI) {
    angle += TWO_PI;
  }
  return angle;
}

double AngleDiff(double from, double to)
{
  return NormalizeAngle180(to - from);
}

double GetDistance(const Coord& a, const Coord& b)
{
  return (b - a).Length();
}