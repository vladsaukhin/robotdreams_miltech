#include "telemetry/telemetry.hpp"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <format>
#include <cerrno>

// Debugging exercise notes:
// this file intentionally contains four runtime defects.
// The defects are related to malformed input shape, invalid numeric values,
// unsafe time deltas, and empty logs. Exact locations are not marked on purpose.

namespace {

constexpr int EXPECTED_FIELD_COUNT{7};
constexpr int MAX_LINE_LENGTH{256};

constexpr int RETURN_OK{0};
constexpr int RETURN_INVALID_FIELD{-1};
constexpr int RETURN_NOT_ENOUGH_FIELDS{-2};
constexpr int RETURN_TOO_MANY_FIELDS{-3};

bool is_space(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Splits a line into fields by whitespace and null-terminates them.
// returns the number of fields found, or -3 (RETURN_TOO_MANY_FIELDS) if the field count exceeds max_fields.
int split_line(char line[], char* fields[], int max_fields)
{
  int count = 0;
  char* cursor = line;

  while (*cursor != '\0') {
    while (is_space(*cursor)) {
      *cursor = '\0';
      ++cursor;
    }

    if (*cursor == '\0') {
      break;
    }

    if (count < max_fields) {
      fields[count] = cursor;
      ++count;
    }
    else {
      return RETURN_TOO_MANY_FIELDS;
    }

    while (*cursor != '\0' && !is_space(*cursor)) {
      ++cursor;
    }
  }

  return count;
}

template <typename T>
  requires(std::is_same_v<T, int> || std::is_same_v<T, long> || std::is_same_v<T, double> || std::is_same_v<T, float>)
bool parse_number(const char* text, T& value)
{
  errno = 0;
  char* end = nullptr;

  if constexpr (std::is_same_v<T, double>) {
    value = std::strtod(text, &end);
  }
  else if constexpr (std::is_same_v<T, float>) {
    value = std::strtof(text, &end);
  }
  else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, long>) {
    const long temp = std::strtol(text, &end, 10);

    if constexpr (std::is_same_v<T, int>) {
      if (temp < std::numeric_limits<int>::min() || temp > std::numeric_limits<int>::max()) {
        return false;
      }

      value = static_cast<int>(temp);
    }
    else  // long
    {
      value = temp;
    }
  }

  return end != text && *end == '\0' && errno != ERANGE;
}

// parses a line of text into a Frame struct, validating the field count and numeric values.
// returns 0 if the frame was successfully parsed
//        -1 (RETURN_INVALID_FIELD) if the frame field is invalid
//        -2 (RETURN_NOT_ENOUGH_FIELDS) if there are not enough fields
//        -3 (RETURN_TOO_MANY_FIELDS) if there are too many fields
int parse_frame(char line[], Frame& frame)
{
  char* fields[EXPECTED_FIELD_COUNT]{};
  const int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT);

  if (field_count == RETURN_TOO_MANY_FIELDS) {
    return RETURN_TOO_MANY_FIELDS;
  }
  if (field_count != EXPECTED_FIELD_COUNT) {
    return RETURN_NOT_ENOUGH_FIELDS;
  }

  if (!parse_number(fields[0], frame.timestamp_ms)) {
    return RETURN_INVALID_FIELD;
  }
  if (!parse_number(fields[1], frame.seq)) {
    return RETURN_INVALID_FIELD;
  }
  if (!parse_number(fields[2], frame.voltage_v)) {
    return RETURN_INVALID_FIELD;
  }
  if (!parse_number(fields[3], frame.current_a)) {
    return RETURN_INVALID_FIELD;
  }
  if (!parse_number(fields[4], frame.temperature_c)) {
    return RETURN_INVALID_FIELD;
  }
  if (!parse_number(fields[5], frame.gps_fix)) {
    return RETURN_INVALID_FIELD;
  }
  if (!parse_number(fields[6], frame.satellites)) {
    return RETURN_INVALID_FIELD;
  }

  return RETURN_OK;
}

bool validate_frame_sequential(int line_number, const Frame& frame, const Frame& previous_frame)
{
  if (previous_frame.timestamp_ms >= frame.timestamp_ms) {
    std::cerr << std::format("error: non-increasing timestamp at line {}\n", line_number);
    return false;
  }
  if (frame.seq != previous_frame.seq + 1) {
    std::cerr << std::format("error: non-sequential frame sequence at line {}\n", line_number);
    return false;
  }
  return true;
}

