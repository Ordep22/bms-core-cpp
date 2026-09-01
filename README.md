# Núcleo Orientado a Objetos para Gestão e Diagnóstico de Baterias (BMS Core C++)

Núcleo modular em C++17 para monitoramento, supervisão de segurança e estimação preditiva de indicadores de degradação (SoH/RUL) para baterias de íon-lítio. Projeto desenvolvido no âmbito da disciplina CABS002 (Programação Avançada) do PPGCA/UTFPR[cite: 3].

## 📌 Funcionalidades Principais

- **Máquina de Estados Operacional (FSM):** Implementação via padrão *State* com transições determinísticas entre `Boot/POST`, `Nominal` e `Contingência/Failsafe`[cite: 2].
- **Supervisão de Limites de Segurança:** Monitoramento em tempo de execução de sobretensão, sobrecorrente e estresse térmico[cite: 2].
- **Motor de Inferência Polimórfico:** Interface extensível (`IEstimator`) para acoplamento de modelos preditivos de degradação[cite: 2].
- **Injeção de Séries Temporais:** Leitura de bases de referência (NASA/CALCE) com aplicação de filtro de média móvel para mitigação de ruídos[cite: 2].
- **Auditoria e Logs:** Gravação estruturada de falhas e transições de estado para rastreabilidade[cite: 2].

---

## 🛠️ Tecnologias Utilizadas

- **Linguagem:** C++17
- **Compilador / Build:** GCC / Clang, CMake (>= 3.16)
- **IDEs Homologadas:** VS Code, Code::Blocks[cite: 3]

---

## 🚀 Como Compilar e Executar

### Pré-requisitos
Certifique-se de ter instalado o compilador C++ (GCC ou Clang) e o CMake:
```bash
sudo apt update
sudo apt install build-essential cmake