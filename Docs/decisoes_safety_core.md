# Registro de decisões de projeto: SafetyMonitor e CoreController
 
Projeto: núcleo OO em C++17 para gestão e diagnóstico de baterias (BMS).
Atualizado em: 18/11/2026. Válido até o congelamento do código (04/12).
 
Formato: contexto, decisão, motivo, consequência para o código e para os testes.
 
**Numeração.** As decisões **D1 a D4** são as do cabeçalho de `test_safety_monitor.cpp` e dizem respeito à detecção. As decisões **P1 a P4** dizem respeito à política e à máquina de estados (`CoreController`).
 
| ID | Assunto | Situação |
|---|---|---|
| D1 | Sinal da corrente | Decidida (aceita em 17/11) |
| D2 | Valor exatamente no limite | Decidida (aceita em 17/11) |
| D3 | Só um dos limiares térmicos | Decidida; no radar (ver L1) |
| D4 | Prioridade entre violações | Decidida (ver L2) |
| P1 | Quem aplica a política | Decidida |
| P2 | V/I/T → Failsafe direto | Decidida |
| P3 | Alvo do rearme: Boot | Decidida |
| P4 | dT/dt e memória da amostra anterior | Decidida |
| P5 | Sem debounce nem histerese | Decidida |
| P6 | O monitor não faz watchdog; o tempo só serve ao dT/dt | Decidida (ver L3) |
 
---
 
# Parte 1. Detecção (SafetyMonitor)
 
## D1. Sinal da corrente
 
**Contexto.** Em bases como a da NASA, a corrente de descarga pode aparecer com sinal negativo. O limite `max_current_a` é positivo.
 
**Decisão.** Comparar o **módulo**: violação quando `|I| > max_current_a`. Vale para carga e descarga.
 
**Motivo.** Uma sobrecorrente é perigosa nos dois sentidos, e o teste não depende da convenção de sinal do dataset.
 
**Consequência.** Limites assimétricos (um para carga, outro para descarga) ficam como trabalho futuro. Confirmar no CSV a convenção de sinal antes do teste 5.
 
## D2. Valor exatamente no limite
 
**Decisão.** Há violação **somente quando o valor passa estritamente do limite**: `V > Vmax`, `V < Vmin`, `|I| > Imax`. Valor igual ao limite é permitido.
 
**Motivo.** O limite é o máximo permitido, então atingi-lo ainda é operar dentro dele.
 
**Consequência.** O teste 4 usa o valor exato do limite e espera `None`. Cuidado com `float`: usar valores representáveis (por exemplo 4.0f, 30.0f) ou comparar com margem, para o teste não depender de arredondamento.
 
## D3. Só um dos limiares térmicos
 
**Contexto.** O CONOPS (seção 2.7.1) define o detector com limiares **acoplados**: `T > Tmax` **e** `dT/dt > θ`.
 
**Decisão.** Se só um dos dois ocorrer, o resultado é **`None`**.
 
**Motivo.** Segue o CONOPS. Não cria um evento fora do conjunto enumerado de `SafetyEvent` (Req. 3).
 
**Consequência.** Lacuna conhecida: um superaquecimento lento (T alta e dT/dt baixa) não é detectado. Registrada como **L1** (seção de limitações).
 
## D4. Prioridade com várias violações na mesma amostra
 
**Decisão.** `evaluate()` devolve **um único** evento, por ordem fixa de severidade:
 
1. `ThermalRunaway`
2. `OverCurrent`
3. `OverVoltage`
4. `UnderVoltage`
`None` apenas se não houver violação. O teste 11 verifica a ordem.
 
**Consequência.** Com um único evento por amostra, as violações simultâneas não prioritárias não aparecem no registro. Registrada como **L2**.
 
## Limites e estrutura
 
`SafetyThresholds` precisa de um campo por condição usada no `evaluate()`:
 
| Condição | Campo | Unidade |
|---|---|---|
| Sobretensão | `max_voltage_v` | V |
| Subtensão | `min_voltage_v` | V |
| Sobrecorrente | `max_current_a` | A |
| Temperatura máxima (Tmax) | `max_temperature_c` | °C |
| Taxa crítica (θ) | `max_dtemp_c_per_s` | °C/s |
 
