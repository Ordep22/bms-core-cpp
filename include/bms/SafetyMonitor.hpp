#pragma once

#include "bms/Types.hpp"

namespace bms {

    // Supervisor de segurança (Req. 2). Classe CONCRETA: sem funções virtuais.
    // Classe comum: declarações aqui, corpos em src/SafetyMonitor.cpp.
    class SafetyMonitor {
    public:

        SafetyEvent evaluate(const BatteryTelemetry& raw);

        // Substitui os limites. Valem a partir da próxima amostra.
        void setThresholds(const SafetyThresholds& t);

    private:

        //Limites atuais
        SafetyThresholds threshold_{};

        //Amostra anterior (temperatura e instante), para o dT/dt
        BatteryTelemetry previous_{};

        //Indicador "já recebi uma amostra anterior?
        bool hasPrevious_ {false};

    };

}  // namespace bms