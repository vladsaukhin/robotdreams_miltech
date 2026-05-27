#include <exception>
#include <iostream>

#include "config/ConfigFactory.h"
#include "providers/ProviderFactory.h"
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
  auto targetLoader = CreateTargetLoader(TargetLoaderType::JSON_FILE);
  auto logger = CreateLogger(LoggerType::JSON_FILE);
  auto solver = CreateSolver(SolverType::ANALYTICAL);

  try {
    MissionProcessor missionProcessor(std::move(configLoader), std::move(targetLoader), std::move(solver), std::move(logger));

    missionProcessor.Init(argv[1]);

    missionProcessor.ProcessMission();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
