#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>
#include <fstream>
#include <format>
#include <sstream>

namespace DataReader {

struct TicksInTime {
  int timestamp{};  // miliseconds since the start of the recording
  int fl{};
  int fr{};
  int bl{};
  int br{};
};

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

namespace Calculations {

constexpr int32_t ticks_per_revolution = 1024;
constexpr double wheel_radius_m = 0.3;
constexpr double wheelbase_m = 1.0;

}  // namespace Calculations

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

  return 0;
}
