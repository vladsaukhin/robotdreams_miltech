#pragma once

#include <gpiod.h>

#include <string>

class GpioOutput {
public:
  GpioOutput(const std::string& chipName, unsigned int line, const char* consumer);

  ~GpioOutput();

  GpioOutput(const GpioOutput&) = delete;
  GpioOutput& operator=(const GpioOutput&) = delete;

  GpioOutput(GpioOutput&&) = delete;
  GpioOutput& operator=(GpioOutput&&) = delete;

  void Set(bool active);

private:
  std::string m_chipPath;
  unsigned int m_line = 0;

#if defined(__aarch64__)
  gpiod_line_request* m_request = nullptr;
#endif
};
