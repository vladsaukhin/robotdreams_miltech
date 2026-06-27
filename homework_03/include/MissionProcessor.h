#pragma once

#include "interfaces/IConfigLoader.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ILogger.h"
#include "interfaces/IDroneState.h"

#include "DronePhysics.h"

class MissionProcessor {
public:
  MissionProcessor(IConfigLoaderPtr, ITargetProviderPtr, IBallisticSolverPtr, ILoggerPtr);

  ~MissionProcessor();

private:
  MissionProcessor(const MissionProcessor&) = delete;
  MissionProcessor& operator=(const MissionProcessor&) = delete;

  MissionProcessor(MissionProcessor&&) = delete;
  MissionProcessor& operator=(MissionProcessor&&) = delete;

public:
  void Init(std::string_view dataFolderPath);

  void ChangeSolver(IBallisticSolverPtr);

  void AutoRun();

  void Reset();

  bool HasNext();

  void Step();

private:
  void runWorker();

  double getStopTime() const;

  void adjustDroneStateToTarget(const TargetFireParams&);

  void changeDronePosition(double dt);

  void moveDrone();

private:
  IConfigLoaderPtr m_configLoader;
  ITargetProviderPtr m_targetProvider;
  IBallisticSolverPtr m_ballisticSolver;
  ILoggerPtr m_logger;

  std::string m_dataFolderPath{};

private:
  DronePhysics m_drone;

  double m_acceleration{};

private:
  bool m_initialized{false};
  bool m_wasHit{false};
  std::thread m_worker;

  IDroneStatePtr m_droneState{};

  size_t m_step{0};
  double m_currentTime{};
};