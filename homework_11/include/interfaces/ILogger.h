#pragma once

#include <memory>
#include <string_view>

#include "DroneAbstractions.h"

#include "drone_link.h"

class ILogger {
public:
  virtual ~ILogger() = default;

public:
  virtual void RecordStep(const dlink::Telemetry&, const TargetFireParams&, int droneStateIdx) = 0;
  virtual void DumpLog(std::string_view dataFolderPath, size_t lastStepIdx) = 0;
  virtual void Reset() = 0;
};

using ILoggerPtr = std::unique_ptr<ILogger>;
