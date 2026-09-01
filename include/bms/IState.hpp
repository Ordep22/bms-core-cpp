#pragma once

#include "bms/Types.hpp"

namespace bms {

// Declaração antecipada da controladora
class CoreController;

class IState {
public:
    virtual ~IState() = default;

    virtual void onEntry(CoreController& context) = 0;
    virtual void process(CoreController& context, const BatteryTelemetry& telemetry) = 0;
    virtual void onExit(CoreController& context) = 0;
    virtual SystemState getStateId() const = 0;
};

} // namespace bms