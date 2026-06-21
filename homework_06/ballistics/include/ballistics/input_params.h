#pragma once

#include "coord.h"

#include <optional>

struct AmmoParams {
  std::string_view name;
  double mass{};
  double drag{};
  double lift{};  // 0 = FreeFall or 1.0 = Gliding
};

using AmmoParamsOpt = std::optional<AmmoParams>;

AmmoParamsOpt GetAmmoParams(std::string_view ammo_name);

struct InputParams {
  Coord position{};
  double altitude{};
  Coord target{};
  double attackSpeed{};
  double accelerationPath{};
  std::string ammo_name;
};

using InputParamsOpt = std::optional<InputParams>;

InputParamsOpt ParseInputParams(std::string_view file_path);
