#pragma once

#include <memory>

#include "IConfigLoader.h"
#include "ITargetLoader.h"
#include "DroneAbstractions.h"

struct BallisticsSolverContext {
  int targetIdx{UNDEFINED_TARGET_ID};
  const IConfigLoader& conf;
  const Drone& drone;
  const ITargetLoader& targetLoader;
  double currentTime{};
  double acceleration{};
};

class IBallisticSolver {
public:
  virtual ~IBallisticSolver() = default;

public:
  virtual Target Solve(const BallisticsSolverContext&) = 0;
};

using IBallisticSolverPtr = std::unique_ptr<IBallisticSolver>;
