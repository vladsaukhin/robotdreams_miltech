#pragma once

#include "interfaces/IDroneState.h"

class StateStopped : public IDroneState {
public:
  IDroneStatePtr Execute(DroneStateContext& ctx) override;

  double GetStopTime() const override { return 0.0; }

  std::string_view GetName() const override { return "Stopped"; }

  int GetIdx() const override { return 0; }

  dlink::Control GetControl() const override { return dlink::Control{.accel = 0.0f, .turnRate = 0.0f}; };
};
