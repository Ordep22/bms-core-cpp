#pragma once

// =====================================================================
// MovingAverageFilter<N>   (Req. 1: Ingestão e Filtragem)
// ---------------------------------------------------------------------
// Filtro de média móvel de janela fixa. A cada push() guarda a amostra
// bruta; value() devolve a média de tensão, corrente e temperatura das
// últimas N amostras recebidas (ou de todas, se ainda forem menos de N).
//
// Restrições de projeto (Req. 7 e 8):
//   - N é fixo em tempo de compilação (parâmetro de template)
//   - buffer circular estático, sem alocação dinâmica
//   - custo constante por amostra (o laço de value() tem no máximo N voltas)
//
// ---------------------------------------------------------------------
// NOTAS DE APRENDIZADO: dificuldades e como foram resolvidas
// ---------------------------------------------------------------------
// [1] Template no .hpp, sem .cpp.
//     Dúvida: ".hpp só guarda declarações". Vale para classes comuns.
//     Um template é um molde: o compilador só fabrica a versão concreta
//     (por exemplo, N = 4) onde ela é usada, e para isso precisa ver o
//     corpo das funções. Por isso o corpo fica aqui no header.
//
// [2] std::vector não serve.
//     É dinâmico (Req. 7/8) e std::vector<N> é inválido: N é um número, não
//     um tipo. Solução: std::array<tipo, N>, de tamanho fixo.
//
// [3] Variável global N.
//     Um "unsigned int N;" fora do template criava outra variável. O N do
//     template já é uma constante de compilação e basta ele.
//
// [4] Índice circular.
//     "(index + 1) % N" já volta a 0 sozinho. Um "if (index > 3)" fixo
//     funcionaria só com N = 4.
//
// [5] Contador que satura.
//     O contador de amostras sobe até N e para (não volta a 0). Se zerasse,
//     o filtro "esqueceria" que a janela está cheia.
//
// [6] Função const e membros de cálculo.
//     value() só lê, por isso é const, e uma função const não altera
//     membros. As somas viraram variáveis LOCAIS. Se fossem membros, o
//     resultado de uma chamada se acumularia na seguinte.
//
// [7] "N == 0" versus "contador == 0".
//     N nunca é 0 (static_assert garante). O caso "ainda não chegou
//     amostra" é um teste sobre o contador. Sem ele, a média seria 0/0.
//
// [8] Somar só as posições 0 .. contador-1.
//     Antes de encher, só essas posições têm amostra. Depois de encher,
//     são todas as N. A mesma conta serve nos dois casos, porque a ordem
//     da soma não muda a média.
//
// [9] Tipos do índice e do laço: std::size_t, o mesmo de N. Misturar int
//     com size_t gera aviso de sinal (-Wsign-compare).
//
// [10] Membros privados terminam com "_" para se distinguir de variáveis
//      locais (por exemplo, a "voltage" da soma e o "buffer_" do filtro).
// =====================================================================

#include <array>
#include <cstddef>

#include "bms/Types.hpp"

namespace bms {

template <std::size_t N>
class MovingAverageFilter {
    static_assert(N > 0, "N must be greater than 0");

public:
    // Guarda a amostra bruta na próxima posição do buffer circular.
    void push(const BatteryTelemetry& raw) {
        buffer_[index_] = raw;
        index_ = (index_ + 1) % N;   // [4] volta a 0 ao chegar em N
        if (counter_ < N) {          // [5] satura em N
            ++counter_;
        }
    }

    // Média dos três canais sobre as amostras recebidas (no máximo N).
    // Sem amostras, devolve uma telemetria zerada. [7]
    // TODO: decidir o que vai em timestamp_ms, soc, soh e rul.
    BatteryTelemetry value() const {
        if (counter_ == 0) {         // [7]
            return BatteryTelemetry{};
        }

        float current = 0.0f;        // [6] locais, não membros
        float voltage = 0.0f;
        float temperature = 0.0f;

        for (std::size_t i = 0; i < counter_; ++i) {   // [8][9]
            current += buffer_[i].current_a;
            voltage += buffer_[i].voltage_v;
            temperature += buffer_[i].temperature_c;
        }

        const auto n = static_cast<float>(counter_);
        BatteryTelemetry result{};
        result.current_a = current / n;
        result.voltage_v = voltage / n;
        result.temperature_c = temperature / n;
        return result;
    }

private:
    std::array<BatteryTelemetry, N> buffer_{};   // [2] tamanho fixo
    std::size_t index_ = 0;                      // onde a próxima amostra entra
    std::size_t counter_ = 0;                    // quantas amostras já chegaram
};

}  // namespace bms