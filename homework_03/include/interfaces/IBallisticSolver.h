#pragma once

#include <memory>
#include <string_view>

#include "IConfigLoader.h"
#include "ITargetProvider.h"
#include "DroneAbstractions.h"

struct BallisticsSolverContext {
  int targetIdx{UNDEFINED_TARGET_ID};
  const IConfigLoader& conf;
  const DroneTelemetry& telemetry;
  ITargetProvider& targetProvider;
  double currentTime{};
  double acceleration{};
  std::string_view dataPath{};
};

class IBallisticSolver {
public:
  virtual ~IBallisticSolver() = default;

public:
  virtual TargetFireParams Solve(const BallisticsSolverContext&) = 0;
};

using IBallisticSolverPtr = std::unique_ptr<IBallisticSolver>;
