#pragma once

#include "interfaces/IDroneState.h"

class StateStopped : public IDroneState {
public:
  IDroneStatePtr Execute(DroneStateContext& ctx) override;

  double GetStopTime() const override { return 0.0; }

  std::string_view GetName() const override { return "Stopped"; }

  int GetIdx() const override { return 0; }
};
