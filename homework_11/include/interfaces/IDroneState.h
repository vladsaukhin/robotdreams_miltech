#pragma once

#include <memory>
#include <string_view>

#include "drone_link.h"

struct DroneStateContext;

class IDroneState {
public:
  virtual ~IDroneState() = default;

  // Виконати логіку стану, повернути наступний стан.
  // Якщо стан не змінився — повернути nullptr
  // (головний цикл залишить поточний).
  virtual std::unique_ptr<IDroneState> Execute(DroneStateContext& ctx) = 0;

  virtual double GetStopTime() const = 0;

  virtual dlink::Control GetControl() const = 0;

  virtual std::string_view GetName() const = 0;
  virtual int GetIdx() const = 0;  // 0 - 4, for backward compatibility with old simulation logs
};

using IDroneStatePtr = std::unique_ptr<IDroneState>;