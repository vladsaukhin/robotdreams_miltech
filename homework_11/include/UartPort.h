#pragma once

#include <cstdint>
#include <string>
#include <vector>

class UartPort {
public:
  explicit UartPort(const std::string& device);
  ~UartPort();

  UartPort(const UartPort&) = delete;
  UartPort& operator=(const UartPort&) = delete;

  UartPort(UartPort&& other) noexcept;
  UartPort& operator=(UartPort&& other) noexcept;

  std::vector<uint8_t> ReadAvailable();
  void WriteAll(const uint8_t* data, size_t size);
  void Flush();

  int Fd() const { return m_fd; }

private:
  void Open(const std::string& device);
  void Close();

private:
  int m_fd = -1;
};