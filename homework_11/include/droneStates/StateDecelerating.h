#pragma once

#include "interfaces/IDroneState.h"

class StateDecelerating : public IDroneState {
public:
  IDroneStatePtr Execute(DroneStateContext& ctx) override;

  double GetStopTime() const override { return m_stopTime; }

  std::string_view GetName() const override { return "Decelerating"; }

  int GetIdx() const override { return 2; }

  dlink::Control GetControl() const override { return m_control; }

private:
  double m_stopTime{};
  dlink::Control m_control{};
};
