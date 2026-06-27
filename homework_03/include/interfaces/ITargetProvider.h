#pragma once

#include <memory>
#include <optional>

#include "Coords.hpp"

struct TargetState {
  Coord position;
  Coord velocity;
};

class ITargetProvider {
public:
  virtual ~ITargetProvider() = default;

public:
  virtual size_t GetTargetCount() const = 0;

  virtual void Start(std::optional<double> currentTime = std::nullopt) = 0;
  virtual void Stop() = 0;

  virtual TargetState GetTargetState(size_t targetIdx) const = 0;
};

using ITargetProviderPtr = std::unique_ptr<ITargetProvider>;