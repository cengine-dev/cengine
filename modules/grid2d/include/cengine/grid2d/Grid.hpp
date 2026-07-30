#pragma once

// cengine::grid2d (task 26): a conversao entre CELULA e PIXEL, nos dois
// sentidos.
//
// ## De onde veio (ADR 0002)
//
// Extraido de tres copias a mao, das quais duas eram o mecanismo inteiro:
//
// - **delve @ 7a2610d** (`ForgeWorldLayer.cpp:59-62`) — so a IDA: grade
//   centrada, celula -> retangulo, para desenhar o andar.
// - **bulwark @ 4554c8d** (`ForgeWorldLayer.cpp:36-64`) — ida e VOLTA. Primeiro
//   jogo com ponteiro do ecossistema.
// - **tactics @ 5c59205** (`ForgeBoardLayer.cpp:86-120`) — ida e volta de novo,
//   e a `cellAt` dele e a do bulwark linha a linha, mudando so o tipo inteiro
//   (`int` x `uint32_t`) e o jeito de perguntar os limites.
//
// A VOLTA e a metade que paga, e ela e que trouxe o gate: enquanto tinha um
// consumidor so, a task 26 ficou estacionada de proposito.
//
// ## O que NAO subiu, e por que
//
// **Onde a grade COMECA.** Os tres jogos centralizam na janela com a mesma
// conta — e mesmo assim ela fica fora, porque o tactics ja desvia dela (soma 20
// pixels para caber o titulo) e o 8puzzle centraliza so em X. E o mesmo corte
// da `camera2d` (task 23), que deixou "escala, letterbox e centralizacao em
// PIXELS" nas cenas: a engine projeta, o jogo decide de onde.
//
// O que sobra da IDA depois desse corte e `origem + coluna * lado` — uma
// multiplicacao e uma soma. Sozinha ela nao pagaria um modulo; ela sobe de
// carona na VOLTA, para os dois sentidos nao viverem em lugares diferentes e
// um dia divergirem.
//
// ## Vizinhanca com a camera2d
//
// A `camera2d` trabalha em unidades de MUNDO e nao sabe o tamanho da janela.
// Este modulo trabalha em PIXELS. Sao vizinhos e nao concorrentes: um jogo com
// rolagem E grade usaria os dois em sequencia, e nenhum dos dois sabe do outro.

namespace cengine::grid2d {

/// Uma grade desenhada em pixels: onde ela comeca, quanto mede cada celula, e
/// quantas cabem.
///
/// A ORIGEM vem pronta: quem a calcula e o jogo (centralizar? ancorar? deixar
/// espaco para um painel?). Ver o bloco acima.
struct Grid
{
    float originX = 0.0f; ///< canto superior esquerdo, em pixels
    float originY = 0.0f;
    float cellSize = 1.0f; ///< lado da celula, em pixels
    int   cols = 0;
    int   rows = 0;
};

/// Retangulo de uma celula, em pixels.
struct Rect
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

/// Celula -> retangulo. Nao valida limites: desenhar fora nao quebra nada, e
/// quem varre a grade ja varre dentro dela.
[[nodiscard]] Rect cellRect(const Grid& grid, int col, int row);

/// A celula esta dentro da grade?
[[nodiscard]] bool inside(const Grid& grid, int col, int row);

/// Pixel -> celula. Devolve `false` quando o ponto cai fora da grade.
///
/// **A ordem dos testes e a regra, e nao detalhe de implementacao.** O ponto e
/// levado para o espaco da grade, e o teste de NEGATIVO vem ANTES da divisao e
/// da conversao para inteiro. Escrito na ordem "natural" — converter e depois
/// conferir so o limite de cima — o codigo funciona por acidente com `int` e e
/// comportamento indefinido com inteiro sem sinal, que foi o tipo que o
/// primeiro consumidor usou.
///
/// E o unico lugar do modulo onde da para errar em silencio; e por causa dele
/// que a VOLTA vale um modulo e a ida, sozinha, nao valeria.
[[nodiscard]] bool cellAt(const Grid& grid, float x, float y, int& col, int& row);

} // namespace cengine::grid2d
