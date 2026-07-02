#include <exception>
#include <iostream>

#include "solvers/SolverFactory.h"
#include "loggers/LoggerFactory.h"

#include "ParseArgs.hpp"

#include "MissionProcessor.h"

// checker TX  GPIO14 / pin 8   → student RX GPIO5  / pin 29
// student TX  GPIO4  / pin 7   → checker RX GPIO15 / pin 10

// student START GPIO24 / pin 18 → checker START GPIO27 / pin 13
// student DROP  GPIO23 / pin 16 → checker DROP  GPIO22 / pin 15

// ls -l /dev/ttyAMA*
// ls -l /dev/serial*

// sudo ./checker 1 --hw --uart /dev/serial0 --gpiochip gpiochip0 --start-line 27 --drop-line 22
// ./homework11 --uart /dev/ttyAMA3 --gpiochip gpiochip0 --start-line 24 --drop-line 23
// ./homework11 --u /dev/ttyAMA3 --g gpiochip0 --start-line 24 --drop-line 23

int main(int argc, char** argv)
{
  Args args = ParseArgs(argc, argv);

  auto logger = CreateLogger(LoggerType::JSON_FILE);
  auto solver = CreateSolver(SolverType::ANALYTICAL);

  try {
    MissionProcessor missionProcessor(std::move(args), std::move(solver), std::move(logger));

    missionProcessor.Init(args.dataFolder);

    missionProcessor.AutoRun();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
