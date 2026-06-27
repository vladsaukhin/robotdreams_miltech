#include "MissionProcessor.h"

#include <format>

#include "DronePhysics.h"
#include "Utils.h"

#include "DroneConfig.h"
#include "DroneAbstractions.h"

#include "droneStates/StateStopped.h"
#include "interfaces/DroneStateContext.h"
#include "interfaces/ITargetProvider.h"

namespace {

constexpr size_t MAX_STEPS{10'000};

}  // namespace

MissionProcessor::MissionProcessor(IConfigLoaderPtr configLoader,
                                   ITargetProviderPtr targetProvider,
                                   IBallisticSolverPtr ballisticSolver,
                                   ILoggerPtr logger)
  : m_configLoader(std::move(configLoader))
  , m_targetProvider(std::move(targetProvider))
  , m_ballisticSolver(std::move(ballisticSolver))
  , m_logger(std::move(logger))
  , m_drone(*m_configLoader)
{
  if (!m_configLoader) {
    throw std::logic_error("ConfigLoader is not initialized");
  }

  if (!m_targetProvider) {
    throw std::logic_error("TargetProvider is not initialized");
  }

  if (!m_ballisticSolver) {
    throw std::logic_error("BallisticSolver is not initialized");
  }

  if (!m_logger) {
    throw std::logic_error("Logger is not initialized");
  }
}

MissionProcessor::~MissionProcessor()
{
  if (m_worker.joinable()) {
    m_worker.join();
  }

  m_targetProvider->Stop();
  m_drone.Stop();

  if (m_step < MAX_STEPS) {
    m_logger->DumpLog(m_dataFolderPath, m_step);
  }
}

void MissionProcessor::Init(std::string_view dataFolderPath)
{
  m_droneState = std::make_unique<StateStopped>();

  m_acceleration = std::pow(m_configLoader->GetConfig().attackSpeed, 2) / (2.0f * m_configLoader->GetConfig().accelerationPath);
  m_dataFolderPath = dataFolderPath;

  m_initialized = true;

  m_drone.Start();
  m_targetProvider->Start();
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
  m_drone.Reset();
  m_droneState = std::make_unique<StateStopped>();
  m_currentTime = 0.0;
  m_step = 0;

  m_logger->Reset();
}

void MissionProcessor::AutoRun()
{
  if (!m_initialized) {
    throw std::logic_error("MissionProcessor is not initialized");
  }

  if (m_worker.joinable()) {
    return;  // already started
  }

  m_worker = std::thread(&MissionProcessor::runWorker, this);

  m_worker.join();
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

  TargetFireParams bestTarget{};

  auto telemetry = m_drone.GetTelemetry();
  for (int currentTargetIdx = 0; currentTargetIdx < static_cast<int>(m_targetProvider->GetTargetCount()); ++currentTargetIdx) {
    BallisticsSolverContext context{.targetIdx = currentTargetIdx,
                                    .conf = *m_configLoader,
                                    .telemetry = telemetry,
                                    .targetProvider = *m_targetProvider,
                                    .currentTime = m_currentTime,
                                    .acceleration = m_acceleration,
                                    .dataPath = m_dataFolderPath};
    auto target = m_ballisticSolver->Solve(context);

    if (telemetry.currentTarget != UNDEFINED_TARGET_ID && telemetry.currentTarget != currentTargetIdx) {
      target.totalTime += m_droneState->GetStopTime();
    }

    if (target.totalTime < bestTarget.totalTime) {
      bestTarget = std::move(target);
    }
  }

  // adjust drone state to target
  telemetry.currentTarget = bestTarget.idx;
  const auto targetDir = bestTarget.releasePoint - telemetry.position;
  telemetry.targetDir = NormalizeAngle180(std::atan2(targetDir.y, targetDir.x));

  DroneStateContext ctx{.telemetry = telemetry, .cfg = conf, .acceleration = m_acceleration};
  auto nextState = m_droneState->Execute(ctx);

  m_drone.PostUpdater([telemetry](DroneTelemetry& orig) { orig = telemetry; });

  // move
  const auto prevDroneStateIdx = m_droneState->GetIdx();
  if (nextState) {
    m_droneState = std::move(nextState);
  }

  m_logger->RecordStep(telemetry, bestTarget, prevDroneStateIdx);

  // release point
  if ((bestTarget.releasePoint - telemetry.position).Length() <= 0.25 * conf.hitRadius) {
    m_wasHit = true;
    return;
  }

  m_currentTime += conf.simTimeStep;
  m_step += 1;
}

void MissionProcessor::runWorker()
{
  const auto sleepTime = m_configLoader->GetConfig().simTimeStep / m_configLoader->GetConfig().timeScale;
  while (HasNext()) {
    Step();

    std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
  }
}
