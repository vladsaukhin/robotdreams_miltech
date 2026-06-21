#pragma once

#include "interfaces/ILogger.h"

enum class LoggerType { JSON_FILE };

ILoggerPtr CreateLogger(LoggerType type);