Valor de θ: definir a partir do CONOPS, seção 2.7.1. Os limites entram por esta struct; o core não lê arquivo (Req. 7).
 
---
 
# Parte 2. Política e máquina de estados (CoreController)
 
## P1. Quem aplica a política de segurança
 
**Decisão.** O monitor só **classifica**: devolve um `SafetyEvent`. Quem decide a transição é o estado ativo, em `handleEvent` (por exemplo `NominalState`).
 
**Motivo.** Separa detecção de política. O monitor fica testável sozinho, sem conhecer o `CoreController`. Transições apenas por eventos enumerados (Req. 3).
 
**Consequência.**
- `SafetyMonitor` não inclui nem referencia `CoreController` nem `IState`.
- Testes do monitor verificam o evento devolvido. Testes dos estados verificam a transição.
## P2. Violação leva direto ao Failsafe
 
**Decisão.** Em `Nominal`, qualquer evento diferente de `None` leva ao `EmergencyFailsafe`. Não há estado intermediário nesta etapa.
 
**Motivo.** Menos estados, menos transições, menos testes, dentro do prazo de código (04/12).
 
**Consequência.** `Degraded` e tolerância temporal ficam como trabalho futuro.
 
## P3. Alvo do rearme é o Boot
 
**Decisão.** O rearme leva a **`Boot`**. O autoteste roda de novo e só então o sistema chega a `Nominal`.
 
**Motivo.** É o caminho mais seguro e reaproveita a lógica de Boot existente.
 
**Consequência.**
- `requestRearm()` só tem efeito em `EmergencyFailsafe` e com o **modo de caracterização ligado**. Caso contrário, recusa.
- Testes: recusado com caracterização desligada; aceito com ela ligada, indo para `Boot`; recusado fora do Failsafe.
### O que é rearme
 
Quando o sistema entra em `EmergencyFailsafe`, ele fica **retentivo** (*latched*): permanece em estado seguro **mesmo que a causa desapareça**. Exemplo: a tensão passou do limite, o sistema foi ao Failsafe, e dois segundos depois a tensão voltou ao normal. Mesmo assim ele não volta sozinho.
 
Isso é intencional. Se voltasse sozinho, uma grandeza oscilando no limite faria o sistema entrar e sair do Failsafe repetidamente. E uma violação de segurança deve ser reconhecida antes de a operação continuar.
 
**Rearme** é o comando explícito que libera essa retenção. Analogia: o disjuntor desarma na sobrecarga e só volta a conduzir depois que alguém o rearma.
 
Diferença para o que pode ser confundido com rearme:
- **Reiniciar o programa**: recria tudo do zero. O rearme acontece com o programa em execução e só troca o estado.
- **Retorno automático**: não existe neste projeto. Nenhum evento, inclusive `None`, tira o sistema do Failsafe.
O rearme só é aceito no **modo de caracterização** (bancada de experimentos, com injeção de anomalias e cenários repetidos). Em operação normal, é recusado.
 
```
Nominal --(OverVoltage)--> EmergencyFailsafe
EmergencyFailsafe: recebe None, OverCurrent... -> ignora (retentivo)
requestRearm() com caracterização desligada -> recusado
setCharacterizationMode(true); requestRearm() -> Boot
Boot --(None, autoteste ok)--> Nominal
```
 
## P4. dT/dt e a memória da amostra anterior
 
**Decisão.**
- Unidade de dT/dt: **°C/s**.
- O monitor guarda `previous_` e `hasPrevious_`.
- Na **primeira amostra** (`hasPrevious_ == false`), o dT/dt não é avaliado.
- O Δt vem de `timestamp_ms` da amostra (campo já existente em `BatteryTelemetry`). Se o instante atual não for maior que o anterior, o dT/dt não é avaliado nessa amostra.
**Atenção (tipo sem sinal).** `timestamp_ms` é `uint32_t`. Se o instante atual for menor que o anterior, `atual - anterior` **não dá negativo: dá um número enorme**. Comparar os dois instantes **antes** de subtrair.
 
**Motivo.** Com período fixo, qualquer atraso de leitura vira falso `ThermalRunaway`. Com timestamp, a taxa é calculada sobre o intervalo real.
 
**Consequência.** `evaluate()` **não pode ser `const`**.
 
