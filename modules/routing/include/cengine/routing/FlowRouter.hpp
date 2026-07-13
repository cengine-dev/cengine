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
    void setNextState(std::unique_ptr<IState> state) const {
        m_router->requestState(std::move(state));
    }
};

} // namespace cengine::routing
