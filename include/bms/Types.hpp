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
        None = 0x0000,
        OverVoltage = 0x0001,
        UnderVoltage = 0x0002,
        OverCurrent = 0x0004,
        OverTemperature = 0x0008,
        CommunicationLoss = 0x0010,
        SensorFault = 0x0020
    };

    enum class SafetyEvent {
        None,
        OverVoltage,
        UnderVoltage,
        OverCurrent,
        ThermalRunaway,
        DeadlineViolation
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

    // Regra: UM campo por condição usada no SafetyMonitor::evaluate().
    struct SafetyThresholds {
    float max_voltage_v{4.2f};        // sobretensão:   V > max_voltage_v
    float min_voltage_v{2.8f};        // subtensão:     V < min_voltage_v
    float max_current_a{30.0f};       // sobrecorrente: |I| > max_current_a (D1)
    float max_temperature_c{60.0f};   // Tmax do detector térmico
    float max_dtemp_c_per_s{1.0f};    // θ crítico, em °C/s
    // TODO: valor PROVISÓRIO. Trocar pelo θ do CONOPS (2.7.1).
};
}  // namespace bms