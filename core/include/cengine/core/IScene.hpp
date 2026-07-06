#pragma once

namespace cengine::core {

/**
 * @brief Uma tela/estado renderizável do jogo (menu, gameplay, game over...).
 *
 * O jogo implementa esta interface; a engine apenas orquestra o ciclo de vida.
 * A engine é agnóstica de biblioteca gráfica: o que `draw()`/`input()` fazem por
 * baixo (SDL, Raylib, terminal...) é decisão do jogo.
 *
 * ## Ciclo de vida (por iteração do game loop)
 * A cada volta do loop o gerenciador de jogo chama, nesta ordem:
 * `onEnter()` (só na primeira vez em que a cena é ativada) → `input()` →
 * `draw()`. Ao trocar de cena, a cena que sai recebe `onExit()`.
 *
 * O par `onEnterExecuted()`/`isOnEnterExecuted()` existe para garantir que
 * `onEnter()` rode **uma única vez** por ativação, mesmo que a cena permaneça
 * ativa por muitas iterações.
 *
 * @note Contrato de tempo de vida: referências a uma cena obtidas via
 *       `cengine::routing::IRouter::currentScene()` só valem até a próxima
 *       navegação. Nunca retenha uma `IScene&` através de uma troca de estado.
 */
class IScene {
public:
    IScene() = default;
    virtual ~IScene() = default;

    /// Inicialização única da cena (carregar recursos, montar objetos).
    /// Chamado uma vez por ativação, antes do primeiro `input()`/`draw()`.
    virtual void onEnter() = 0;

    /// Marca `onEnter()` como já executado (chamado pela engine após `onEnter()`).
    virtual void onEnterExecuted() = 0;

    /// @return true se `onEnter()` já rodou nesta ativação (evita reexecução).
    [[nodiscard]] virtual bool isOnEnterExecuted() const = 0;

    /// Desenha o quadro atual. Chamado toda iteração enquanto a cena está ativa.
    virtual void draw() = 0;

    /// Processa entrada do usuário. Chamado toda iteração, antes de `draw()`.
    virtual void input() = 0;

    /// Finalização da cena (liberar recursos). Chamado ao sair/trocar de cena.
    virtual void onExit() = 0;
};

} // namespace cengine::core
