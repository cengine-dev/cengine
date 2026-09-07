#pragma once

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <cengine/routing/IRouter.hpp>
#include <cengine/routing/IState.hpp>

namespace cengine::routing {

/**
 * @brief Mecânica da fachada de navegação de domínio sobre o `IRouter`.
 *
 * Os jogos consumidores repetiam a mesma fachada ("GameRouter"): guardar o
 * router, fazer o `dynamic_cast` do estado atual para a máquina de fluxo do
 * jogo e delegar o agendamento da próxima cena. Esta classe extrai só essa
 * mecânica (ver .ai/task/19 e o filtro anti-depósito no ADR 0002); o
 * VOCABULÁRIO de navegação — `menu()`, `gameOver()`... — continua no jogo,
 * que herda deste helper e escreve apenas as transições:
 *
 * @code
 * class GameRouter final : public cengine::routing::FlowRouter<StateGameFlow> {
 * public:
 *     using FlowRouter::FlowRouter;
 *     void menu()     { current().menu(*this); }
 *     void gameOver() { current().gameOver(*this); }
 * };
 * @endcode
 *
 * ## Quem POSSUI quem, e por que isto importa
 *
 * Este helper guarda um `shared_ptr<IRouter>`, e o router possui o repositório,
 * que possui as cenas. **Uma cena que guarde este objeto por valor ou por
 * `shared_ptr` fecha um CICLO** — router -> repositório -> cena -> router — e
 * nada é destruído: um vazamento que nenhum teste de jogo veria, porque só
 * aparece no fim do processo.
 *
 * O contrato, portanto: **a cena recebe o `FlowRouter` por REFERÊNCIA**, nunca
 * por posse compartilhada. Ele vive no composition root, ao lado do router.
 *
 * @tparam TFlow o tipo-base da máquina de fluxo do jogo (deriva de `IState`);
 *         as transições despacham sobre o estado ATUAL do router.
 */
template <typename TFlow>
class FlowRouter {
    static_assert(std::is_base_of_v<IState, TFlow>,
                  "FlowRouter: TFlow must derive from cengine::routing::IState");

    std::shared_ptr<IRouter> m_router;

public:
    /// @throws std::invalid_argument se @p router for nulo.
    explicit FlowRouter(std::shared_ptr<IRouter> router) : m_router(std::move(router)) {
        if (!m_router) {
            throw std::invalid_argument("FlowRouter: router must not be null");
        }
    }

    /**
     * O estado atual do router já castado para a máquina de fluxo do jogo.
     *
     * Mesmo contrato de tempo de vida de `IRouter::currentState()`: a
     * referência vale até a próxima navegação — não a retenha.
     *
     * @throws std::runtime_error se o estado corrente não for um @p TFlow
     *         (fluxo montado com estado de outro domínio).
     */
    [[nodiscard]] const TFlow& current() const {
        const auto* flow = dynamic_cast<const TFlow*>(&m_router->currentState());
        if (!flow) {
            throw std::runtime_error("FlowRouter: current state is not of the flow type");
        }
        return *flow;
    }

    /// Agenda a próxima cena (chamado pelas transições da máquina de fluxo);
    /// delega para `IRouter::requestState()` — efetivação em duas fases.
    ///
    /// **O `const` é DELIBERADO, e não um descuido.** Ele parece mentir (o
    /// método navega, portanto muda o mundo), e a tentação de "limpar" é real —
    /// mas ele é a peça que sustenta o desenho dos estados de fluxo:
    ///
    /// ```cpp
    /// // asteroids/src/asteroids/game/state/StateGame.h
    /// void menu(const GameRouter& game) const override;
    /// ```
    ///
    /// Os estados são objetos SEM ESTADO, e recebem o router como
    /// `const GameRouter&`. O `const` ali diz *"você pode pedir uma navegação;
    /// não pode reconfigurar o roteador"* — e é essa a fronteira. Treze jogos do
    /// ecossistema escrevem assim (asteroids, breakout, bulwark, counter, cue,
    /// delve, fold, klondike, mario-bros, starforce, tactics, vigil, zelda);
    /// tirar o `const` daqui quebra todos.
    void setNextState(std::unique_ptr<IState> state) const {
        m_router->requestState(std::move(state));
    }
};

} // namespace cengine::routing
