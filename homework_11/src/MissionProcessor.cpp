#include "MissionProcessor.h"

#include <format>
#include <utility>

#include "Coords.hpp"
#include "Utils.h"

#include "DroneAbstractions.h"

#include "droneStates/StateStopped.h"
#include "drone_link.h"
#include "interfaces/DroneStateContext.h"

namespace {

constexpr size_t MAX_STEPS{10'000};

}  // namespace

MissionProcessor::MissionProcessor(Args&& args, IBallisticSolverPtr ballisticSolver, ILoggerPtr logger)
  : m_args(std::move(args))
  , m_ballisticSolver(std::move(ballisticSolver))
  , m_logger(std::move(logger))
  , m_uart(m_args.uart)
  , m_startGpio(m_args.gpiochip, m_args.startLine, "drone-start")
  , m_dropGpio(m_args.gpiochip, m_args.dropLine, "drone-drop")
{
  if (!m_ballisticSolver) {
    throw std::logic_error("BallisticSolver is not initialized");
  }

  if (!m_logger) {
    throw std::logic_error("Logger is not initialized");
  }
}

MissionProcessor::~MissionProcessor()
{
  if (m_step < MAX_STEPS) {
    m_logger->DumpLog(m_dataFolderPath, m_step);
  }
}

void MissionProcessor::Init(std::string_view dataFolderPath)
{
  m_droneState = std::make_unique<StateStopped>();

  m_dataFolderPath = dataFolderPath;

  m_initialized = true;
}

void MissionProcessor::AutoRun()
{
  if (!m_initialized) {
    throw std::logic_error("MissionProcessor is not initialized");
  }

  m_startGpio.Set(true);  // init checker

  while (HasNext()) {
    Step();

    const double sleepTime = m_droneCfg->timeStep / m_droneCfg->timeScale;
    std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
  }
}

bool MissionProcessor::HasNext()
{
  if (!m_initialized) {
    throw std::logic_error("MissionProcessor is not initialized");
  }

  return !m_wasHit && m_step < MAX_STEPS;
}

void MissionProcessor::Step()
{
  if (m_step == MAX_STEPS) {
    throw std::runtime_error(std::format("Simulation exceeded {} steps.\n", MAX_STEPS));
  }

  const auto bytes = m_uart.ReadAvailable();

  for (uint8_t byte : bytes) {
    uint8_t type = 0;
    uint8_t len = 0;
    uint8_t payload[260]{};

    if (m_parser.feed(byte, type, payload, len)) {
      processPacket(dlink::PacketType(type), payload, len);
    }
  }

  if (!m_ammo || !m_droneCfg) {
    std::cout << "Waiting for ammo and drone config...\n";
    return;  // wait for telemetry and ammo
  }

  if (!m_telemetry || m_targets.size() < m_ammo->nTargets) {
    std::cout << "Waiting for telemetry and targets...\n";
    return;  // wait for telemetry and ammo
  }

  m_acceleration = std::pow(m_droneCfg->attackSpeed, 2) / (2.0f * m_droneCfg->accelerationPath);

  TargetFireParams bestTarget{};

  for (int currentTargetIdx = 0; currentTargetIdx < static_cast<int>(m_ammo->nTargets); ++currentTargetIdx) {
    BallisticsSolverContext context{.targetIdx = currentTargetIdx,
                                    .ammo = *m_ammo,
                                    .droneCfg = *m_droneCfg,
                                    .telemetry = *m_telemetry,
                                    .targetPos = m_targets.at(currentTargetIdx),
                                    .currentTime = m_currentTime,
                                    .acceleration = m_acceleration};
    auto target = m_ballisticSolver->Solve(context);

    if (m_currentTarget.has_value() && m_currentTarget.value() != currentTargetIdx) {
      target.totalTime += m_droneState->GetStopTime();
    }

    if (target.totalTime < bestTarget.totalTime) {
      bestTarget = std::move(target);
    }
  }

  // adjust drone state to target
  m_currentTarget = bestTarget.idx;
  const auto targetDir = bestTarget.releasePoint - Coord(m_telemetry->x, m_telemetry->y);
  const double targetAngle = NormalizeAngle180(std::atan2(targetDir.y, targetDir.x));

  DroneStateContext ctx{.droneCfg = *m_droneCfg, .telemetry = *m_telemetry, .targetAngle = targetAngle, .acceleration = m_acceleration};
  auto nextState = m_droneState->Execute(ctx);

  // send control for current state
  m_control = m_droneState->GetControl();
  sendControl();

  const auto prevDroneStateIdx = m_droneState->GetIdx();

  if (nextState) {
    m_droneState = std::move(nextState);
  }

  m_logger->RecordStep(*m_telemetry, bestTarget, prevDroneStateIdx);

  const Coord bestTargetPos{m_targets.at(bestTarget.idx).x, m_targets.at(bestTarget.idx).y};

  // release point
  if ((bestTarget.releasePoint - bestTargetPos).Length() <= 0.25 * m_ammo->hitRadius) {
    m_dropGpio.Set(true);
    usleep(80'000);
    m_dropGpio.Set(false);
    m_wasHit = true;
    return;
  }

  m_currentTime += m_droneCfg->timeStep;
  m_step += 1;
}

void MissionProcessor::sendControl()
{
  m_control.accel = std::clamp(m_control.accel, -1.0f, 1.0f);
  m_control.turnRate = std::clamp(m_control.turnRate, -1.0f, 1.0f);

  uint8_t out[64]{};
  const size_t size = dlink::encode(dlink::PKT_CONTROL, &m_control, static_cast<uint8_t>(sizeof(m_control)), out);

  m_uart.WriteAll(out, size);
}

void MissionProcessor::processPacket(dlink::PacketType type, const uint8_t* payload, uint8_t len)
{
  switch (type) {
    case dlink::PKT_TELEMETRY: {
      if (len != sizeof(dlink::Telemetry)) {
        std::cerr << "Invalid TELEMETRY size: " << static_cast<int>(len) << '\n';
        return;
      }

      dlink::Telemetry t{};
      std::memcpy(&t, payload, sizeof(t));
      m_telemetry = std::move(t);

      return;
    }
    case dlink::PKT_AMMO: {
      if (len != sizeof(dlink::AmmoCfg)) {
        std::cerr << "Invalid AMMO size: " << static_cast<int>(len) << '\n';
        return;
      }

      dlink::AmmoCfg a{};
      std::memcpy(&a, payload, sizeof(a));
      m_ammo = std::move(a);

      std::cout << "AMMO: " << a.name << ", hitRadius=" << a.hitRadius << ", nTargets=" << static_cast<int>(a.nTargets) << '\n';

      return;
    }
    case dlink::PKT_TARGET: {
      if (len != sizeof(dlink::TargetPos)) {
        std::cerr << "Invalid TARGET size: " << static_cast<int>(len) << '\n';
        return;
      }

      dlink::TargetPos p{};
      std::memcpy(&p, payload, sizeof(p));

      m_targets[p.id] = p;
      return;
    }

    case dlink::PKT_RESULT: {
      if (len != sizeof(dlink::Result)) {
        std::cerr << "Invalid RESULT size: " << static_cast<int>(len) << '\n';
        return;
      }

      dlink::Result r{};
      std::memcpy(&r, payload, sizeof(r));

      std::cout << "RESULT: " << (r.hit ? "HIT" : "MISS") << ", targetId=" << static_cast<int>(r.targetId) << ", miss=" << r.miss_m
                << ", drop_t_ms=" << r.drop_t_ms << '\n';

      return;
    }

    case dlink::PKT_CONFIG: {
      if (len != sizeof(dlink::DroneCfg)) {
        std::cerr << "Invalid CONFIG size: " << static_cast<int>(len) << '\n';
        return;
      }

      dlink::DroneCfg cfg{};
      std::memcpy(&cfg, payload, sizeof(cfg));
      m_droneCfg = std::move(cfg);

      std::cout << "CONFIG: attackSpeed=" << cfg.attackSpeed << ", accelerationPath=" << cfg.accelerationPath
                << ", angularSpeed=" << cfg.angularSpeed << ", turnThreshold=" << cfg.turnThreshold << ", timeStep=" << cfg.timeStep
                << ", timeScale=" << cfg.timeScale << '\n';

      return;
    }
    default:
      std::cerr << "Unknown packet type: " << static_cast<int>(type) << '\n';
  }
}