#include "GpioOutput.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace {
std::string toChipPath(const std::string& chip)
{
  if (chip.rfind("/dev/", 0) == 0) {
    return chip.data();
  }

  return "/dev/" + chip;
}

}  // namespace

GpioOutput::GpioOutput(const std::string& chipName, unsigned int line, [[maybe_unused]] const char* consumer)
  : m_chipPath(toChipPath(chipName))
  , m_line(line)
{
#if defined(__aarch64__)
  gpiod_chip* chip = gpiod_chip_open(m_chipPath.c_str());
  if (!chip) {
    throw std::runtime_error("gpiod_chip_open failed for " + m_chipPath + ": " + std::strerror(errno));
  }

  gpiod_line_settings* settings = gpiod_line_settings_new();
  if (!settings) {
    gpiod_chip_close(chip);
    throw std::runtime_error("gpiod_line_settings_new failed");
  }

  gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
  gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

  gpiod_line_config* lineConfig = gpiod_line_config_new();
  if (!lineConfig) {
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    throw std::runtime_error("gpiod_line_config_new failed");
  }

  unsigned int offsets[] = {m_line};

  if (gpiod_line_config_add_line_settings(lineConfig, offsets, 1, settings) < 0) {
    gpiod_line_config_free(lineConfig);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    throw std::runtime_error("gpiod_line_config_add_line_settings failed: " + std::string(std::strerror(errno)));
  }

  gpiod_request_config* requestConfig = gpiod_request_config_new();
  if (!requestConfig) {
    gpiod_line_config_free(lineConfig);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    throw std::runtime_error("gpiod_request_config_new failed");
  }

  gpiod_request_config_set_consumer(requestConfig, consumer);

  m_request = gpiod_chip_request_lines(chip, requestConfig, lineConfig);

  gpiod_request_config_free(requestConfig);
  gpiod_line_config_free(lineConfig);
  gpiod_line_settings_free(settings);
  gpiod_chip_close(chip);

  if (!m_request) {
    throw std::runtime_error("gpiod_chip_request_lines failed for line " + std::to_string(m_line) + ": " + std::strerror(errno));
  }
#endif
}

GpioOutput::~GpioOutput()
{
#if defined(__aarch64__)

  if (m_request) {
    Set(false);
    gpiod_line_request_release(m_request);
  }
#endif
}

void GpioOutput::Set([[maybe_unused]] bool active)
{
#if defined(__aarch64__)
  const auto value = active ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;

  if (gpiod_line_request_set_value(m_request, m_line, value) < 0) {
    throw std::runtime_error("gpiod_line_request_set_value failed: " + std::string(std::strerror(errno)));
  }
#endif
}
