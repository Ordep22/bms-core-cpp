// =====================================================================
// state_demo.cpp: padrão State aplicado ao CoreController do BMS
// ---------------------------------------------------------------------
// Compilar (as mesmas flags de aviso do projeto; -fno-exceptions e
// -fno-rtti valem para o core, Req. 8):
//   g++ -std=c++17 -Wall -Wextra -Wpedantic -fno-exceptions -fno-rtti
//       state_demo.cpp -o state_demo   (tudo em uma linha)
//
// Papéis do padrão:
//   Context        -> CoreController
//   State          -> IState
//   ConcreteState  -> BootState, NominalState, FailsafeState
//
// Decisões de projeto (Req. 3 e Req. 8):
//   - Os estados são MEMBROS do contexto: sem new e sem delete.
//   - Transições só por eventos enumerados (SafetyEvent).
//   - Failsafe é retentivo: só sai por rearme explícito, e apenas em
//     modo de caracterização.
// =====================================================================
#include "state_demo.hpp"

namespace {
    enum class SystemState : std::uint8_t { Boot, Nominal, EmergencyFailsafe };
}

namespace {
    enum class SafetyEvent : std::uint8_t {
        None,
        OverVoltage,
        UnderVoltage,
        OverCurrent,
        ThermalRunaway,
        DeadlineViolation
    };
}

const char* toString(SystemState s) {
    switch (s) {
        case SystemState::Boot:              return "Boot";
        case SystemState::Nominal:           return "Nominal";
        case SystemState::EmergencyFailsafe: return "EmergencyFailsafe";
    }
    return "?";
}

const char* toString(SafetyEvent e) {
    switch (e) {
        case SafetyEvent::None:              return "None";
        case SafetyEvent::OverVoltage:       return "OverVoltage";
        case SafetyEvent::UnderVoltage:      return "UnderVoltage";
        case SafetyEvent::OverCurrent:       return "OverCurrent";
        case SafetyEvent::ThermalRunaway:    return "ThermalRunaway";
        case SafetyEvent::DeadlineViolation: return "DeadlineViolation";
    }
    return "?";
}

// Declaração antecipada: IState só precisa do NOME do contexto.
class CoreController;

// ---------------------------------------------------------------------
// State: a interface comum a todos os estados
// ---------------------------------------------------------------------
class IState {
public:
    virtual ~IState() = default;
    virtual void onEntry(CoreController& ctx) = 0;
    virtual void handleEvent(CoreController& ctx, SafetyEvent ev) = 0;
    virtual void onExit(CoreController& ctx) = 0;
    virtual SystemState stateId() const = 0;
};

// ---------------------------------------------------------------------
// ConcreteStates (só as declarações; os corpos vêm depois do contexto,
// porque precisam chamar ctx.transitionTo())
// ---------------------------------------------------------------------
class BootState final : public IState {
public:
    void onEntry(CoreController& ctx) override;
    void handleEvent(CoreController& ctx, SafetyEvent ev) override;
    void onExit(CoreController& ctx) override;
    SystemState stateId() const override { return SystemState::Boot; }
};

class NominalState final : public IState {
public:
    void onEntry(CoreController& ctx) override;
    void handleEvent(CoreController& ctx, SafetyEvent ev) override;
    void onExit(CoreController& ctx) override;
    SystemState stateId() const override { return SystemState::Nominal; }
};

class FailsafeState final : public IState {
public:
    void onEntry(CoreController& ctx) override;
    void handleEvent(CoreController& ctx, SafetyEvent ev) override;
    void onExit(CoreController& ctx) override;
    SystemState stateId() const override { return SystemState::EmergencyFailsafe; }
};

// ---------------------------------------------------------------------
// Context: guarda o estado atual e DELEGA o comportamento a ele
// ---------------------------------------------------------------------
class CoreController {
public:
    void start() { current_->onEntry(*this); }

