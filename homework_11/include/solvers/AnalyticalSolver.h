#pragma once

#include "interfaces/IBallisticSolver.h"
#include "CommonSolver.h"

class AnalyticalSolver : public IBallisticSolver, public CommonSolver {
public:
  AnalyticalSolver() = default;

  AnalyticalSolver(AnalyticalSolver&&) = default;
  AnalyticalSolver& operator=(AnalyticalSolver&&) = default;

private:
  AnalyticalSolver(const AnalyticalSolver&) = delete;
  AnalyticalSolver& operator=(const AnalyticalSolver&) = delete;

public:
  TargetFireParams Solve(const BallisticsSolverContext& context) override;

private:
  bool solveCommonBallistics(const dlink::AmmoCfg&, const dlink::DroneCfg&, float altitude) override;

  bool setTimeOfFlight(const dlink::AmmoCfg&, double altitude, double speed);
  bool setHorizontalFlightDistance(const dlink::AmmoCfg&, double speed);
};