#include <cengine/routing/RouterInMemory.hpp>

#include <stdexcept>
#include <utility>

namespace cengine::routing {

RouterInMemory::RouterInMemory(std::unique_ptr<ISceneRepository> sceneRepository,
                               std::unique_ptr<IState> initialState)
    : m_sceneRepository(std::move(sceneRepository)),
      m_currentState(std::move(initialState)) {
    // Mesma regra do `EngineManager` e do `FlowRouter`: argumento nulo e erro do
    // CHAMADOR, e ele aparece na construcao. Sem isto, o nulo so se manifestava
    // no primeiro `currentState()` -- ou seja, dentro do laco, longe de quem
    // montou a fiacao.
    if (!m_sceneRepository) {
        throw std::invalid_argument("RouterInMemory: sceneRepository must not be null");
    }
    if (!m_currentState) {
        throw std::invalid_argument("RouterInMemory: initialState must not be null");
    }
}

void RouterInMemory::requestState(std::unique_ptr<IState> state) {
    // Agendar "nenhum estado" nao e uma navegacao -- e um cancelamento que
    // ninguem pediu. Aceitar em silencio deixaria o `hasPendingStateChange()`
    // devolver false e a transicao simplesmente nao acontecer.
    if (!state) {
        throw std::invalid_argument("RouterInMemory::requestState: state must not be null");
    }
    m_nextState = std::move(state);
}

bool RouterInMemory::hasPendingStateChange() const {
    return m_nextState != nullptr;
}

void RouterInMemory::commitStateChange() {
    if (!m_nextState) {
        return;
    }

    m_sceneRepository->unloadScene(m_currentState->getCode());
    m_currentState = std::move(m_nextState);
}

const IState& RouterInMemory::currentState() const {
    return *m_currentState;
}

core::IScene& RouterInMemory::currentScene() {
    return m_sceneRepository->getScene(m_currentState->getCode());
}

} // namespace cengine::routing
