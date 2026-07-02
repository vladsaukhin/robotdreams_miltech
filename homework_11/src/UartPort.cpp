#include "UartPort.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace {

std::runtime_error makeError(const std::string& msg)
{
  return std::runtime_error(msg + ": " + std::strerror(errno));
}

}  // namespace

UartPort::UartPort(const std::string& device)
{
  Open(device);
}

UartPort::~UartPort()
{
  Close();
}

UartPort::UartPort(UartPort&& other) noexcept
  : m_fd(other.m_fd)
{
  other.m_fd = -1;
}

UartPort& UartPort::operator=(UartPort&& other) noexcept
{
  if (this != &other) {
    Close();
    m_fd = other.m_fd;
    other.m_fd = -1;
  }

  return *this;
}

void UartPort::Open(const std::string& device)
{
  m_fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (m_fd < 0) {
    throw makeError("open UART failed for " + device);
  }

  termios tio{};
  if (tcgetattr(m_fd, &tio) < 0) {
    Close();
    throw makeError("tcgetattr failed");
  }

  cfmakeraw(&tio);

  if (cfsetispeed(&tio, B115200) < 0) {
    Close();
    throw makeError("cfsetispeed failed");
  }

  if (cfsetospeed(&tio, B115200) < 0) {
    Close();
    throw makeError("cfsetospeed failed");
  }

  tio.c_cflag |= CLOCAL;
  tio.c_cflag |= CREAD;

  tio.c_cflag &= ~PARENB;  // no parity
  tio.c_cflag &= ~CSTOPB;  // 1 stop bit
  tio.c_cflag &= ~CSIZE;
  tio.c_cflag |= CS8;  // 8 data bits

  // Non-blocking behavior
  tio.c_cc[VMIN] = 0;
  tio.c_cc[VTIME] = 0;

  if (tcsetattr(m_fd, TCSANOW, &tio) < 0) {
    Close();
    throw makeError("tcsetattr failed");
  }

  Flush();
}

void UartPort::Close()
{
  if (m_fd >= 0) {
    close(m_fd);
    m_fd = -1;
  }
}

void UartPort::Flush()
{
  if (m_fd < 0) {
    throw std::runtime_error("UART is not open");
  }

  if (tcflush(m_fd, TCIOFLUSH) < 0) {
    throw makeError("tcflush failed");
  }
}

std::vector<uint8_t> UartPort::ReadAvailable()
{
  if (m_fd < 0) {
    throw std::runtime_error("UART is not open");
  }

  uint8_t buffer[512]{};
  const ssize_t n = read(m_fd, buffer, sizeof(buffer));

  if (n > 0) {
    return std::vector<uint8_t>(buffer, buffer + n);
  }

  if (n == 0) {
    return {};
  }

  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    return {};
  }

  throw makeError("UART read failed");
}

void UartPort::WriteAll(const uint8_t* data, size_t size)
{
  if (m_fd < 0) {
    throw std::runtime_error("UART is not open");
  }

  size_t writtenTotal = 0;

  while (writtenTotal < size) {
    const ssize_t written = write(m_fd, data + writtenTotal, size - writtenTotal);

    if (written > 0) {
      writtenTotal += static_cast<size_t>(written);
      continue;
    }

    if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      usleep(1000);
      continue;
    }

    throw makeError("UART write failed");
  }
}