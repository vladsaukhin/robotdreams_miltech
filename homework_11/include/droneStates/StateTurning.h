#pragma once

#include "interfaces/IDroneState.h"

class StateTurning : public IDroneState {
public:
  IDroneStatePtr Execute(DroneStateContext& ctx) override;

  double GetStopTime() const override { return m_stopTime; }

  std::string_view GetName() const override { return "Turning"; }

  int GetIdx() const override { return 3; }

  dlink::Control GetControl() const override { return m_control; }

private:
  double m_stopTime{};
  dlink::Control m_control{};
};
