#include <iostream>
#include "bms/Types.hpp"

int main() {
    bms::BatteryTelemetry sample{100, 3.7f, 1.2f, 25.4f, 85.0f, 98.0f, 450.0f};

    std::cout << "[BMS Core] Inicializando modulo...\n";
    std::cout << "Telemetria Inicial -> Tensao: " << sample.voltage_v 
              << "V | Corrente: " << sample.current_a 
              << "A | Temperatura: " << sample.temperature_c << "C\n";

    return 0;
}