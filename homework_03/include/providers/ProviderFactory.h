#pragma once

#include "interfaces/ITargetLoader.h"

enum class TargetLoaderType { JSON_FILE };

ITargetLoaderPtr CreateTargetLoader(TargetLoaderType type);