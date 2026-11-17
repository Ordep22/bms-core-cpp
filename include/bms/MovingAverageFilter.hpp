//
// Created by Pedro Pereira on 06/11/26.
//

#ifndef BMS_CORE_CPP_MOVINGAVERAGEFILTER_H
#define BMS_CORE_CPP_MOVINGAVERAGEFILTER_H

// include/bms/MovingAverageFilter.hpp
#pragma once
// TODO: includes necessários (array, cstddef, o seu Types.hpp)

// TODO: namespace? Confira como o resto do repositório faz.
template <std::size_t N>
class MovingAverageFilter {
    // TODO: static_assert para impedir N == 0
public:
    // TODO: push(...)   assinatura do diagrama
    // TODO: value()     assinatura do diagrama (pode alterar o objeto?)
private:
    // TODO: o buffer (tamanho fixo N)
    // TODO: onde escrever a próxima amostra
    // TODO: quantas amostras já chegaram (sua ideia do contador!)
    //       Pesquise: inicializador de membro, "= 0" direto na declaração
};

#endif //BMS_CORE_CPP_MOVINGAVERAGEFILTER_H
