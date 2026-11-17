// tests/test_moving_average_filter.cpp
// =====================================================================
// MovingAverageFilter<N>   (Req. 1: Ingestão e Filtragem)
// ---------------------------------------------------------------------
// O QUE É
//   Filtro digital de média móvel. Recebe amostras brutas de tensão,
//   corrente e temperatura e devolve a média das últimas N, para
//   estabilizar as leituras exibidas na CLI e usadas pelas estimativas.
//
// POR QUE É ASSIM
//   - N é parâmetro de template: a janela é fixa em tempo de compilação.
//   - Guarda as amostras num buffer circular estático (sem alocação).
//   - Custo constante por amostra (Req. 8).
//   - NÃO alimenta o SafetyMonitor, que opera sobre a amostra bruta.
//
// CONTRATO TESTADO
//   - Filtro vazio:     Devolve zeros
//   - Janela parcial:   média só das amostras recebidas
//   - Janela cheia:     média das N mais recentes
//   - Excedeu N:        a mais antiga é descartada
//   - Canais:           tensão, corrente e temperatura são independentes
//   - Demais campos:    TODO decidir o que value() devolve em timestamp_ms,
//                       soc, soh e rul (o filtro só calcula média de V, I, T)
// =====================================================================

#include <gtest/gtest.h>
#include <vector>
#include "bms/MovingAverageFilter.hpp"
#include "bms/Types.hpp"

// O diagrama chama a amostra de "Sample"; no código ela é BatteryTelemetry.
using bms::BatteryTelemetry;
using bms::MovingAverageFilter;

namespace {

// Tolerância para comparar números em ponto flutuante.
constexpr float kTol = 0.001f;

// Auxiliar: monta uma amostra com os três canais.
// Os demais campos (timestamp_ms, soc, soh, rul) ficam no valor padrão.
BatteryTelemetry makeSample(float tensao, float corrente, float temperatura) {
    BatteryTelemetry sample{};
    sample.voltage_v = tensao;
    sample.current_a = corrente;
    sample.temperature_c = temperatura;
    return sample;
}

}  // namespace

// ---------------------------------------------------------------------
// Teste 1. Filtro recém-criado
// Comportamento: antes de qualquer push(), value() devolve ___ (decida).
// TODO: devolve zeros. Atenção à divisão 0/0.
// ---------------------------------------------------------------------
TEST(MovingAverageFilter, FiltroRecemCriado) {
    // Arrange
    MovingAverageFilter<4> filter;

    // Act
    const BatteryTelemetry result = filter.value();

    // Assert
    EXPECT_NEAR(result.voltage_v, 0.0f, kTol);
    EXPECT_NEAR(result.current_a, 0.0f, kTol);
    EXPECT_NEAR(result.temperature_c, 0.0f, kTol);
}

// ---------------------------------------------------------------------
// Teste 2. Uma única amostra
// Comportamento: depois de 1 push() numa janela de 4, value() devolve
// exatamente essa amostra. Também protege contra o erro de dividir por
// N (4) em vez de pela quantidade recebida (1).
// ---------------------------------------------------------------------
TEST(MovingAverageFilter, UmaAmostraRetornaEssaAmostra) {
    // Arrange
    MovingAverageFilter<4> filter;

    // Act
    filter.push(makeSample(220.0f, 1.5f, 42.0f));
    const BatteryTelemetry result = filter.value();

    // Assert
    EXPECT_NEAR(result.voltage_v, 220.0f, kTol);
    EXPECT_NEAR(result.current_a, 1.5f, kTol);
    EXPECT_NEAR(result.temperature_c, 42.0f, kTol);
}

// Teste 3. Janela parcial (N=4, 2 amostras) -> média das 2
//   DICA: escolha valores onde a média seja fácil de calcular de cabeça.
TEST(MovingAverageFilter, JanelaParcialMediaDasAmostras) {
    // Arrange
    MovingAverageFilter<4> filter;

    // Act
    filter.push(makeSample(10.0f, 5.0f, 30.0f));
    filter.push(makeSample(50.0f, 25.0f, 10.0f));
    const BatteryTelemetry result = filter.value();

    // Assert
    EXPECT_NEAR(result.voltage_v, 30.0f, kTol);
    EXPECT_NEAR(result.current_a, 15.0f, kTol);
    EXPECT_NEAR(result.temperature_c, 20.0f, kTol);
}

// Teste 4. Janela cheia (N amostras) -> média exata das N
TEST(MovingAverageFilter, JanelaCheiaMediaExataDasAmostras) {
    // Arrange
    MovingAverageFilter<4> filter;

    // Act
    filter.push(makeSample(100.0f, 12.0f, 28.0f));
    filter.push(makeSample(140.0f, 16.0f, 34.0f));
    filter.push(makeSample(180.0f, 20.0f, 40.0f));
    filter.push(makeSample(127.0f, 10.46f, 69.0f));
    const BatteryTelemetry result = filter.value();


    EXPECT_NEAR(result.voltage_v, 136.75f, kTol);
    EXPECT_NEAR(result.current_a, 14.615f, kTol);
    EXPECT_NEAR(result.temperature_c, 42.75f, kTol);
}

