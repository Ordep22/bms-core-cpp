#pragma once

#include "bms/Types.hpp"

namespace bms{

// Supervisor de segurança (Req. 2).
//
// Classe CONCRETA, sem funções virtuais no caminho de detecção (Req. 8).
// Só CLASSIFICA: devolve um SafetyEvent. Quem decide o que fazer com ele
// é o estado ativo (decisão P1). Por isso esta classe NÃO inclui
// CoreController nem IState.

class SafetyMonitor{

public:
    //   Monitor com limites padrões, construido sem argumentos.
    //   Usamos "explicit" para evitar a conversões automáticas durante a compilação
    SafetyMonitor() = default;
    explicit SafetyMonitor(const SafetyThresholds& thresholds);

    
    //   o método atualiza previous_ e hasPrevious_ a cada chamada.
    //   Devolve UM evento por amostra, pela prioridade da decisão D4.
    SafetyEvent evaluate(const BatteryTelemetry& sample);


    // Substitui os limites. Valem a partir da próxima amostra.
    // [3] Pergunta de projeto: trocar os limites deve apagar a amostra
    //     anterior (previous_)? Neste momento sim. Não vejo motivos para salvar as amotras em um buffer
    void setThresholds(const SafetyThresholds& thresholds);

private:
    // [4] Auxiliar SÓ de leitura: decide se há ThermalRunaway comparando a
    //     amostra atual com previous_. Repare que ele PODE ser const (não
    //     altera nada). Compare com o evaluate() e veja a diferença.
    bool isThermalRunaway(const BatteryTelemetry& sample) const;

    // Limites atuais
    SafetyThresholds threshold_{};

    // Amostra anterior (temperatura e instante), para o dT/dt.
    // Sem alocação: é um membro por valor (Req. 8).
    BatteryTelemetry previous_{};

    //Controla o recebimento da amostra anterior.
    bool hasPrevious_{false};

}



    
} // namespace bm



