#pragma once

#include "interfaces/IConfigLoader.h"

class JsonConfigLoader : public IConfigLoader {
public:
  JsonConfigLoader() = default;

  JsonConfigLoader(JsonConfigLoader&&) = default;
  JsonConfigLoader& operator=(JsonConfigLoader&&) = default;

private:
  JsonConfigLoader(const JsonConfigLoader&) = delete;
  JsonConfigLoader& operator=(const JsonConfigLoader&) = delete;

public:
  bool Load(std::string_view dataFolderPath) override { return readConfig(dataFolderPath) && readAmmo(dataFolderPath) && validateConfig(); }
  const DroneConfig& GetConfig() const override { return m_config; };
  const AmmoParams& GetAmmoParams() const override { return m_config.ammoParams; };

private:
  bool readConfig(std::string_view dataFolderPath);

  bool readAmmo(std::string_view dataFolderPath);

  bool validateConfig();

private:
  DroneConfig m_config{};
};