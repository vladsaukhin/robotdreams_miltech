#pragma once

#include "interfaces/IDroneState.h"

class StateMoving : public IDroneState {
public:
  IDroneStatePtr Execute(DroneStateContext& ctx) override;

  double GetStopTime() const override { return m_stopTime; }

  std::string_view GetName() const override { return "Moving"; }

  int GetIdx() const override { return 4; }

  dlink::Control GetControl() const override { return m_control; }

private:
  double m_stopTime{};
  dlink::Control m_control{};
};