### Por que `evaluate()` não é `const`
 
`const` é a promessa de não alterar o estado interno. O `evaluate()` atualiza `previous_` e `hasPrevious_` a cada chamada, para que a amostra seguinte tenha com o que ser comparada. O motivo não é o monitor tratar de segurança: é ele ter **memória**. Um método de verificação puro, sem memória, deveria ser `const`. Evitar `mutable`, que esconderia a mutação que importa.
 
## P5. Sem debounce nem histerese nesta etapa
 
**Decisão.** Uma amostra fora do limite gera o evento. Se a dissertação precisar de filtragem, o `MovingAverageFilter` pode ser composto antes do monitor numa etapa posterior.
 
## P6. O monitor não faz watchdog
 
**Contexto.** `BatteryTelemetry` tem `timestamp_ms`. Surgiu a dúvida se o monitor deveria também vigiar o tempo (watchdog) ou apenas usar a diferença entre amostras.
 
**Decisão.** O monitor usa o tempo **somente** para calcular dT/dt (Δt entre duas amostras). Detectar **ausência ou atraso de dados** é função do ciclo do controlador, que gera o evento `DeadlineViolation` (Req. 3 e 8). `DeadlineViolation` nunca é devolvido por `SafetyMonitor::evaluate()`.
 
**Motivo.** O `evaluate()` só executa quando uma amostra chega. Se o sensor parar, a função não é chamada e nada dentro dela percebe a ausência. Um watchdog exige uma referência de tempo **externa** ao fluxo de amostras (relógio ou temporizador).
 
**Consequência.**
- A detecção de `DeadlineViolation` é implementada no `CoreController::tick`, com um relógio injetável (por exemplo, `IClock`), testável com um relógio falso.
- `timestamp_ms` é `uint32_t` em milissegundos e dá a volta em cerca de 49,7 dias. Nesse caso o instante atual fica menor que o anterior, e o dT/dt não é avaliado naquela amostra. Aceitável no escopo da dissertação.
---
 
# Limitações conhecidas e trabalho futuro
 
| # | Limitação | Origem | Evolução futura |
|---|---|---|---|
| L1 | Superaquecimento **lento** (T alta com dT/dt baixa) não é detectado | D3 | Novo evento `OverTemperature` em `SafetyEvent` |
| L2 | Violações **simultâneas**: o registro mostra só a prioritária. A segurança não é afetada, pois qualquer evento leva ao Failsafe retentivo (P2, P3); perde-se informação de **diagnóstico** | D4 | O monitor expor também uma máscara `FaultCode` (enum de bits já existente) com todas as violações da amostra, mantendo um único `SafetyEvent` para a máquina de estados |
| L3 | Amostras **atrasadas** diluem o dT/dt: 20 °C em 10 s dá 2 °C/s, embora o aquecimento possa ter ocorrido nos últimos 2 s, o que pode mascarar um `ThermalRunaway` | P6 | Guarda de Δt máximo no monitor (não confiar no dT/dt acima de um limite) e watchdog no controlador |
| L4 | Sem debounce nem histerese: uma única amostra fora do limite aciona o evento | P5 | Compor o `MovingAverageFilter` antes do monitor, ou confirmar a violação em N amostras |
| L5 | Sem estado intermediário (`Degraded`) nem tempo de tolerância | P2 | Novo estado entre `Nominal` e `EmergencyFailsafe` |
 
Critério para entrar nesta lista: a limitação é conhecida, tem efeito explicável e tem uma evolução identificada. O relatório deve apresentá-las como escolhas de escopo, não como falhas.
 
---
 
## Pendências
 
- [ ] Definir o valor de θ (CONOPS 2.7.1) e restaurar `max_temperature_c` em `SafetyThresholds`.
- [ ] Remover o `class SafetyMonitor {};` global de `src/SafetyMonitor.cpp` ao implementar.
- [ ] Atualizar diagramas: enum `SafetyEvent`, estado interno do `SafetyMonitor`, nome `BatteryTelemetry`, seta `CoreController → IDataSource`.
- [ ] Reconciliar `IState` do repositório (`process(ctx, telemetry)`) com o do demo (`handleEvent(ctx, SafetyEvent)`) na promoção do demo (20 a 22/11).
 