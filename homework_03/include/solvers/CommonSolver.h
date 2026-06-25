#pragma once

#include "interfaces/IBallisticSolver.h"

class CommonSolver {
public:
  CommonSolver() = default;

  CommonSolver(CommonSolver&&) = default;
  CommonSolver& operator=(CommonSolver&&) = default;

private:
  CommonSolver(const CommonSolver&) = delete;
  CommonSolver& operator=(const CommonSolver&) = delete;

protected:
  virtual bool solveCommonBallistics(const DroneConfig&) = 0;

protected:
  Target solveTargetFireParams(const BallisticsSolverContext& context);

  Coord getInterpolatedTarget(const ITargetLoader&, size_t targetIdx, double arrayTimeStep, double time);

  Coord getTargetVelocity(size_t targetIdx, const DroneConfig&, const ITargetLoader&, double currentTime);

  Coord getFirePoint(const Coord& dronPos, const Coord& targetPos);

  double computeTravelTime(double distance, double acceleration, double currentSpeed, double maxSpeed);

  Coord getAimPoint(const Drone&);

protected:
  double m_timeOfFlight{};
  double m_horizontalFlightDistance{};
};