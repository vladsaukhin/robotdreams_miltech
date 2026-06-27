#pragma once

#include <variant>

#include "providers/JsonTargetProvider.h"
#include "providers/ThreadSafeJsonTargetProvider.h"

using TargetProviderConfig = std::variant<JsonTargetProviderConfig, ThreadSafeJsonTargetProviderConfig>;

ITargetProviderPtr CreateTargetProvider(TargetProviderConfig);