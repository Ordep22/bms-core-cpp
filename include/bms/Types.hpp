#pragma once

#include <cstdint>

namespace bms {

// Modos de operação do sistema
enum class SystemState : uint8_t {
    Boot = 0,
    Nominal,
    EmergencyFailsafe
};

// Flags de falha para auditoria
enum class FaultCode : uint16_t {
    None                = 0x0000,
    OverVoltage         = 0x0001,
    UnderVoltage        = 0x0002,
    OverCurrent         = 0x0004,
    OverTemperature     = 0x0008,
    CommunicationLoss   = 0x0010,
    SensorFault         = 0x0020
};

// Estrutura de telemetria estática (pronta para RTOS)
struct BatteryTelemetry {
    uint32_t timestamp_ms{0};
    float voltage_v{0.0f};
    float current_a{0.0f};
    float temperature_c{0.0f};
    float soc_percentage{0.0f};
    float soh_percentage{0.0f};
    float rul_cycles{0.0f};
};

// Limites operacionais de segurança
struct SafetyThresholds {
    float max_voltage_v{4.2f};
    float min_voltage_v{2.8f};
    float max_current_a{30.0f};
    float max_temperature_c{60.0f};
};

} // namespace bms