// Teste 5. N+1 amostras -> a mais antiga sai
//   DICA: escolha valores em que um erro de índice mude o resultado.
TEST(MovingAverageFilter, TestandoBufferCircularExcluiMaisAntigo) {
    // Arrange
    MovingAverageFilter<4> filter;

    // Act
    filter.push(makeSample(60.0f, 8.0f, 22.0f));
    filter.push(makeSample(100.0f, 12.0f, 28.0f));
    filter.push(makeSample(140.0f, 16.0f, 34.0f));
    filter.push(makeSample(180.0f, 20.0f, 40.0f));
    filter.push(makeSample(127.0f, 10.46f, 69.0f));
    const BatteryTelemetry result = filter.value();


    EXPECT_NEAR(result.voltage_v, 136.75f, kTol);
    EXPECT_NEAR(result.current_a, 14.615f, kTol);
    EXPECT_NEAR(result.temperature_c, 42.75f, kTol);
}


// Teste 6. 3*N amostras (várias voltas no buffer circular)
//   DICA: pesquise "for" dentro de um TEST. É o teste que mais pega bug.
TEST(MovingAverageFilter, TestandoBufferCircularTresVezes) {
    //Local Varibles
    struct SampleData{
        float voltage {0.0f};
        float current {0.0f};
        float temperature {0.0f};
    };


    // Matriz (vetor de vetores) contendo vários valores de teste divididos em 3 execuções/lotes
    std::vector<std::vector<SampleData>> testBatches = {
        // Lote 1
        {
            {10.0f, 2.0f, 20.0f},
            {20.0f, 3.0f, 21.0f},
            {30.0f, 4.0f, 22.0f},
            {40.0f, 5.0f, 23.0f}
        },
        // Lote 2
        {
            {50.0f, 6.0f, 24.0f},
            {60.0f, 7.0f, 25.0f},
            {70.0f, 8.0f, 26.0f},
            {80.0f, 9.0f, 27.0f}
        },
        // Lote 3 (Ajustado para que a soma dos 4 valores de voltage seja 547.0f -> Média = 136.75f)
        {
            {110.0f, 10.0f, 30.0f},
            {125.0f, 12.0f, 32.0f},
            {155.0f, 14.0f, 34.0f},
            {157.0f, 16.0f, 36.0f}
        }
    };

    // Arrange
    MovingAverageFilter<4> filter;

    // Act
    for (int run = 0; run < 3; ++run) {
        for (const auto& sample : testBatches[run]) {
            filter.push(makeSample(sample.voltage, sample.current, sample.temperature));
        }

        if (run == 2) {


            const BatteryTelemetry result = filter.value();

            EXPECT_NEAR(result.voltage_v, 136.75f, kTol);     // (110 + 125 + 155 + 157) / 4 = 136.75f
            EXPECT_NEAR(result.current_a, 13.00f, kTol);     // (10 + 12 + 14 + 16) / 4 = 13.00f
            EXPECT_NEAR(result.temperature_c, 33.00f, kTol); // (30 + 32 + 34 + 36) / 4 = 33.00f
        }
    }
}


// Teste 7. Tensão, corrente e temperatura independentes
//  DICA: use valores bem diferentes nos três canais.
TEST(MovingAverageFilter, TensaoCorrenteTempIndependentes) {

    // Arrange
    MovingAverageFilter<4> filter;

    // Act
    filter.push(makeSample(60.0f, 8.0f, 22.0f));
    filter.push(makeSample(100.0f, 12.0f, 28.0f));
    filter.push(makeSample(140.0f, 16.0f, 34.0f));
    filter.push(makeSample(180.0f, 20.0f, 40.0f));
    const BatteryTelemetry result = filter.value();


    EXPECT_NEAR(result.voltage_v, 120.00f, kTol);
    EXPECT_NEAR(result.current_a, 14.0f, kTol);
    EXPECT_NEAR(result.temperature_c, 31.0f, kTol);
}


// Teste 8. N = 1 -> sempre a última amostra
TEST(MovingAverageFilter, JanelaUnitariaDevolveUltimaAmostra) {

    // Arrange
    MovingAverageFilter<1> filter;

    // Act
    filter.push(makeSample(180.0f, 20.0f, 40.0f));
    filter.push(makeSample(60.0f, 8.0f, 22.0f));

    const BatteryTelemetry result = filter.value();


    EXPECT_NEAR(result.voltage_v, 60.0f, kTol);
    EXPECT_NEAR(result.current_a, 8.0f, kTol);
    EXPECT_NEAR(result.temperature_c, 22.0f, kTol);
}