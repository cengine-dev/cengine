#pragma once

#include <memory>

#include <cengine/core/IGameManager.hpp>
#include <cengine/core/IWindowManager.hpp>

namespace cengine::core {

/**
 * @brief O coração da engine: o game loop.
 *
 * É a única classe concreta do `core`. Recebe por injeção de dependência um
 * `IWindowManager` (a plataforma gráfica) e um `IGameManager` (as regras/telas
 * do jogo), assumindo posse de ambos, e roda o loop principal até o jogo pedir
 * para sair.
 *
 * Cada iteração executa: `window.update()` → `game.onEnter()` → `game.input()`
 * → `game.render()` → `game.onExit()`, e para quando `game.shouldExit()` retorna
 * true — encerrando com `cleanup()`.
 *
 * @code
 * cengine::core::EngineManager engine{
 *     std::make_unique<MyWindowManager>(),
 *     std::make_unique<MyGameManager>()
 * };
 * engine.start(); // bloqueia até o jogo pedir para sair
 * @endcode
 */
class EngineManager {
    void run();
    std::unique_ptr<IWindowManager> m_windowManager;
    std::unique_ptr<IGameManager> m_gameManager;
    bool m_isRunning;

public:
    /**
     * @param windowManager plataforma gráfica (posse transferida à engine).
     * @param gameManager   regras/telas do jogo (posse transferida à engine).
     */
    EngineManager(
        std::unique_ptr<IWindowManager> windowManager,
        std::unique_ptr<IGameManager> gameManager);

    ~EngineManager() = default;

    /// Inicializa a janela e entra no game loop. Bloqueia até `shouldExit()`.
    void start();

    /// Libera recursos do jogo e da janela. Invocado ao final de `start()`.
    void cleanup() const;
};

} // namespace cengine::core
