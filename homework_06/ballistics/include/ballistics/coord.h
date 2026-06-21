#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <ostream>
#include <format>

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

  constexpr Coord& operator-=(double scalar) noexcept
  {
    x -= scalar;
    y -= scalar;
    return *this;
  }

  constexpr Coord& operator+=(double scalar) noexcept
  {
    x += scalar;
    y += scalar;
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

  std::string ToString() const { return std::format("({:.2f}, {:.2f})", x, y); }

  [[nodiscard]] bool IsValid() const { return std::isfinite(x) && std::isfinite(y) && x >= 0.0f && y >= 0.0f; }
};

using ListOfCoords = std::vector<Coord>;

inline std::ostream& operator<<(std::ostream& os, const Coord& c)
{
  os << c.ToString();
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

constexpr Coord operator+(Coord l, double scalar) noexcept
{
  l += scalar;
  return l;
}

constexpr Coord operator-(Coord l, double scalar) noexcept
{
  l -= scalar;
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