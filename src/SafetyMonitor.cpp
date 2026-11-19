#include <cmath>
#include "bms/SafetyMonitor.hpp"

namespace bms

{
SafetyMonitor::SafetyMonitor(const SafetyThresholds& thresholds){

    threshold_ = thresholds;
}




// ---------------------------------------------------------------------
// [2] evaluate(): ordem de raciocínio sugerida
// ---------------------------------------------------------------------
SafetyEvent SafetyMonitor::evaluate (const BatteryTelemetry& sample)
{

    // PASSO 1. Calcule os resultados SEM retornar ainda.
    //   - thermal = isThermalRunaway(sample);
    //   - tensão e corrente: compare com threshold_ (D1: use o MÓDULO da
    //     corrente; 
    //   - D2: estritamente maior/menor, nunca >= ou <=).
    bool thermalEvent  = {false};
    bool OverVoltageEvent {false};
    bool UnderVoltageEvent {false};
    bool OverCurrentEvent {false};


    thermalEvent = SafetyMonitor::isThermalRunaway(sample);

    if (sample.voltage_v > threshold_.max_voltage_v)
    {
        OverVoltageEvent = true;
    }
    else if (sample.voltage_v < threshold_.min_voltage_v)
    {
        UnderVoltageEvent = true;
    }
    else
    {
        //do nothing
    }

    if (std::abs(sample.current_a)> threshold_.max_current_a)
    {
        OverCurrentEvent = true;
    }
    else
    {
        //do nothing
    }

    previous_ = sample;
    hasPrevious_ = true;

    if(thermalEvent)
    {
        return SafetyEvent::ThermalRunaway;
    }
    else if(OverCurrentEvent)
    {
        return SafetyEvent::OverCurrent;
    }
    else if(OverVoltageEvent) 
    { 
       return SafetyEvent::OverVoltage;
    }
    else if(UnderVoltageEvent)
    {
        return SafetyEvent::UnderVoltage;
    }
    else
    {
        return SafetyEvent::None;
    }

}

// ---------------------------------------------------------------------
// [3] setThresholds()
// ---------------------------------------------------------------------
void SafetyMonitor::setThresholds (const SafetyThresholds& thresholds) 
{
    threshold_ = thresholds;
}


// ---------------------------------------------------------------------
// [4] isThermalRunaway(): detector de dois limiares ACOPLADOS (D3)
//        T > Tmax   E   dT/dt > θ
// ---------------------------------------------------------------------
bool SafetyMonitor::isThermalRunaway(const BatteryTelemetry& sample) const
{
    uint32_t timeInterval_ms {0};
    float dtemp_c_per_s{0.0f};

    // PASSO 1. Sem amostra anterior não há dT/dt: devolva false.
    //          (decisão P4; teste 9)
    
    if(!hasPrevious_)
    {
        return false;
    }
    else
    {

    // PASSO 2. Cuidado com o tempo: timestamp_ms é uint32_t, SEM SINAL.
    //          Se sample.timestamp_ms < previous_.timestamp_ms, a conta
    //          "sample - previous" NÃO dá negativo: dá um número gigante.
    //          Então COMPARE os dois instantes ANTES de subtrair.
    //          Se o instante atual não for maior que o anterior, devolva
    //          false (sem dividir por zero; teste 10).
        if (sample.timestamp_ms > previous_.timestamp_ms)
        {
            timeInterval_ms = previous_.timestamp_ms - sample.timestamp_ms;
            dtemp_c_per_s = ((sample.temperature_c - previous_.temperature_c)/timeInterval_ms)*1000.0f;

            if ((sample.temperature_c > threshold_.max_dtemp_c_per_s) && (dtemp_c_per_s > threshold_.max_dtemp_c_per_s))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
}

} // namespace bms {}







