#pragma once

#include "interfaces/IBallisticSolver.h"

class AnalyticalSolver : public IBallisticSolver {
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
  bool setTimeOfFlight(const AmmoParams&, double altitude, double speed);

  bool setHorizontalFlightDistance(const AmmoParams&, double speed);

  void solveCommonBallistics(const DroneConfig&);

  Coord getInterpolatedTarget(const ITargetLoader&, size_t targetIdx, double arrayTimeStep, double time);

  Coord getTargetVelocity(size_t targetIdx, const DroneConfig&, const ITargetLoader&, double currentTime);

  Coord getFirePoint(const Coord& dronPos, const Coord& targetPos);

  double computeTravelTime(double distance, double acceleration, double currentSpeed, double maxSpeed);

  Coord getAimPoint(const Drone&);

private:
  bool m_commonBallisticsSolved{false};
  double m_timeOfFlight{};
  double m_horizontalFlightDistance{};
};