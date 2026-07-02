#pragma once

#include <memory>
#include <optional>

#include "DroneAbstractions.h"

#include "drone_link.h"

struct BallisticsSolverContext {
  int targetIdx{UNDEFINED_TARGET_ID};
  const dlink::AmmoCfg& ammo;
  const dlink::DroneCfg& droneCfg;
  const dlink::Telemetry& telemetry;
  const dlink::TargetPos targetPos;
  double currentTime{};
  double acceleration{};
};

class IBallisticSolver {
public:
  virtual ~IBallisticSolver() = default;

public:
  virtual TargetFireParams Solve(const BallisticsSolverContext&) = 0;
};

using IBallisticSolverPtr = std::unique_ptr<IBallisticSolver>;
