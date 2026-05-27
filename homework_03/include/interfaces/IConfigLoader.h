#pragma once

#include <memory>

#include "DroneConfig.h"

class IConfigLoader {
public:
  virtual ~IConfigLoader() = default;

public:
  virtual bool Load(std::string_view dataFolderPath) = 0;

  virtual const DroneConfig& GetConfig() const = 0;
  virtual const AmmoParams& GetAmmoParams() const = 0;
};

using IConfigLoaderPtr = std::unique_ptr<IConfigLoader>;