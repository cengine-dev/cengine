#pragma once

namespace cengine::core {

/**
 * @brief Abstrai a janela/plataforma gráfica sobre a qual o jogo roda.
 *
 * É o ponto de extensão que mantém a engine **agnóstica de biblioteca gráfica**:
 * o jogo fornece uma implementação (SDL, Raylib, GLFW, terminal...) e a engine
 * só a aciona pelo contrato abaixo.
 *
 * Chamadas pelo `EngineManager`: `init()` uma vez em `start()`, `update()` toda
 * iteração do loop (processar eventos da janela, swap de buffers), e `cleanup()`
 * uma vez ao encerrar.
 */
class IWindowManager {
public:
    virtual ~IWindowManager() = default;

    /// Cria/inicializa a janela e o contexto gráfico. Chamado uma vez.
    virtual void init() = 0;

    /// Atualiza a janela por iteração (eventos de SO, apresentação do quadro).
    virtual void update() = 0;

    /// Destrói a janela e libera recursos gráficos. Chamado uma vez, ao sair.
    virtual void cleanup() = 0;
};

} // namespace cengine::core
