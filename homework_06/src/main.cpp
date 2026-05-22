#include <iostream>
#include <fstream>

#include "ballistics/ballistics.h"

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_input_file>\n";
    return 1;
  }

  auto inputParamsOpt = ParseInputParams(argv[1]);
  if (!inputParamsOpt) {
    return 1;
  }

  auto ammoParamsOpt = GetAmmoParams(inputParamsOpt->ammo_name);
  if (!ammoParamsOpt) {
    std::cerr << "Unknown ammo type: " << inputParamsOpt->ammo_name << "\n";
    return 1;
  }

  auto timeOfFlightOpt = GetTimeOfFlight(*ammoParamsOpt, inputParamsOpt->altitude, inputParamsOpt->attackSpeed);
  if (!timeOfFlightOpt) {
    std::cerr << "Failed to calculate time of flight.\n";
    return 1;
  }

  auto horizontalFlightDistanceOpt = GetHorizontalFlightDistance(*ammoParamsOpt, inputParamsOpt->attackSpeed, *timeOfFlightOpt);
  if (!horizontalFlightDistanceOpt) {
    std::cerr << "Failed to calculate horizontal flight distance.\n";
    return 1;
  }

  auto balisticsSolution = GetBallistics(*inputParamsOpt, *timeOfFlightOpt, *horizontalFlightDistanceOpt);

  // save output points
  {
    std::ofstream output("output.txt");
    if (!output) {
      std::cerr << "Cannot open output.txt file for writing.\n";
      return 1;
    }

    if (balisticsSolution.maneuverPoint) {
      output << balisticsSolution.maneuverPoint->x << ' ' << balisticsSolution.maneuverPoint->y << '\n';
    }

    output << balisticsSolution.firePoint.x << ' ' << balisticsSolution.firePoint.y << '\n';
  }

  return 0;
}
