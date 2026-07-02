#pragma once

#include <thread>
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ILogger.h"
#include "interfaces/IDroneState.h"

#include "ParseArgs.hpp"

#include <drone_link.h>
#include <GpioOutput.h>
#include <UartPort.h>

class MissionProcessor {
public:
  MissionProcessor(Args&&, IBallisticSolverPtr, ILoggerPtr);

  ~MissionProcessor();

private:
  MissionProcessor(const MissionProcessor&) = delete;
  MissionProcessor& operator=(const MissionProcessor&) = delete;

  MissionProcessor(MissionProcessor&&) = delete;
  MissionProcessor& operator=(MissionProcessor&&) = delete;

public:
  void Init(std::string_view dataFolderPath);

  void AutoRun();

  bool HasNext();

  void Step();

private:
  void sendControl();

  void processPacket(dlink::PacketType type, const uint8_t* payload, uint8_t len);

private:
  Args m_args;

  IBallisticSolverPtr m_ballisticSolver;
  ILoggerPtr m_logger;

  std::string m_dataFolderPath{};

private:
  double m_acceleration{};

private:
  bool m_initialized{false};
  bool m_wasHit{false};

  IDroneStatePtr m_droneState{};

  std::optional<int> m_currentTarget{};
  size_t m_step{0};
  double m_currentTime{};

private:
  UartPort m_uart;
  GpioOutput m_startGpio;
  GpioOutput m_dropGpio;

  dlink::Parser m_parser;

  std::optional<dlink::AmmoCfg> m_ammo;
  std::optional<dlink::DroneCfg> m_droneCfg;
  std::unordered_map<uint8_t, dlink::TargetPos> m_targets;

  std::optional<dlink::Telemetry> m_telemetry;
  dlink::Control m_control;
};