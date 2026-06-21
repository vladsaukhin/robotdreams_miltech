#include "MissionProcessor.h"

#include <format>

#include "Utils.h"

#include "DroneConfig.h"
#include "DroneAbstractions.h"

#include "droneStates/StateStopped.h"
#include "interfaces/DroneStateContext.h"

namespace {

constexpr size_t MAX_STEPS{10'000};

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

MissionProcessor::~MissionProcessor()
{
  if (m_step < MAX_STEPS) {
    m_logger->DumpLog(m_dataFolderPath, m_step);
  }
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

  m_drone.state = std::make_unique<StateStopped>();

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
  m_drone.state = std::make_unique<StateStopped>();
  m_currentTime = 0.0;
  m_step = 0;

  m_logger->Reset();
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

  const auto& conf = m_configLoader->GetConfig();

  Target bestTarget{};

  for (int currentTargetIdx = 0; currentTargetIdx < static_cast<int>(m_targetLoader->GetTargetCount()); ++currentTargetIdx) {
    BallisticsSolverContext context{.targetIdx = currentTargetIdx,
                                    .conf = *m_configLoader,
                                    .drone = m_drone,
                                    .targetLoader = *m_targetLoader,
                                    .currentTime = m_currentTime,
                                    .acceleration = m_acceleration};

    auto target = m_ballisticSolver->Solve(context);

    if (m_drone.currentTarget != UNDEFINED_TARGET_ID && m_drone.currentTarget != currentTargetIdx) {
      target.totalTime += m_drone.state->GetStopTime();
    }

    if (target.totalTime < bestTarget.totalTime) {
      bestTarget = std::move(target);
    }
  }

  // adjust drone state to target
  m_drone.currentTarget = bestTarget.idx;
  const auto targetDir = bestTarget.releasePoint - m_drone.position;
  m_drone.targetDir = NormalizeAngle180(std::atan2(targetDir.y, targetDir.x));

  DroneStateContext ctx{.drone = m_drone, .cfg = conf, .acceleration = m_acceleration};
  auto nextState = m_drone.state->Execute(ctx);

  // move
  if (nextState) {
    m_drone.state = std::move(nextState);
  }

  m_logger->RecordStep(m_drone, bestTarget);

  // release point
  if ((bestTarget.releasePoint - m_drone.position).Length() <= 0.25 * conf.hitRadius) {
    m_wasHit = true;
    return;
  }

  m_currentTime += conf.simTimeStep;
  m_step += 1;
}
