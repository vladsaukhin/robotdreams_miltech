#pragma once

#include "interfaces/IConfigLoader.h"
#include "interfaces/ITargetLoader.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ILogger.h"

class MissionProcessor {
public:
  MissionProcessor(IConfigLoaderPtr, ITargetLoaderPtr, IBallisticSolverPtr, ILoggerPtr);

  ~MissionProcessor();

private:
  MissionProcessor(const MissionProcessor&) = delete;
  MissionProcessor& operator=(const MissionProcessor&) = delete;

  MissionProcessor(MissionProcessor&&) = delete;
  MissionProcessor& operator=(MissionProcessor&&) = delete;

public:
  void Init(std::string_view dataFolderPath);

  void ChangeSolver(IBallisticSolverPtr);

  void Reset();

  bool HasNext();

  void Step();

private:
  double getStopTime() const;

  void adjustDroneStateToTarget(const Target&);

  void changeDronePosition(double dt);

  void moveDrone();

private:
  IConfigLoaderPtr m_configLoader;
  ITargetLoaderPtr m_targetLoader;
  IBallisticSolverPtr m_ballisticSolver;
  ILoggerPtr m_logger;

  bool m_initialized{false};
  std::string m_dataFolderPath{};

private:
  Drone m_drone{};
  size_t m_step{0};
  double m_currentTime{};
  bool m_wasHit{};

  double m_acceleration{};
};