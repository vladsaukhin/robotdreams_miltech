#pragma once

#include <vector>
#include <iostream>
#include <cmath>

struct Coord {
  double x{};
  double y{};

  constexpr Coord& operator+=(const Coord& other) noexcept
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  constexpr Coord& operator-=(const Coord& other) noexcept
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  constexpr Coord& operator*=(double scalar) noexcept
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  constexpr Coord& operator/=(double scalar) noexcept
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  constexpr auto operator<=>(const Coord&) const noexcept = default;

  // Length / normalization
  double Length() const noexcept { return std::sqrt(x * x + y * y); }

  Coord Normalized() const noexcept
  {
    double len = Length();
    if (len == 0.0f)
      return {0.0f, 0.0f};
    return {x / len, y / len};
  }
};

using ListOfCoords = std::vector<Coord>;

inline std::ostream& operator<<(std::ostream& os, const Coord& c)
{
  os << "(" << c.x << ", " << c.y << ")";
  return os;
}

constexpr Coord operator+(Coord l, const Coord& r) noexcept
{
  l += r;
  return l;
}

constexpr Coord operator-(Coord l, const Coord& r) noexcept
{
  l -= r;
  return l;
}

constexpr Coord operator*(Coord l, double scalar) noexcept
{
  l *= scalar;
  return l;
}

constexpr Coord operator*(double scalar, Coord r) noexcept
{
  r *= scalar;
  return r;
}

constexpr Coord operator/(Coord l, double scalar) noexcept
{
  l /= scalar;
  return l;
}