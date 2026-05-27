#include "providers/ProviderFactory.h"

#include <format>

#include "providers/JsonTargetLoader.h"

ITargetLoaderPtr CreateTargetLoader(TargetLoaderType type)
{
  switch (type) {
    case TargetLoaderType::JSON_FILE:
      return std::make_unique<JsonTargetLoader>();
    default:
      throw std::out_of_range(std::format("CreateTargetLoader factory cannot create a Loader for type {}",
                                          static_cast<std::underlying_type_t<TargetLoaderType>>(type)));
  }
}
