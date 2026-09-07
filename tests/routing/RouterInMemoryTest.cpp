#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <string>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/RouterInMemory.hpp>
#include <stdexcept>

#include <cengine/routing/SceneRepository.hpp>

#include <mock/FakeState.hpp>
#include <mock/MockScene.hpp>
#include <mock/MockSceneRepository.hpp>

using namespace cengine::core;
using namespace cengine::routing;

// O router é o dono da máquina de estados (task 13): os testes exercitam o
// par atual/próximo de verdade e usam o repositório mockado apenas para
// observar o provisionamento (getScene/unloadScene).
class RouterInMemoryTest : public ::testing::Test {
protected:
    MockSceneRepository* mockSceneRepository{nullptr}; // observado; posse é do router
    std::unique_ptr<RouterInMemory> routerService;

    MockScene mockScene;

    void SetUp() override {
        auto repository = std::make_unique<MockSceneRepository>();
        mockSceneRepository = repository.get();
        routerService = std::make_unique<RouterInMemory>(
            std::move(repository), std::make_unique<FakeState>("A"));
    }
};

TEST_F(RouterInMemoryTest, CurrentStateIsTheInitialState) {
    ASSERT_EQ(routerService->currentState().getCode(), "A");
}

TEST_F(RouterInMemoryTest, HasPendingStateChangeIsFalseInitially) {
    ASSERT_FALSE(routerService->hasPendingStateChange());
}

TEST_F(RouterInMemoryTest, RequestStateSchedulesWithoutChangingCurrentState) {
    routerService->requestState(std::make_unique<FakeState>("B"));

    ASSERT_TRUE(routerService->hasPendingStateChange());
    ASSERT_EQ(routerService->currentState().getCode(), "A"); // troca ainda não efetivada
}

TEST_F(RouterInMemoryTest, RequestStateWithSameCodeSchedulesReload) {
    // Pedir o código do estado atual é uma troca válida (reload deliberado da
    // cena) — antes da task 13 isso era indistinguível de "nada pendente".
    routerService->requestState(std::make_unique<FakeState>("A"));

    ASSERT_TRUE(routerService->hasPendingStateChange());
}

TEST_F(RouterInMemoryTest, CommitStateChangeUnloadsOldSceneAndPromotesNext) {
    EXPECT_CALL(*mockSceneRepository, unloadScene("A")).Times(1);

    routerService->requestState(std::make_unique<FakeState>("B"));
    routerService->commitStateChange();

    ASSERT_EQ(routerService->currentState().getCode(), "B");
    ASSERT_FALSE(routerService->hasPendingStateChange());
}

TEST_F(RouterInMemoryTest, CommitStateChangeWithoutPendingChangeIsNoOp) {
    EXPECT_CALL(*mockSceneRepository, unloadScene(testing::_)).Times(0);

    routerService->commitStateChange();

    ASSERT_EQ(routerService->currentState().getCode(), "A");
}

TEST_F(RouterInMemoryTest, CurrentSceneResolvesSceneOfCurrentState) {
    EXPECT_CALL(*mockSceneRepository, getScene("A")).WillOnce(testing::ReturnRef(mockScene));

    IScene& returnedScene = routerService->currentScene();

    ASSERT_EQ(&returnedScene, &mockScene);
}

// =============================================================================
// A FIACAO NULA e erro do chamador, e ele aparece na CONSTRUCAO
// =============================================================================
//
// Ate aqui so `EngineManager` e `FlowRouter` validavam. Aqui o nulo atravessava
// o construtor e so estourava no primeiro `currentState()` -- dentro do laco,
// longe de quem montou a fiacao.

TEST(RouterInMemoryNullTest, RepositorioNuloERecusadoNaConstrucao)
{
    EXPECT_THROW(RouterInMemory(nullptr, std::make_unique<FakeState>("menu")), std::invalid_argument);
}

TEST(RouterInMemoryNullTest, EstadoInicialNuloERecusadoNaConstrucao)
{
    EXPECT_THROW(RouterInMemory(std::make_unique<SceneRepository>(), nullptr), std::invalid_argument);
}

TEST(RouterInMemoryNullTest, AgendarEstadoNuloERecusado)
{
    RouterInMemory router{ std::make_unique<SceneRepository>(), std::make_unique<FakeState>("menu") };

    EXPECT_THROW(router.requestState(nullptr), std::invalid_argument);
    // E a recusa nao pode deixar meia navegacao pendurada.
    EXPECT_FALSE(router.hasPendingStateChange());
}
