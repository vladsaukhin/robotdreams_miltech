#pragma once

#include "interfaces/IDroneState.h"

class StateAccelerating : public IDroneState {
public:
  IDroneStatePtr Execute(DroneStateContext& ctx) override;

  double GetStopTime() const override { return m_stopTime; }

  std::string_view GetName() const override { return "Accelerating"; }

  int GetIdx() const override { return 1; }

private:
  double m_stopTime{};
};
