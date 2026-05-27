#pragma once

#include "interfaces/IConfigLoader.h"

enum class ConfigLoaderType { JSON_FILE };

IConfigLoaderPtr CreateLoader(ConfigLoaderType type);