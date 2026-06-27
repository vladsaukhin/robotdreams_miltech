#include "providers/ProviderFactory.h"

ITargetProviderPtr CreateTargetProvider(TargetProviderConfig conf)
{
  return std::visit(
    [](auto&& concreteConfig) -> ITargetProviderPtr {
      using ConfigType = std::decay_t<decltype(concreteConfig)>;

      if constexpr (std::is_same_v<ConfigType, JsonTargetProviderConfig>) {
        return std::make_unique<JsonTargetProvider>(std::move(concreteConfig));
      }
      else if constexpr (std::is_same_v<ConfigType, ThreadSafeJsonTargetProviderConfig>) {
        return std::make_unique<ThreadSafeJsonTargetProvider>(std::move(concreteConfig));
      }
    },
    std::move(conf));
}
