#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>
#include <fstream>
#include <format>
#include <sstream>
#include <cmath>

namespace DataReader {

struct TicksInTime {
  int timestamp{};  // miliseconds since the start of the recording
  int fl{};
  int fr{};
  int bl{};
  int br{};
};

std::ostream& operator<<(std::ostream& os, const TicksInTime& tick)
{
  return os << std::format("Tick({} {} {} {} {})", tick.timestamp, tick.fl, tick.fr, tick.bl, tick.br);
}

using TicksInTimeVector = std::vector<TicksInTime>;

bool ReadTicks(std::string_view path, TicksInTimeVector& ticks)
{
  if (path.empty()) {
    std::cerr << "Error: empty path\n";
    return false;
  }

  std::ifstream input(path.data());
  if (!input) {
    std::cerr << std::format("Can't open {}", path) << std::endl;
    return false;
  }

  std::string line;

  constexpr size_t expectedValuesPerLine = 5;
  std::vector<int> valuesInLine;
  valuesInLine.reserve(expectedValuesPerLine);
  int x;

  while (std::getline(input, line)) {
    std::istringstream iss(line);

    while (iss >> x) {
      if (x < 0) {
        std::cerr << "Error: negative value in input\n";
        return false;
      }
      valuesInLine.push_back(x);
    }

    if (valuesInLine.size() != expectedValuesPerLine) {
      std::cerr << "Error: wrong number of values in line\n";
      return false;
    }

    ticks.push_back({valuesInLine[0], valuesInLine[1], valuesInLine[2], valuesInLine[3], valuesInLine[4]});
    valuesInLine.clear();
  }

  return true;
}

void PrintTicks(const TicksInTimeVector& ticks)
{
  for (const auto& tick : ticks) {
    std::cout << std::format("{} {} {} {} {}\n", tick.timestamp, tick.fl, tick.fr, tick.bl, tick.br);
  }
}

}  // namespace DataReader

int main(int argc, char** argv)
{
  // The program expects exactly one argument: a path to telemetry samples.
  if (argc != 2) {
    std::cerr << "usage: ugv_odometry <input_path>\n";
    return 1;
  }

  DataReader::TicksInTimeVector ticks;
  if (!DataReader::ReadTicks(argv[1], ticks)) {
    return 1;
  }

  if (ticks.size() < 2) {
    std::cerr << "Error: not enough data from input\n";
    return 1;
  }

  constexpr double ticksPerRevolution = 1024.0;
  constexpr double wheelRadius = 0.3;  // m
  constexpr double wheelbase = 1.0;    // m

  double x = 0.0;
  double y = 0.0;
  double theta = 0.0;

  for (size_t i = 1; i < ticks.size(); ++i) {
    const auto& prevTick = ticks[i - 1];
    const auto& tick = ticks[i];
    // std::cout << prevTick << std::endl << tick << std::endl;
    const double d_left = (tick.fl - prevTick.fl + tick.bl - prevTick.bl) / 2.0;
    const double d_right = (tick.fr - prevTick.fr + tick.br - prevTick.br) / 2.0;

    // std::cout << std::format("time: {} d_left: {} d_right: {}\n", tick.timestamp, d_left, d_right);

    const double distance_per_tick = 2 * M_PI * wheelRadius / ticksPerRevolution;

    const double dL = d_left * distance_per_tick;
    const double dR = d_right * distance_per_tick;

    // std::cout << std::format("distance_per_tick = {} dL = {} dR = {}\n", distance_per_tick, dL, dR);

    const double d = (dL + dR) / 2.0;
    const double dtheta = (dR - dL) / wheelbase;

    // std::cout << std::format("d = {} dtheta = {}\n", d, dtheta);
    // std::cout << std::format("cos = {} sin = {}\n", cos(theta + dtheta / 2.0), sin(theta + dtheta / 2.0));
    x += d * cos(theta + dtheta / 2.0);
    y += d * sin(theta + dtheta / 2.0);
    theta += dtheta;

    std::cout << std::format("{} {:.2f} {:.2f} {:.2f}\n", tick.timestamp, x, y, theta);
  }

  return 0;
}