    // Em vez de um switch com a lógica de cada estado, delega.
    void tick(SafetyEvent ev) { current_->handleEvent(*this, ev); }

    void transitionTo(SystemState next) {
        current_->onExit(*this);
        switch (next) {
            case SystemState::Boot:              current_ = &boot_;     break;
            case SystemState::Nominal:           current_ = &nominal_;  break;
            case SystemState::EmergencyFailsafe: current_ = &failsafe_; break;
        }
        current_->onEntry(*this);
    }

    void setCharacterizationMode(bool on) { characterization_ = on; }

    // Rearme: só vale no Failsafe e só em modo de caracterização.
    bool requestRearm() {
        if (state() != SystemState::EmergencyFailsafe) return false;
        if (!characterization_) {
            std::puts("  [Core] rearme recusado: caracterização desligada");
            return false;
        }
        // Decisão em aberto no projeto: voltar a Nominal ou passar pelo POST.
        // Aqui volta ao Boot (refaz o autoteste).
        transitionTo(SystemState::Boot);
        return true;
    }

    SystemState state() const { return current_->stateId(); }

private:
    // Estados como membros: alocação estática, sem new (Req. 8).
    BootState     boot_;
    NominalState  nominal_;
    FailsafeState failsafe_;
    IState*       current_ = &boot_;
    bool          characterization_ = false;
};

// ---------------------------------------------------------------------
// Comportamento de cada estado: está TODO dentro da sua própria classe
// ---------------------------------------------------------------------
void BootState::onEntry(CoreController&) {
    std::puts("  [Boot] autoteste (POST) em andamento");
}
void BootState::handleEvent(CoreController& ctx, SafetyEvent ev) {
    if (ev == SafetyEvent::None) {
        ctx.transitionTo(SystemState::Nominal);            // autoteste ok
    } else {
        ctx.transitionTo(SystemState::EmergencyFailsafe);  // falha no autoteste
    }
}
void BootState::onExit(CoreController&) {}

void NominalState::onEntry(CoreController&) {
    std::puts("  [Nominal] supervisão ativa");
}
void NominalState::handleEvent(CoreController& ctx, SafetyEvent ev) {
    if (ev != SafetyEvent::None) {
        ctx.transitionTo(SystemState::EmergencyFailsafe);
    }
}
void NominalState::onExit(CoreController&) {}

void FailsafeState::onEntry(CoreController&) {
    std::puts("  [Failsafe] saída de segurança ACIONADA");
}
void FailsafeState::handleEvent(CoreController&, SafetyEvent ev) {
    // Retentivo: nenhum evento tira o sistema daqui.
    if (ev != SafetyEvent::None) {
        std::printf("  [Failsafe] %s ignorado (retentivo)\n", toString(ev));
    }
}
void FailsafeState::onExit(CoreController&) {
    std::puts("  [Failsafe] saída de segurança liberada");
}

// ---------------------------------------------------------------------
// Cenário de demonstração
// ---------------------------------------------------------------------
static void step(CoreController& c, SafetyEvent ev) {
    std::printf("tick(%s)\n", toString(ev));
    c.tick(ev);
    std::printf("  -> estado: %s\n", toString(c.state()));
}

int main() {
    CoreController core;
    core.start();

    step(core, SafetyEvent::None);          // Boot -> Nominal
    step(core, SafetyEvent::None);          // permanece em Nominal
    step(core, SafetyEvent::OverVoltage);   // Nominal -> Failsafe
    step(core, SafetyEvent::OverCurrent);   // ignorado: retentivo

    std::puts("requestRearm() sem modo de caracterização");
    core.requestRearm();

    std::puts("setCharacterizationMode(true); requestRearm()");
    core.setCharacterizationMode(true);
    core.requestRearm();                    // Failsafe -> Boot
    std::printf("  -> estado: %s\n", toString(core.state()));

    step(core, SafetyEvent::None);          // Boot -> Nominal
    return 0;
}
