# 26 - Grade em pixels: celula <-> pixel

- **Status:** ESTACIONADA — **1/2 na metade que paga.** O gate NAO disparou.
- **Categoria:** Arquitetura (candidata a modulo opt-in, recorte a decidir)
- **Registrada em:** 2026-07-28 (revisao pos-Bulwark, 9o jogo)

## A candidata

Uma grade de `cols x rows` celulas de lado `S`, posicionada numa area de
`W x H` pixels, e a conversao nos DOIS sentidos:

- **ida:** `(col, row) -> retangulo em pixels` (para desenhar)
- **volta:** `(x, y) em pixels -> (col, row)`, ou "fora da grade" (para o
  ponteiro)

Nenhum vocabulario de jogo: nao sabe o que mora na celula.

## Evidencias

**Ida — 2 consumidores, codigo praticamente identico:**

- **delve @ 7a2610d**
  (`src/platform/theforge/src/DelveForge/scene/layer/ForgeWorldLayer.cpp:59-62`):

  ```cpp
  const float gridW  = static_cast<float>(dungeon.cols()) * kCell;
  const float gridH  = static_cast<float>(dungeon.rows()) * kCell;
  const float startX = (w - gridW) * 0.5f;
  const float startY = (h - gridH) * 0.5f;
  ```

- **bulwark @ 4554c8d**
  (`src/platform/theforge/src/BulwarkForge/scene/layer/ForgeWorldLayer.cpp:36-41`):
  a mesma conta, so nomeando o resultado (`Grid{originX, originY}`) porque
  aqui ela e usada por dois caminhos (desenho e ponteiro).

**Volta — 1 consumidor:**

- **bulwark @ 4554c8d** (`ForgeWorldLayer.cpp:43-64`, `cellAt`). Nenhum outro
  jogo escreveu isto, porque nenhum outro tem ponteiro.

**Variante que NAO conta como terceira evidencia:** o 8puzzle
(`ForgeBoardView.cpp:7-10`) centraliza so em X, em torno de um `centerX`
recebido, e o topo vem de fora (`topY`). E uma politica de layout diferente —
serve para mostrar que a CENTRALIZACAO nao e universal, o que aparece abaixo
no corte.

## Por que o gate NAO disparou (o argumento que importa)

**Pelo precedente da task 23 (`camera2d`).** La foi decidido, e esta escrito no
cabecalho do modulo, que a ORIGEM e politica do jogo — a engine so projeta.
Aplicando o mesmo corte aqui, o que restaria da IDA e:

```cpp
origin + col * cell
```

Uma multiplicacao e uma soma. A engine nao existe para guardar isso, e o bloco
de documentacao explicando o modulo seria maior que o modulo. **A ida passa no
criterio 2 e nao paga.**

A parte que PAGA e a volta, e ela tem **um** consumidor:

```cpp
if (localX < 0.0f || localY < 0.0f) { return false; }   // ANTES do cast
const auto candidateCol = static_cast<uint32_t>(localX / kCell);
if (candidateCol >= cols) { return false; }
```

Aquele primeiro `return` nao e decoracao: converter float negativo para
`uint32_t` e comportamento indefinido. Quem reescrevesse conferindo so
`>= cols` estaria apostando no wrap do valor indefinido — funciona na pratica
nos compiladores usuais e e exatamente o tipo de coisa que deve existir uma vez
so, com teste. **Mas "seria bom ter em um lugar so" e argumento de
especulacao; o ADR 0002 pede o segundo consumidor.**

## Vizinhanca com a `camera2d` (nao e sobreposicao)

A `camera2d` declara no proprio cabecalho:

> *"Escala, letterbox e centralizacao em PIXELS tambem ficam nas cenas: este
> modulo trabalha em unidades de MUNDO e nao sabe o tamanho da janela real."*

Esta candidata mora exatamente no espaco que aquela recusou de proposito. Nao
ha conflito a resolver — ha uma fronteira a lembrar quando o gate disparar: se
subir, sobe como coisa de PIXEL, e nao dentro da `camera2d`.

## O corte (se o gate um dia disparar)

- **Mecanismo (subiria):** dimensoes da grade, celula -> retangulo,
  pixel -> celula com rejeicao de borda correta.
- **Politica (fica no jogo):** o tamanho da celula (feel), COMO a origem e
  calculada (centralizar? ancorar? deixar espaco para HUD?) e o que existe em
  cada celula. O 8puzzle e a prova de que a centralizacao e uma escolha entre
  varias.

## O que destrava

Um **segundo jogo de grade COM ponteiro**. Ele fecha esta candidata junto com a
27 (mouse como porta) e com o limite espacial da task 18 — as tres sao a mesma
aposta, declarada na revisao do Bulwark
(`bulwark/.ai/task/09-revisao-candidatas.md`).
