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
  Target Solve(const BallisticsSolverContext& context) override;

private:
  bool solveCommonBallistics(const DroneConfig&) override;

  bool setTimeOfFlight(const AmmoParams&, double altitude, double speed);
  bool setHorizontalFlightDistance(const AmmoParams&, double speed);
};