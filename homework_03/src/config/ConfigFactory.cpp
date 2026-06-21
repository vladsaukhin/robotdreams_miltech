#include "config/ConfigFactory.h"

#include <format>
#include "config/JsonConfigLoader.h"

IConfigLoaderPtr CreateLoader(ConfigLoaderType type)
{
  switch (type) {
    case ConfigLoaderType::JSON_FILE:
      return std::make_unique<JsonConfigLoader>();
    default:
      throw std::out_of_range(std::format("CreateLoader factory cannot create a Loader for type {}",
                                          static_cast<std::underlying_type_t<ConfigLoaderType>>(type)));
  }
}