#include <exception>
#include <iostream>
#include <string_view>

#include "config/ConfigFactory.h"
#include "providers/ProviderFactory.h"
#include "providers/ThreadSafeJsonTargetProvider.h"
#include "solvers/SolverFactory.h"
#include "loggers/LoggerFactory.h"

#include "MissionProcessor.h"

int main(int argc, char** argv)
{
  // The program expects exactly one argument: a path to data folder
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_dir_with_files>\n";
    return 1;
  }

  auto configLoader = CreateLoader(ConfigLoaderType::JSON_FILE);
  if (!configLoader->Load(argv[1])) {
    std::cerr << "Cannot initialize ConfigLoader." << std::endl;
    return 1;
  }

  // JsonTargetProviderConfig targetProviderConf{.dataFolderPath = argv[1],
  //                                             .arrayTimeStep = configLoader->GetConfig().arrayTimeStep,
  //                                             .simTimeStep = configLoader->GetConfig().simTimeStep};
  // auto targetProvider = CreateTargetProvider(targetProviderConf);

  ThreadSafeJsonTargetProviderConfig targetProviderConfThread{.dataFolderPath = argv[1],
                                                              .arrayTimeStep = configLoader->GetConfig().arrayTimeStep,
                                                              .targetTimeStep = configLoader->GetConfig().targetTimeStep};
  auto targetProvider = CreateTargetProvider(targetProviderConfThread);

  auto logger = CreateLogger(LoggerType::JSON_FILE);
  auto solver = CreateSolver(SolverType::ANALYTICAL);

  try {
    MissionProcessor missionProcessor(std::move(configLoader), std::move(targetProvider), std::move(solver), std::move(logger));

    missionProcessor.Init(argv[1]);

    // Manual run example
    // while (missionProcessor.HasNext()) { missionProcessor.Step(); }
    missionProcessor.AutoRun();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
