#include "loggers/LoggerFactory.h"

#include "loggers/JsonLogger.h"

#include <format>

ILoggerPtr CreateLogger(LoggerType type)
{
  switch (type) {
    case LoggerType::JSON_FILE:
      return std::make_unique<JsonLogger>();
    default:
      throw std::out_of_range(
        std::format("CreateLogger factory cannot create a Logger for type {}", static_cast<std::underlying_type_t<LoggerType>>(type)));
  }
}