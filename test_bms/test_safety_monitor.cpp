// tests/test_safety_monitor.cpp
// =====================================================================
// SafetyMonitor   (Req. 2: Monitoramento e Proteção)
// ---------------------------------------------------------------------
// O QUE É
//   Classe supervisora dedicada. A cada amostra bruta avalia sobretensão,
//   subtensão e sobrecorrente contra limites configuráveis e aplica o
//   detector térmico de dois limiares acoplados do CONOPS (Seção 2.7.1):
//       T > Tmax   E   dT/dt > θ crítico
//   Devolve UM SafetyEvent por amostra. Quem decide o que fazer com o
//   evento é a máquina de estados, não o monitor.
//
// POR QUE É ASSIM
//   - Opera sobre a amostra BRUTA, sem filtro (sem atraso de grupo).
//   - Classe concreta, sem funções virtuais no caminho da detecção (Req. 8).
//   - Sem exceções e sem alocação: o resultado é um tipo enumerado.
//   - Custo constante por amostra.
//   - Guarda a amostra anterior num membro (sem alocação) para o dT/dt.
//
// CONTRATO TESTADO
//   [D] = decisão a registrar aqui quando for tomada.
//   - Amostra dentro dos limites ........... None
//   - V > Vmax ............................. OverVoltage
//   - V < Vmin ............................. UnderVoltage
//   - corrente |I| acima do limite ......... OverCurrent        [D1: módulo]
//   - Valor exatamente no limite ........... None (violação só além do limite) [D2]
//   - T > Tmax E dT/dt > θ ................. ThermalRunaway
//   - Só um dos dois limiares térmicos ..... None  [D3: aceita; limitação conhecida, no radar]
//   - Primeira amostra (sem anterior) ...... dT/dt não é avaliado
//   - Intervalo de tempo igual a zero ...... sem divisão por zero
//   - Mais de uma violação na amostra ...... ThermalRunaway > OverCurrent > OverVoltage > UnderVoltage [D4]
//   - setThresholds() ...................... novos limites valem na amostra seguinte
// =====================================================================

#include <gtest/gtest.h>

#include "bms/SafetyMonitor.hpp"
#include "bms/Types.hpp"

using bms::BatteryTelemetry;
using bms::SafetyEvent;
using bms::SafetyMonitor;
using bms::SafetyThresholds;

namespace {

// Auxiliar: amostra com os três canais e o instante de medição.
// DICA: o timestamp é necessário para o dT/dt (diferença entre duas
//       amostras dividida pelo intervalo de tempo).
// TODO: BatteryTelemetry makeSample(float tensao, float corrente,
//                                   float temperatura, uint32_t t_ms)

// Valores "seguros" para partir deles (dentro dos limites padrão de
// SafetyThresholds: 2.8 a 4.2 V, 30 A, 60 °C).
// DICA: um teste que viola UM limite parte de uma amostra nominal e muda
//       só aquele campo. Assim o motivo da falha fica claro.
// TODO: constantes ou uma função auxiliar para a amostra nominal.

}  // namespace

// ---------------------------------------------------------------------
// Teste 1. Amostra nominal
// Comportamento: amostra dentro de todos os limites -> SafetyEvent::None
// ATENÇÃO: com o monitor "vazio" (sempre devolve None) este teste PASSA.
//          Um teste que passa antes de existir código não prova nada.
//          Ele só vale em par com os testes de violação.
// ---------------------------------------------------------------------
TEST(SafetyMonitor, AmostraNominalNaoGeraEvento) {
    GTEST_SKIP() << "TODO: escrever";   // apague esta linha ao escrever o teste
    // Arrange: TODO monitor com limites padrão
    // Act:     TODO evaluate() de uma amostra nominal
    // Assert:  TODO comparar com SafetyEvent::None (EXPECT_EQ)
}

// ---------------------------------------------------------------------
// Teste 2. Sobretensão
// Comportamento: V acima do limite -> SafetyEvent::OverVoltage
// DICA: use um valor claramente acima (não no limite; isso é o teste 4).
// ---------------------------------------------------------------------
TEST(SafetyMonitor, TensaoAcimaDoLimiteGeraSobretensao) {
    GTEST_SKIP() << "TODO: escrever";
}

// Teste 3. Subtensão: V abaixo do mínimo -> UnderVoltage
//
// Teste 4. Fronteira: V exatamente no limite [D2]
//   DICA: escreva a decisão no cabeçalho e o teste a confirma.
//
// Teste 5. Sobrecorrente [D1]
//   DICA: confira no CSV da NASA se a corrente de descarga aparece com
//         sinal negativo. Isso muda como o limite é comparado.
//
// Teste 6. Só T > Tmax, com variação lenta [D3]
// Teste 7. Só dT/dt > θ, com T abaixo de Tmax -> None (limiares ACOPLADOS)
// Teste 8. T > Tmax E dT/dt > θ -> ThermalRunaway
//   DICA: duas amostras. Escolha os instantes para o dT/dt ser fácil de
//         calcular de cabeça (por exemplo, 1000 ms entre elas).
//
// Teste 9. Primeira amostra com T muito alta -> sem ThermalRunaway
// Teste 10. Duas amostras com o mesmo timestamp -> não trava, sem divisão por zero
// Teste 11. Duas violações na mesma amostra [D4]
// Teste 12. setThresholds(): a mesma amostra muda de resultado