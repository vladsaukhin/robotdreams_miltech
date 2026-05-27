#include "MissionProcessor.h"

#include <format>

#include "Utils.h"

#include "DroneConfig.h"
#include "DroneAbstractions.h"

namespace {

constexpr size_t MAX_STEPS{10'000};
constexpr double gEps{1e-6f};

}  // namespace

MissionProcessor::MissionProcessor(IConfigLoaderPtr configLoader,
                                   ITargetLoaderPtr targetLoader,
                                   IBallisticSolverPtr ballisticSolver,
                                   ILoggerPtr logger)
{
  if (!configLoader) {
    throw std::logic_error("ConfigLoader is not initialized");
  }

  if (!targetLoader) {
    throw std::logic_error("TargetLoader is not initialized");
  }

  if (!ballisticSolver) {
    throw std::logic_error("BallisticSolver is not initialized");
  }

  if (!logger) {
    throw std::logic_error("Logger is not initialized");
  }

  m_configLoader = std::move(configLoader);
  m_targetLoader = std::move(targetLoader);
  m_ballisticSolver = std::move(ballisticSolver);
  m_logger = std::move(logger);
}

void MissionProcessor::Init(std::string_view dataFolderPath)
{
  if (!m_configLoader->Load(dataFolderPath)) {
    throw std::logic_error("Cannot initialize ConfigLoader");
  }

  if (!m_targetLoader->Load(dataFolderPath)) {
    throw std::logic_error("Cannot initialize TargetLoader");
  }

  m_drone.position = m_configLoader->GetConfig().startPos;
  m_drone.diraction = m_configLoader->GetConfig().initialDir;
  m_drone.state = DroneState::STOPPED;

  m_acceleration = std::pow(m_configLoader->GetConfig().attackSpeed, 2) / (2.0f * m_configLoader->GetConfig().accelerationPath);
  m_dataFolderPath = dataFolderPath;

  m_initialized = true;
}

void MissionProcessor::ChangeSolver(IBallisticSolverPtr ballisticSolver)
{
  if (!ballisticSolver) {
    throw std::logic_error("New BallisticSolver is not initialized");
  }
  m_ballisticSolver = std::move(ballisticSolver);
}

void MissionProcessor::Reset()
{
  m_drone.position = m_configLoader->GetConfig().startPos;
  m_drone.diraction = m_configLoader->GetConfig().initialDir;
  m_drone.state = DroneState::STOPPED;
  m_currentTime = 0.0;
  m_step = 0;

  m_logger->Reset();
}

void MissionProcessor::ProcessMission()
{
  if (!m_initialized) {
    throw std::logic_error("MissionProcessor is not initialized");
  }

  const auto& conf = m_configLoader->GetConfig();

  while (m_step < MAX_STEPS) {
    Target bestTarget{};
    m_currentProcessedTargetID = 0;

    while (hasNext()) {
      auto target = step();
      if (target.totalTime < bestTarget.totalTime) {
        bestTarget = std::move(target);
      }
    }

    adjustDroneStateToTarget(bestTarget);

    moveDrone();

    m_logger->RecordStep(m_drone, bestTarget);

    // release point
    if (m_drone.state == MOVING && (bestTarget.releasePoint - m_drone.position).Length() <= 0.25 * conf.hitRadius) {
      break;
    }

    // move
    m_currentTime += conf.simTimeStep;
    m_step += 1;
  }

  if (m_step == MAX_STEPS) {
    throw std::runtime_error(std::format("Simulation exceeded {} steps.\n", MAX_STEPS));
  }

  m_logger->DumpLog(m_dataFolderPath, m_step);
}

double MissionProcessor::getStopTime() const
{
  switch (m_drone.state) {
    case STOPPED:
      return 0.0f;

    case ACCELERATING:
    case MOVING:
    case DECELERATING:
      return m_drone.speed / m_acceleration;

    case TURNING:
      return m_drone.turnRemaining;

    default:
      return 0.0f;
  }
}

// required method ?
bool MissionProcessor::hasNext()
{
  return m_currentProcessedTargetID < static_cast<int>(m_targetLoader->GetTargetCount());
}

// required method ?
Target MissionProcessor::step()
{
  BallisticsSolverContext context{.targetIdx = m_currentProcessedTargetID,
                                  .conf = *m_configLoader,
                                  .drone = m_drone,
                                  .targetLoader = *m_targetLoader,
                                  .currentTime = m_currentTime,
                                  .acceleration = m_acceleration};

  auto target = m_ballisticSolver->Solve(context);

  if (m_drone.currentTarget != UNDEFINED_TARGET_ID && m_drone.currentTarget != m_currentProcessedTargetID) {
    target.totalTime += getStopTime();
  }

  m_currentProcessedTargetID++;

  return target;
}

void MissionProcessor::adjustDroneStateToTarget(const Target& target)
{
  const auto& conf = m_configLoader->GetConfig();

  m_drone.currentTarget = target.idx;
  const auto targetDir = target.releasePoint - m_drone.position;
  m_drone.targetDir = NormalizeAngle180(std::atan2(targetDir.y, targetDir.x));

  const double deltaAngle = std::fabs(AngleDiff(m_drone.diraction, m_drone.targetDir));

  if (deltaAngle > conf.turnThreshold) {
    if (m_drone.state == DroneState::MOVING || m_drone.state == DroneState::ACCELERATING) {
      m_drone.state = DECELERATING;  // plan deceleration
    }
    else if (m_drone.state == DroneState::STOPPED) {
      // plan turning
      m_drone.state = TURNING;
      m_drone.turnRemaining = deltaAngle / conf.angularSpeed;
    }
  }
  else {
    // change diraction without stopping
    m_drone.diraction = m_drone.targetDir;

    if (m_drone.speed < conf.attackSpeed - gEps) {
      m_drone.state = ACCELERATING;
    }
    else {
      m_drone.state = MOVING;
    }
  }
}

void MissionProcessor::changeDronePosition(double dt)
{
  const Coord positionToAdd = {std::cos(m_drone.diraction) * m_drone.speed * dt, std::sin(m_drone.diraction) * m_drone.speed * dt};
  m_drone.position += positionToAdd;
}

void MissionProcessor::moveDrone()
{
  const auto& conf = m_configLoader->GetConfig();

  const double dt = conf.simTimeStep;

  switch (m_drone.state) {
    case DroneState::STOPPED: {
      m_drone.speed = 0.0f;
      break;
    }
    case DroneState::ACCELERATING: {
      m_drone.speed += m_acceleration * dt;

      if (m_drone.speed >= conf.attackSpeed) {
        m_drone.speed = conf.attackSpeed;
        m_drone.state = MOVING;
      }

      changeDronePosition(dt);
      break;
    }
    case DroneState::DECELERATING: {
      m_drone.speed -= m_acceleration * dt;

      if (m_drone.speed <= gEps) {
        m_drone.speed = 0.0f;
        m_drone.state = STOPPED;
      }

      changeDronePosition(dt);
      break;
    }
    case DroneState::TURNING: {
      m_drone.speed = 0.0f;

      const double deltaAngle = AngleDiff(m_drone.diraction, m_drone.targetDir);
      const double deltaAngleAbs = std::fabs(deltaAngle);

      const double maxTurn = conf.angularSpeed * dt;

      if (deltaAngleAbs <= maxTurn + gEps) {
        m_drone.diraction = m_drone.targetDir;
        m_drone.turnRemaining = 0.0f;
        m_drone.state = ACCELERATING;
      }
      else {
        const double turnStep = (deltaAngle > 0.0f ? maxTurn : -maxTurn);
        m_drone.diraction = NormalizeAngle180(m_drone.diraction + turnStep);
        m_drone.turnRemaining = (deltaAngleAbs - maxTurn) / conf.angularSpeed;
      }
      break;
    }
    case DroneState::MOVING: {
      changeDronePosition(dt);
      break;
    }
  }
}