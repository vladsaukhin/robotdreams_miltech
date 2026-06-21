#include "ballistics/input_params.h"

#include <array>
#include <fstream>
#include <iomanip>

namespace {

constexpr AmmoParams VOG17{"VOG-17", 0.35, 0.07, 0.0};
constexpr AmmoParams M67{"M67", 0.6, 0.1, 0.0};
constexpr AmmoParams RKG3{"RKG-3", 1.2, 0.1, 0.0};
constexpr AmmoParams GLIDING_VOG{"GLIDING-VOG", 0.45, 0.1, 1.0};
constexpr AmmoParams GLIDING_RKG{"GLIDING-RKG", 1.4, 0.1, 1.0};

constexpr std::array g_AmmoTable{VOG17, M67, RKG3, GLIDING_VOG, GLIDING_RKG};

constexpr size_t MAX_AMMO_NAME_SIZE{20};

bool validateInputParams(const InputParams& params)
{
  if (!params.position.IsValid() || params.altitude < 0.0) {
    return false;
  }
  if (!params.target.IsValid()) {
    return false;
  }
  if (params.attackSpeed <= 0.0) {
    return false;
  }
  if (params.accelerationPath < 0.0) {
    return false;
  }
  if (params.ammo_name.empty() || params.ammo_name.size() > MAX_AMMO_NAME_SIZE) {
    return false;
  }

  return true;
}

}  // namespace

std::optional<AmmoParams> GetAmmoParams(std::string_view name)
{
  for (const auto& ammo : g_AmmoTable) {
    if (ammo.name == name)
      return ammo;
  }

  return std::nullopt;
}

std::optional<InputParams> ParseInputParams(std::string_view file_path)
{
  InputParams inputParams{};

  std::ifstream input(file_path.data());
  if (!input) {
    return std::nullopt;
  }
  else if (!(input >> inputParams.position.x >> inputParams.position.y >> inputParams.altitude >> inputParams.target.x >>
             inputParams.target.y >> inputParams.attackSpeed >> inputParams.accelerationPath >> std::setw(MAX_AMMO_NAME_SIZE) >>
             inputParams.ammo_name)) {
    return std::nullopt;
  }

  if (!validateInputParams(inputParams)) {
    return std::nullopt;
  }

  return inputParams;
}