bool validate_frame_limits(int line_number, const Frame& frame)
{
  if (frame.voltage_v < 0.0) {
    std::cerr << std::format("error: negative voltage value at line {}\n", line_number);
    return false;
  }
  if (frame.current_a < 0.0) {
    std::cerr << std::format("error: negative current value at line {}\n", line_number);
    return false;
  }
  if (frame.temperature_c < -40.0 || frame.temperature_c > 125.0) {
    std::cerr << std::format("error: out-of-range [-40.0; 125.0] temperature value at line {}\n", line_number);
    return false;
  }
  if (frame.gps_fix != 0 && frame.gps_fix != 1) {
    std::cerr << std::format("error: invalid GPS fix value at line {}\n", line_number);
    return false;
  }
  if (frame.satellites < 0) {
    std::cerr << std::format("error: negative satellites value at line {}\n", line_number);
    return false;
  }
  return true;
}

double compute_frame_rate_hz(const Frame frames[], int frame_count)
{
  if (frame_count < 2) {
    std::cerr << "warning: not enough frames to compute frame rate\n";
    return 0.0;
  }

  const double elapsed_ms = frames[frame_count - 1].timestamp_ms - frames[0].timestamp_ms;

  if (elapsed_ms <= 0.0) {
    return 0.0;
  }

  return static_cast<double>((frame_count - 1) * 1000 / elapsed_ms);
}

}  // namespace

int read_frames(const char* path, Frame frames[], int max_frames)
{
  if (max_frames <= 0) {
    std::cerr << "error: max_frames must be greater than 0\n";
    return -1;
  }

  if (path == nullptr) {
    std::cerr << "error: input path is null\n";
    return -1;
  }

  std::ifstream input{path};
  if (!input) {
    std::cerr << "error: failed to open input file: " << path << '\n';
    return -1;
  }

  int frame_count = 0;
  char line[MAX_LINE_LENGTH];

  int line_number = 0;

  while (input.getline(line, MAX_LINE_LENGTH)) {
    ++line_number;

    if (line[0] == '\0') {
      continue;
    }

    bool validFrame = false;

    auto& frame = frames[frame_count];

    if (frame_count < max_frames) {
      const int parse_result = parse_frame(line, frame);
      switch (parse_result) {
        case RETURN_INVALID_FIELD: {
          std::cerr << std::format("error: invalid frame at line {}\n", line_number);
          break;
        }
        case RETURN_NOT_ENOUGH_FIELDS: {
          std::cerr << std::format("error: not enough fields at line {}\n", line_number);
          break;
        }
        case RETURN_TOO_MANY_FIELDS: {
          std::cerr << std::format("error: too many fields at line {}\n", line_number);
          break;
        }
        case RETURN_OK: {
          validFrame = true;

          // validate frame values
          if (frame_count > 0) {
            validFrame &= validate_frame_sequential(line_number, frame, frames[frame_count - 1]);
          }
          validFrame &= validate_frame_limits(line_number, frame);

          break;
        }
      }

      if (!validFrame) {
        return -1;
      }
      ++frame_count;
    }
    else {
      std::cerr << std::format("error: maximum frame count of {} exceeded\n", max_frames);
      return -1;
    }
  }

  if (frame_count == 0) {
    std::cerr << "warning: no frames found in input file: " << path << '\n';
  }

  return frame_count;
}

Summary summarize(const Frame frames[], int frame_count)
{
  Summary summary{};

  if (frame_count == 0) {
    std::cerr << "warning: no frames to summarize\n";
    return summary;
  }

  summary.frames_total = frame_count;
  summary.voltage_min = frames[0].voltage_v;
  summary.voltage_max = frames[0].voltage_v;
  summary.low_voltage_frames = 0;

  double temperature_sum = 0.0;

  for (int i = 0; i < frame_count; ++i) {
    if (frames[i].voltage_v < summary.voltage_min) {
      summary.voltage_min = frames[i].voltage_v;
    }

    if (frames[i].voltage_v > summary.voltage_max) {
      summary.voltage_max = frames[i].voltage_v;
    }

    temperature_sum += frames[i].temperature_c;

    if (frames[i].voltage_v < 22.0) {
      ++summary.low_voltage_frames;
    }
  }

  const int temperature_tenths = static_cast<int>(temperature_sum * 10.0) / frame_count;
  summary.temperature_avg = static_cast<double>(temperature_tenths) / 10.0;
  summary.frame_rate_hz = compute_frame_rate_hz(frames, frame_count);
  return summary;
}

void print_summary(const Summary& summary)
{
  std::cout << "frames_total " << summary.frames_total << '\n';
  std::cout << "voltage_min " << summary.voltage_min << '\n';
  std::cout << "voltage_max " << summary.voltage_max << '\n';
  std::cout << "temperature_avg " << summary.temperature_avg << '\n';
  std::cout << "low_voltage_frames " << summary.low_voltage_frames << '\n';
  std::cout << "frame_rate_hz " << summary.frame_rate_hz << '\n';
}
