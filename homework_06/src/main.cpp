#include <iostream>
#include <fstream>

#include "ballistics/ballistics.h"

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_input_file>\n";
    return 1;
  }

  auto input_params_opt = ParseInputParams(argv[1]);
  if (!input_params_opt) {
    return 1;
  }

  auto ammo_params_opt = GetAmmoParams(input_params_opt->ammo_name);
  if (!ammo_params_opt) {
    std::cerr << "Unknown ammo type: " << input_params_opt->ammo_name << "\n";
    return 1;
  }

  auto time_of_flight_opt = GetTimeOfFlight(*ammo_params_opt, input_params_opt->altitude, input_params_opt->attackSpeed);
  if (!time_of_flight_opt) {
    std::cerr << "Failed to calculate time of flight.\n";
    return 1;
  }

  auto horizontal_flight_distance_opt = GetHorizontalFlightDistance(*ammo_params_opt, input_params_opt->attackSpeed, *time_of_flight_opt);
  if (!horizontal_flight_distance_opt) {
    std::cerr << "Failed to calculate horizontal flight distance.\n";
    return 1;
  }

  auto balistics_solution = GetBallistics(*input_params_opt, *time_of_flight_opt, *horizontal_flight_distance_opt);

  // save output points
  {
    std::ofstream output("output.txt");
    if (!output) {
      std::cerr << "Cannot open output.txt file for writing.\n";
      return 1;
    }

    if (balistics_solution.maneuverPoint) {
      output << balistics_solution.maneuverPoint->x << ' ' << balistics_solution.maneuverPoint->y << '\n';
    }

    output << balistics_solution.firePoint.x << ' ' << balistics_solution.firePoint.y << '\n';
  }

  return 0;
}
