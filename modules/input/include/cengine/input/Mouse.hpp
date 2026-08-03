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

/// Um gesto de ARRASTAR em andamento. `active == false` quando nao ha nenhum.
///
/// Ele carrega DUAS posicoes, e nao uma, porque e disso que um arrasto e feito:
/// a origem diz O QUE se pegou, o ponto atual diz para onde a mao foi. Um clique
/// nao precisa disso porque comeca e termina no mesmo instante.
struct DragState
{
    bool  active = false;
    float startX = 0.0f; ///< onde o botao foi apertado
    float startY = 0.0f;
    float x = 0.0f; ///< onde o ponteiro esta agora
    float y = 0.0f;
};

/// Um gesto que TERMINOU: as duas pontas, juntas.
struct Drop
{
    bool  happened = false;
    float startX = 0.0f;
    float startY = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
};

/// O CONTRATO de ponteiro entre a plataforma (que captura) e as cenas (que
/// consomem) — irmao do `Keyboard`, e pelas mesmas razoes.
///
/// Tres leituras, e nenhuma substitui outra:
///
/// - **posicao e ESTADO** (`pushPosition`/`x`,`y`): "onde o ponteiro esta
///   AGORA". Serve para realce sob o cursor, que precisa da resposta todo
///   quadro.
/// - **clique e EDGE** (`pushClick`/`readClick`): "o jogador CLICOU". Um evento
///   por aperto fisico, consumido no maximo um por `input()` — mesmo limite da
///   fila de teclas, e pela mesma razao: sem ele, dois leitores no mesmo quadro
///   veem coisas diferentes.
/// - **arrasto: ESTADO e EDGE juntos** (`drag()` e `readDrop()`). Um gesto de
///   arrastar precisa dos dois ao mesmo tempo — o estado para desenhar o que
///   esta na mao, todo quadro; o edge para agir quando a mao solta, uma vez.
///
/// ## O arrastar nao cabe na fila de cliques
///
/// Um clique e um EVENTO: aconteceu, tem uma posicao, acabou. Arrastar e um
/// CICLO DE VIDA com duas pontas que carregam posicoes DIFERENTES, e um meio em
/// que o jogo precisa desenhar o que esta na mao.
///
/// E o MESMO aperto abre as duas leituras: a plataforma empurra um clique E
/// comeca um arrasto. **Nao da para saber qual dos dois o jogador quis ate ele
/// soltar** — que e a formulacao mais curta de por que isto nao e um clique.
///
/// ## O que esta porta NAO decide
///
/// Se um gesto foi "clique" ou "arrasto". Ela entrega onde apertou e onde
/// soltou; **quantos pixels de folga ainda contam como clique parado e politica
/// do JOGO** — depende do tamanho do alvo e de quanto tremor de mao se perdoa.
/// Os dois consumidores escolheram numeros diferentes, e por motivos diferentes.
///
/// ## De onde veio (ADR 0002, tasks 27 e 28)
///
/// A posicao e o clique subiram na 0.14.0 (task 27); o arrastar, na 0.15.0
/// (task 28). Os dois pelo mesmo caminho, e ele e o caminho da casa: viveram no
/// casco (`forgeui`) enquanto tinham UM consumidor, e subiram quando o SEGUNDO
/// usou a forma do primeiro **sem pedir nenhuma mudanca de API**.
///
/// O que este objeto NAO sabe: janela, foco, GPU, Win32. A plataforma empurra; a
/// cena le. E so. (O `SetCapture` que o arrastar exigiu ficou la, e nunca foi
/// candidato: o WndProc E o casco.)
class Mouse
{
public:
    /// Teto das filas. Cheia, o evento NOVO e descartado (e nao o antigo): quem
    /// esta na fila chegou primeiro e sera consumido primeiro.
    static constexpr size_t kQueueMax = 16;

    // --- lado da PLATAFORMA (quem captura) ---

    /// Onde o ponteiro esta. Um gesto em andamento acompanha: e o que permite a
    /// cena desenhar o que esta na mao seguindo o cursor.
    void pushPosition(float x, float y);

    void pushClick(MouseClick click);

    /// Comeca um gesto de arrastar (o botao desceu).
    void pushDown(float x, float y);

    /// Termina o gesto e enfileira o `Drop` (o botao subiu). Soltar sem ter
    /// apertado aqui — o aperto foi noutra janela — nao gera nada.
    void pushUp(float x, float y);

    /// Cancela o gesto em andamento SEM gerar `Drop`: a janela perdeu o foco, ou
    /// alguem tomou a captura do ponteiro, e o "soltou" pode nunca chegar.
    ///
    /// Cancelar e melhor que soltar num destino que o jogador nao escolheu — e e
    /// o irmao do `Keyboard::clearHeldKeys()`, pelo mesmo motivo.
    void cancelDrag();

    // --- lado das CENAS (quem consome) ---

    [[nodiscard]] float x() const { return m_x; }
    [[nodiscard]] float y() const { return m_y; }

    /// Consome no maximo um clique. Fila vazia devolve `MouseButton::None`.
    [[nodiscard]] MouseClick readClick();

    /// O gesto em andamento. Ler NAO consome: e estado.
    [[nodiscard]] DragState drag() const { return m_drag; }

    /// Consome no maximo um gesto terminado. Sem nenhum, devolve
    /// `happened == false`.
    [[nodiscard]] Drop readDrop();

private:
    std::vector<MouseClick> m_clicks;
    std::vector<Drop>       m_drops;
    DragState               m_drag;

    float m_x = 0.0f;
    float m_y = 0.0f;
};

} // namespace cengine::input
