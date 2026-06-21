#pragma once

#include <memory>

#include "Coords.hpp"

class ITargetLoader {
public:
  virtual ~ITargetLoader() = default;

public:
  virtual bool Load(std::string_view dataFolderPath) = 0;

  virtual size_t GetTargetCount() const = 0;
  virtual size_t GetTargetTimeStepsCount() const = 0;

  virtual const ListOfCoords& GetTargetTimes(size_t targetIdx) const = 0;
  virtual const Coord& GetTargetCoordByTime(size_t targetIdx, size_t timeIdx) const = 0;
};

using ITargetLoaderPtr = std::unique_ptr<ITargetLoader>;