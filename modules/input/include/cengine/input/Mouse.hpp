#pragma once

#include <cstddef>
#include <vector>

namespace cengine::input {

/// Botao do ponteiro. So os dois que os jogos usam — nomear o do meio, os
/// laterais e a roda sem consumidor seria o deposito que o ADR 0002 recusa.
enum class MouseButton
{
    None,
    Left,
    Right,
};

/// Um clique, com a posicao DO MOMENTO em que aconteceu.
///
/// A posicao vai junto de proposito, e essa e a decisao mais importante deste
/// tipo: o ponteiro anda entre o aperto e o quadro em que a cena le a fila, e
/// ai "onde eu cliquei" e "onde o mouse esta" sao coisas diferentes. Sem isto,
/// um clique rapido seguido de um movimento acertaria a celula errada.
///
/// Coordenadas em PIXELS da area util da janela. Traduzir para o mundo do jogo
/// e trabalho do jogo (ver `cengine::grid2d` para o caso de grade).
struct MouseClick
{
    MouseButton button = MouseButton::None;
    float       x = 0.0f;
    float       y = 0.0f;
};

/// O CONTRATO de ponteiro entre a plataforma (que captura) e as cenas (que
/// consomem) — irmao do `Keyboard`, e pelas mesmas razoes.
///
/// Duas leituras, porque um jogo com ponteiro precisa das duas e elas nao se
/// substituem:
///
/// - **posicao e ESTADO** (`pushPosition`/`x`,`y`): "onde o ponteiro esta
///   AGORA". Serve para realce sob o cursor, que precisa da resposta todo
///   quadro.
/// - **clique e EDGE** (`pushClick`/`readClick`): "o jogador CLICOU". Um evento
///   por aperto fisico, consumido no maximo um por `input()` — mesmo limite da
///   fila de teclas, e pela mesma razao: sem ele, dois leitores no mesmo quadro
///   veem coisas diferentes.
///
/// ## De onde veio (ADR 0002, task 27)
///
/// Este vocabulario viveu no casco (`forgeui`, platform-theforge-common 0.6.0)
/// por dois jogos, de proposito: o enum de teclas so subiu na quarta copia
/// identica, e nao havia por que apressar o ponteiro. Subiu agora porque o
/// SEGUNDO consumidor (tactics, degrau 06) usou a forma do primeiro (bulwark,
/// degrau 05) **sem mudar nada** — que e o sinal mais forte de que a forma
/// esta certa.
///
/// O que este objeto NAO sabe: janela, foco, GPU, Win32. A plataforma empurra;
/// a cena le. E so.
class Mouse
{
public:
    /// Teto da fila. Cheia, o clique NOVO e descartado (e nao o antigo): quem
    /// esta na fila chegou primeiro e sera consumido primeiro.
    static constexpr size_t kQueueMax = 16;

    // --- lado da PLATAFORMA (quem captura) ---

    void pushPosition(float x, float y);
    void pushClick(MouseClick click);

    // --- lado das CENAS (quem consome) ---

    [[nodiscard]] float x() const { return m_x; }
    [[nodiscard]] float y() const { return m_y; }

    /// Consome no maximo um clique. Fila vazia devolve `MouseButton::None`.
    [[nodiscard]] MouseClick readClick();

private:
    std::vector<MouseClick> m_queue;
    float                   m_x = 0.0f;
    float                   m_y = 0.0f;
};

} // namespace cengine::input
