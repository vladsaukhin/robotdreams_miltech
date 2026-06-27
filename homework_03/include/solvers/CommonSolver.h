#pragma once

#include "DroneAbstractions.h"
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
  TargetFireParams solveTargetFireParams(const BallisticsSolverContext& context);

  Coord getFirePoint(const Coord& dronPos, const Coord& targetPos);

  double computeTravelTime(double distance, double acceleration, double currentSpeed, double maxSpeed);

  Coord getAimPoint(const DroneTelemetry&);

protected:
  double m_timeOfFlight{};
  double m_horizontalFlightDistance{};
};