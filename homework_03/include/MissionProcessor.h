#pragma once

#include "interfaces/IConfigLoader.h"
#include "interfaces/ITargetLoader.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ILogger.h"

class MissionProcessor {
public:
  MissionProcessor(IConfigLoaderPtr, ITargetLoaderPtr, IBallisticSolverPtr, ILoggerPtr);

  ~MissionProcessor() = default;

private:
  MissionProcessor(const MissionProcessor&) = delete;
  MissionProcessor& operator=(const MissionProcessor&) = delete;

  MissionProcessor(MissionProcessor&&) = delete;
  MissionProcessor& operator=(MissionProcessor&&) = delete;

public:
  void Init(std::string_view dataFolderPath);

  void ChangeSolver(IBallisticSolverPtr);

  void Reset();

  void ProcessMission();

private:
  double getStopTime() const;

  // required method ?
  bool hasNext();

  // required method ?
  Target step();

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
  int m_currentProcessedTargetID{UNDEFINED_TARGET_ID};
  Drone m_drone{};
  size_t m_step{0};
  double m_currentTime{};

  double m_acceleration{};
};