#pragma once
#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/IRouter.hpp>
#include <cengine/routing/ISceneRepository.hpp>

namespace cengine::routing {

/**
 * @brief `IRouter` em memória, apoiado em um `ISceneRepository`.
 *
 * Delega o estado e as cenas ao repositório; `commitStateChange()` descarrega a
 * cena do estado que sai e promove o próximo. `currentScene()` resolve a cena do
 * estado atual sob demanda (instanciação lazy pelo repositório).
 */
class RouterInMemory final : public IRouter {
    std::shared_ptr<ISceneRepository> m_sceneRepository;

public:
    explicit RouterInMemory(std::shared_ptr<ISceneRepository> sceneRepository);
    ~RouterInMemory() override = default;

    void requestState(std::unique_ptr<IState> state) override;
    [[nodiscard]] bool hasPendingStateChange() const override;
    void commitStateChange() override;

    [[nodiscard]] const IState& currentState() const override;
    [[nodiscard]] core::IScene& currentScene() override;
};

} // namespace cengine::routing
