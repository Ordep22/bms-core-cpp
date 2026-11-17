# Registro de decisões de projeto: SafetyMonitor e CoreController

Projeto: núcleo OO em C++17 para gestão e diagnóstico de baterias (BMS).
Data: 16/11/2026. Situação: decisões aceitas pelo autor, válidas até o congelamento do código (04/12).

Formato de cada decisão: contexto, decisão, motivo, consequência para o código e para os testes.

---

## D-A. Quem aplica a política de segurança

**Contexto.** O `SafetyMonitor` vê as amostras e detecta violações. Alguém precisa decidir o que fazer com elas.

**Decisão.** O monitor só **classifica**: devolve um `SafetyEvent`. Quem decide a transição de estado é o estado ativo, em `handleEvent` (por exemplo `NominalState`).

**Motivo.** Separa detecção de política. O monitor fica testável sozinho, sem conhecer o `CoreController`. As transições acontecem apenas por eventos enumerados, como pede o Req. 3.

**Consequência.**
- `SafetyMonitor` não inclui nem referencia `CoreController` nem `IState`.
- Testes do monitor verificam o evento devolvido. Testes dos estados verificam a transição.

---

## D-B. Violação de V/I/T leva direto ao Failsafe

**Contexto.** Havia dúvida se uma violação deveria ir direto ao `EmergencyFailsafe` ou passar por um estado intermediário (por exemplo `Degraded`) ou por um tempo de tolerância.

**Decisão.** Qualquer evento diferente de `None` em `Nominal` leva ao `EmergencyFailsafe`. Não há estado intermediário nesta etapa.

**Motivo.** Menos estados, menos transições, menos testes. O escopo é um núcleo de diagnóstico, e o prazo de código termina em 04/12.

**Consequência.** `Degraded` e tolerância temporal ficam registrados como **trabalho futuro** no relatório.

---

## D-C. Alvo do rearme é o Boot

**Contexto.** Depois do Failsafe, para onde o rearme leva: `Boot` ou `Nominal`?

**Decisão.** Rearme leva a **`Boot`**. O autoteste roda de novo e só então o sistema chega a `Nominal`.

**Motivo.** É o caminho mais seguro: o sistema só volta a operar depois de se revalidar. Reaproveita a lógica de Boot que já existe e não exige código novo.

**Consequência.**
- `requestRearm()` só tem efeito em `EmergencyFailsafe` e com o **modo de caracterização ligado**. Caso contrário devolve recusa.
- Teste obrigatório: rearme recusado com caracterização desligada; rearme aceito com ela ligada, indo para `Boot`; rearme recusado fora do Failsafe.

### O que é rearme (explicação)

Quando o sistema entra em `EmergencyFailsafe`, ele fica **retentivo** (*latched*): permanece em estado seguro **mesmo que a causa desapareça**. Exemplo: a tensão passou do limite, o sistema foi ao Failsafe, e dois segundos depois a tensão voltou ao normal. Mesmo assim ele não volta sozinho.

Isso é intencional. Se voltasse sozinho, uma grandeza oscilando no limite faria o sistema entrar e sair do Failsafe repetidamente. Além disso, uma violação de segurança deve ser reconhecida por alguém antes de a operação continuar.

**Rearme** é o comando explícito que libera essa retenção. A analogia é o disjuntor: ele desarma quando há sobrecarga e só volta a conduzir depois que alguém o rearma.

Diferença para o que pode ser confundido com rearme:
- **Reiniciar o programa**: recria tudo do zero. Rearme acontece com o programa em execução, só troca o estado.
- **Retorno automático**: não existe neste projeto. Nenhum evento, inclusive `None`, tira o sistema do Failsafe.

Neste projeto o rearme só é aceito no **modo de caracterização** (bancada de experimentos, em que o autor injeta anomalias de propósito e precisa repetir cenários). Em operação normal, o rearme é recusado.

Sequência de exemplo:

```
Nominal --(OverVoltage)--> EmergencyFailsafe
EmergencyFailsafe: recebe None, OverCurrent... -> ignora (retentivo)
requestRearm() com caracterização desligada -> recusado
setCharacterizationMode(true); requestRearm() -> Boot
Boot --(None, autoteste ok)--> Nominal
```

---

## D-D. dT/dt e a memória da amostra anterior

**Contexto.** Detectar *thermal runaway* exige a taxa de variação da temperatura, então o monitor precisa lembrar a amostra anterior.

**Decisão.**
- Unidade de dT/dt: **°C/s**.
- O monitor guarda `previous_` e `hasPrevious_`.
- Na **primeira amostra** (`hasPrevious_ == false`) o dT/dt não é avaliado.
- O Δt vem do **timestamp da amostra**, não de um período fixo. Se Δt ≤ 0, dT/dt não é avaliado nessa amostra.

**Motivo.** Com período fixo, qualquer atraso de leitura vira falso `ThermalRunaway`. Com timestamp, a taxa é calculada sobre o intervalo real.

**Consequência.**
- `evaluate()` **não pode ser `const`** (ver abaixo).
- **A verificar:** `BatteryTelemetry` precisa ter campo de tempo. Se não tiver, adicionar antes de escrever os testes.

### Por que `evaluate()` não é `const`

`const` é a promessa de que o método **não altera o estado interno do objeto**. O `evaluate()` precisa atualizar `previous_` e `hasPrevious_` a cada chamada, para que a próxima amostra tenha com o que ser comparada. Então a promessa seria falsa.

Observação de precisão: o motivo não é o fato de o monitor tratar de segurança. É o fato de ele ter **memória** (a amostra anterior). Um método de verificação puro, sem memória, poderia e deveria ser `const`. Evitar `mutable` aqui: ele esconderia justamente a mutação que importa.

---

## D-E. Prioridade quando há várias violações na mesma amostra

**Decisão.** `evaluate()` devolve **um único** evento, escolhido por ordem fixa de severidade:

1. `ThermalRunaway`
2. `OverCurrent`
3. `OverVoltage`
4. `UnderVoltage`

`None` só quando nenhuma violação ocorre.

**Consequência.** O teste 3 verifica a prioridade com amostras que violam mais de um limite ao mesmo tempo.

---

## D-F. Sem debounce nem histerese nesta etapa

**Decisão.** Uma amostra fora do limite gera o evento. Limites entram por uma struct injetada no construtor; o core não lê arquivo (Req. 7).

**Consequência.** Se a dissertação precisar de filtragem, o `MovingAverageFilter` já existe e pode ser composto antes do monitor numa etapa posterior.

---

## Pendências

- [ ] Conferir o campo de tempo em `BatteryTelemetry` (D-D).
- [ ] Atualizar diagramas: enum `SafetyEvent`, estado interno do `SafetyMonitor`, nome `BatteryTelemetry`, seta `CoreController → IDataSource`.
- [ ] Revisar o `SafetyMonitor.hpp` rascunhado à luz destas decisões.
