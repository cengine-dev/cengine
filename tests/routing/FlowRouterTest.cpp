#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <stdexcept>

#include <mock/FakeState.hpp>
#include <mock/MockRouter.hpp>

#include <cengine/routing/FlowRouter.hpp>

using namespace cengine::routing;
using testing::ReturnRef;

namespace {

// Máquina de fluxo mínima de um jogo fictício: um TFlow que deriva de IState
// e cuja transição agenda a próxima cena via fachada (mesmo desenho dos
// GameRouter do 8puzzle e do spaceinvaders — os consumidores da task 19).
class FakeFlow : public IState {
public:
    [[nodiscard]] std::string getCode() const override { return "fake_flow"; }
    [[nodiscard]] std::string getName() const override { return "fake_flow"; }

    void advance(const FlowRouter<FakeFlow>& router) const {
        router.setNextState(std::make_unique<FakeState>("next"));
    }
};

} // namespace

// A mecânica extraída: current() devolve o estado atual JÁ castado ao fluxo.
TEST(FlowRouterTest, CurrentReturnsStateCastToFlowType) {
    auto router = std::make_shared<MockRouter>();
    const FakeFlow flow;
    EXPECT_CALL(*router, currentState()).WillOnce(ReturnRef(flow));

    const FlowRouter<FakeFlow> flowRouter{router};

    EXPECT_EQ(&flowRouter.current(), &flow);
}

// Estado corrente de outro domínio: erro claro, não UB.
TEST(FlowRouterTest, CurrentThrowsWhenStateIsNotFlowType) {
    auto router = std::make_shared<MockRouter>();
    const FakeState alien{"outro_dominio"};
    EXPECT_CALL(*router, currentState()).WillOnce(ReturnRef(alien));

    const FlowRouter<FakeFlow> flowRouter{router};

    EXPECT_THROW(static_cast<void>(flowRouter.current()), std::runtime_error);
}

// setNextState delega para o agendamento em duas fases do IRouter.
TEST(FlowRouterTest, SetNextStateDelegatesToRequestState) {
    auto router = std::make_shared<MockRouter>();
    EXPECT_CALL(*router, requestState(testing::_)).Times(1);

    const FlowRouter<FakeFlow> flowRouter{router};

    flowRouter.setNextState(std::make_unique<FakeState>("next"));
}

// Sem nullptr na montagem (mesma linha da task 21).
TEST(FlowRouterTest, RejectsNullRouter) {
    EXPECT_THROW(FlowRouter<FakeFlow>{nullptr}, std::invalid_argument);
}

// Uso real: o jogo herda a mecânica e escreve só o vocabulário; a transição
// despacha sobre o estado atual e agenda a próxima cena.
TEST(FlowRouterTest, GameFacadeDispatchesTransitionOverCurrentState) {
    auto router = std::make_shared<MockRouter>();
    const FakeFlow flow;
    EXPECT_CALL(*router, currentState()).WillOnce(ReturnRef(flow));
    EXPECT_CALL(*router, requestState(testing::_)).Times(1);

    class GameRouter final : public FlowRouter<FakeFlow> {
    public:
        using FlowRouter::FlowRouter;
        void advance() const { current().advance(*this); }
    };

    const GameRouter gameRouter{router};
    gameRouter.advance();
}
