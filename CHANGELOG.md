# Changelog

All notable changes to CEngine are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.15.0] - 2026-08-03

O ponteiro aprende a **ARRASTAR**: `DragState` + `Drop` na
`cengine::input::Mouse` (task 28).

> **Nao e breaking.** Puramente aditivo — os metodos novos convivem com a
> posicao e a fila de cliques da 0.14.0, e o casco (`platform-theforge-common`
> 0.12.0) delega mantendo `forgeui::DragState`/`forgeui::Drop` como alias.
> **Nenhum jogo mudou uma linha.**

### Added

- **`DragState`** (estado) — `active`, a origem (`startX`/`startY`) e o ponto
  atual (`x`/`y`). Lido todo quadro, para desenhar o que esta na mao.
- **`Drop`** (edge) — um gesto que TERMINOU, com as duas pontas juntas.
  Consumido no maximo um por `input()`, mesma politica das outras filas.
- **`pushDown`/`pushUp`/`cancelDrag`** — o lado da plataforma. `pushPosition`
  passa a mover o gesto em andamento, e so ele.

### Por que arrastar nao cabia na fila de cliques

Um clique e um EVENTO: aconteceu, tem uma posicao, acabou. Arrastar e um CICLO
DE VIDA com duas pontas que carregam posicoes DIFERENTES — a origem diz O QUE se
pega, o destino diz PARA ONDE vai — e um meio em que o jogo precisa desenhar o
que esta na mao.

E o MESMO aperto abre as duas leituras: a plataforma empurra um clique E comeca
um arrasto. **Nao da para saber qual dos dois o jogador quis ate ele soltar.**

### Por que agora, e nao antes

O vocabulario nasceu LOCAL no casco (common 0.10.0, Klondike) e ficou la ate o
segundo consumidor aparecer. Ele apareceu no **cue, degrau 06** (a tacada por
arrasto), usou as duas leituras para o que elas foram feitas, leu os quatro
campos de cada uma e **nao pediu nenhuma mudanca de API**.

E o mesmo caminho da task 27 (o mouse), e a terceira porta de input com o par
estado+edge: teclado (`isHeld`/`readKey`, 0.8.0), ponteiro (`x`,`y`/`readClick`,
0.14.0), arrastar (`drag`/`readDrop`, 0.15.0).

### O que NAO subiu

**O `SetCapture`.** Sem capturar o ponteiro no aperto, soltar o botao fora da
janela nao gera `WM_LBUTTONUP` e o gesto fica pendurado — mas isso e Win32, e o
WndProc E o casco. O que subiu foi o `cancelDrag()`, que e a resposta em
vocabulario de porta ao que o casco descobre em vocabulario de janela.

**A folga de "isto foi um clique ou um arrasto?".** Ela e politica do JOGO, e o
segundo consumidor provou por que: o Klondike usa 8 pixels para uma carta de 90,
o cue usa 12 para uma bola de 20, e os dois medem perguntas diferentes.

**O instante em que o gesto comecou.** O cue notou a falta, mediu o caso e
decidiu nao pedir: e informacao que o proprio jogo tem, com um `bool`.

## [0.14.0] - 2026-07-30

*(Entrada escrita em retrospecto na 0.15.0: a versao foi lancada e etiquetada
sem passar por aqui. Mesmo tipo de buraco de registro que a task 07 do common e
as tasks 26/27 desta engine — e achado do mesmo jeito, procurando o vizinho para
atualiza-lo.)*

As duas extracoes do **Tactics** (10o jogo), que fecharam a revisao dele sem
divida.

### Added

- **`cengine::grid2d`** (task 26) — a conversao entre pixel e celula, nos dois
  sentidos.
- **`cengine::input::Mouse`** (task 27) — a porta de ponteiro: posicao (estado)
  e clique (edge), irma da porta de teclado da task 20. O `MouseClick` carrega a
  posicao DO MOMENTO em que aconteceu, e nao a de agora.

O casco (`platform-theforge-common` 0.9.0) passou a delegar, com
`forgeui::MouseClick` como alias — nenhum jogo mudou uma linha.

## [0.13.0] - 2026-07-28

A janela ganha voz para encerrar o loop: **`IWindowManager::shouldClose()`**.

> **Nao e breaking.** O metodo NAO e virtual puro — nasce devolvendo `false`,
> entao quem nao tem janela para fechar (terminal) herda e nao muda uma linha.

### Added

- **`IWindowManager::shouldClose()`**: a plataforma avisa que o usuario fechou
  a janela (X, alt-F4). O loop do modo proprio para no fim do quadro, DEPOIS
  do `present()` — o quadro ja desenhado e apresentado — e o `cleanup()` roda
  normalmente, igual ao `shouldExit` do jogo.

### O buraco que isto fecha

Ate aqui o UNICO jeito de o loop terminar era o JOGO rotear para "exit". A
ponte do The-Forge contornava empurrando um `Escape` FALSO na fila de teclas
quando o X era clicado, e isso funcionava por sorte: so nos jogos em que ESC
ja significava "sair". Nos que usam ESC para "voltar ao menu" — o Delve e o
Bulwark — clicar no X levava ao MENU.

A distincao que faltava: **"o jogador pediu para sair" (decisao de jogo) e "o
sistema mandou fechar" (fato da plataforma) nao sao a mesma coisa** e nao
podem dividir o mesmo canal. Reportado pelo dono jogando o Bulwark.

## [0.12.0] - 2026-07-28

Fecha a task 25 em **2/2**: `ClipDesc::loop` e `Animator::finished()`.

> **Nao e breaking.** `loop` nasce `true`, que e o comportamento de sempre.

### Added

- **`ClipDesc::loop`** (default `true`): `false` faz o clip tocar uma vez e
  TRAVAR no ultimo quadro.
- **`Animator::finished()`**: `true` quando um clip que nao cicla ja passou
  por todos os quadros. Clip em loop nunca termina.

### Por que agora

O `starforce::ExplosionAnimator` (2026-07-22) foi a primeira evidencia: 3
quadros que precisavam tocar uma vez e travar, resolvidos com um embrulho por
fora do `Animator` — relogio proprio, `finished()` local, e o cuidado de nunca
deixar o wrap interno escapar. Ficou registrado como 1/2, com o gate explicito:
"se um segundo jogo precisar do mesmo toca-e-trava, a comparacao decide".

O segundo apareceu no Bulwark (degrau 07): a explosao de abate escreveria o
MESMO embrulho, linha por linha. Duas copias do mesmo remendo sao o sinal de
que faltava um eixo aqui dentro — nao de que o remendo era a resposta.

A suite ENCARNA o caso do consumidor congelado, como a Emenda 1 do ADR 0002
exige: os 3 quadros a 1/12s dos testes sao os do `ExplosionAnimator.h`, com a
origem citada. O Star Force nao compila mais contra esta versao; o teste e o
que mantem o caso dele vivo dentro da engine.

## [0.11.0] - 2026-07-28

A promocao que a task 18 esperava desde 2026-07-14: **`cengine::routing::SceneStack`**,
cenas EMPILHADAS. Nao foi desenhada aqui — foi **extraida** do
`delve::LayerStack` (delve@f9cd31b), com o Bulwark como consumidor de
validacao.

> **Nao e breaking.** `SceneStack` e nova; nada existente mudou. O `IRouter`
> segue trocando uma cena por vez, e quem nao empilha nao paga nada.

### Added

- **`cengine::routing::SceneStack`**: `push(layer, consumesInput = true)`,
  `pop()`, `replaceBottom(layer)`, `isTop(layer)`, mais `update`/`draw`/`input`.
  A camada e `core::IScene` — isto empilha CENAS, nao um tipo novo.

  A regra: `update(dt)` e `draw()` chegam em TODAS as camadas, de baixo para
  cima; `input()` chega SO na primeira camada ATIVA a partir do topo.

### Sobre o desenho, e o que NAO subiu

O desenho que a task 18 imaginava antes de existir consumidor tinha tres
politicas por camada: `blocksInputBelow`, `updatesBelow`, `drawsBelow`.
Depois de cinco tipos de camada em uso real (mundo, HUD, pausa, mochila, fim
de partida):

- **`updatesBelow` e `drawsBelow` nao subiram**: zero evidencia. Nenhuma
  camada precisou parar as de baixo. O caso que motivava a flag ("a pausa
  congela o jogo") foi resolvido pelo DOMINIO do consumidor, e congelar pela
  pilha teria destruido a evidencia do proprio gate — que e a cena de baixo
  continuar rodando.
- **`blocksInputBelow` apareceu com outra forma**: nao "esta camada bloqueia
  as de baixo?" e sim **"esta camada PARTICIPA do input?"** — que e o
  `consumesInput`. Um HUD desenha por cima e nao quer tecla nenhuma; sem esse
  eixo, o primeiro HUD empilhado deixa o jogo inteiro sem resposta (aconteceu
  no Delve, com a suite verde, porque nenhum teste tinha camada passiva).
- **`replaceBottom` NAO estava no desenho** e era a terceira condicao do
  proprio gate: trocar a cena de baixo mantendo os overlays.

### Limite conhecido

A regra "so a primeira camada ATIVA do topo recebe input" e correta para
overlay MODAL (pausa, dialogo, menu que para o jogo) e **insuficiente para
overlay clicavel NAO-modal** — um painel lateral que se clica enquanto o jogo
roda embaixo. Com teclado a pergunta nao aparece; com PONTEIRO ela vira
espacial ("o clique caiu no painel ou no mapa?"), e a pilha nao tem como
saber, porque `IScene::input()` nao reporta se consumiu.

Registrado como limite, nao corrigido — e o motivo NAO e custo de migracao.
Uma primeira redacao disto dizia "mudar o contrato do `IScene` que 9 jogos
implementam", e isso estava errado: o ADR 0003 pina os consumidores em
versoes especificas, entao um breaking na 0.12.0 nao obrigaria ninguem a
migrar. O motivo verdadeiro e o criterio 2 do ADR 0002, o de sempre: UM
consumidor precisou disto, e uma API desenhada por um caso so tem o formato
daquele caso. O Bulwark resolveu localmente (compara o clique com a area do
painel antes de traduzir para celula). Se um segundo jogo bater no mesmo
ponto, a comparacao decide o formato — e ai o breaking se paga.

## [0.10.0] - 2026-07-19

Two promotions in one release, both **opt-in**, both with the ADR 0002 gate
closed by mario-bros and the zelda-like: the camera's **transform** (task 23) and
the animation **clip machine** (task 25).

> **Not a breaking release.** `cengine::camera2d` and `cengine::anim` are new
> modules; nothing existing changed.

### Added

- **`cengine::camera2d`** (`CENGINE_BUILD_CAMERA2D`, default `ON`):
  - `Viewport { origin, size, cullMargin }`, `worldToView` (the subtraction) and
    `visible` (culling against the view inflated by the margin).
  - Only the half that was **identical, line for line**, in both games rises.
    **Following** — anchor on the focus, clamp to the level — is *feel*, and it
    diverged in the way that proves the cut: mario scrolls on one axis, the
    zelda-like on two. Scaling and letterboxing stay in the scenes, which are the
    ones that know the window.
- **`cengine::anim`** (`CENGINE_BUILD_ANIM`, default `ON`):
  - `Animator` over a table of `ClipDesc { frameCount, frameTime }`: switching
    clips resets frame *and* time, only multi-frame cycles advance, and an id
    outside the table is a no-op (the same forgiving contract as `play(id)`).
  - Clip **selection**, facing and the atlas region table stay in the games —
    the engine never learns what `Walk` means. Space Invaders, which animates by
    domain rule (the march tick) rather than by clock, is the counter-example
    that made the module opt-in rather than part of `core`.

### Why now (the gate)

Both candidates had been parked with their evidence recorded but deliberately
**not** promoted (mario-bros closed 1/2 for each; the zelda-like closed 2/2 and
still only registered it). The extraction is its own step: the mechanism was
compared side by side first — down to the animation's 0.12s / 2-frame cadence —
and only then lifted, validated by re-pointing **one** consumer.

Per ADR 0003's Amendment 1, the frozen game does not migrate: mario-bros stays
pinned at 0.9.0 and pays the toll by having its cases embodied in the suite. The
tests cite their origins: mario-bros @ `4a8f825` (camera) and @ `8dfbb90`
(`PlayerAnimator`), zelda @ `9658ae0` (camera) and @ `3a3abda` (`HeroAnimator`),
with the games' own values transcribed — 16-unit cull margin, 320×180 view,
0.12s two-frame walk.

## [0.9.0] - 2026-07-17

The audio **port** becomes an opt-in module (task 24). The speaker never will.

> **Not a breaking release.** `cengine::audio` is a new header-only module and
> changes nothing else.

### Added

- **`cengine::audio`** (`CENGINE_BUILD_AUDIO`, default `ON`):
  - `Player` — one contract, `play(id)` and nothing else: the minimal shape both
    real consumers confirmed (neither ever needed volume, priority or stop).
  - `SoundId` is a plain number: sound catalogs differ per game, so the engine
    carries the *request*, never the meaning. Enum sugar keeps
    `audio.play(Sound::Jump)` in the scenes.
  - Immediate dispatch (no consumer asked for a queue or a pump); **mute is
    normal degradation of the contract, not an error**.
  - Synthesis, voice pools and XAudio2 stay in the platform — the same cut as
    the input port (task 20), in the opposite direction: there the platform
    pushes and the scene reads; here the scene asks and the platform delivers.

### Why now (the gate)

The gate fired at 2/2 with the *same* mechanic: breakout's `AudioPlayer`
(@ `31dc850`) and its deliberate copy in mario-bros (@ `0fab493`), both
transcribed and cited in the suite under the provenance rule.

## [0.8.0] - 2026-07-14

The keyboard **contract** becomes an opt-in module (task 20). The **capture**
never will.

> **Not a breaking release.** `cengine::input` is a new opt-in module; it depends
> on neither `core` nor `routing` and changes neither.

### Added

- **`cengine::input`** (`CENGINE_BUILD_INPUT`, default `ON`):
  - `Key` / `KeyEvent` — the vocabulary the *scenes* speak. No scancodes, no
    virtual keys, no terminal events: translating the real keyboard is the
    platform's job and never enters the engine.
  - `Keyboard` — the mechanism the games had been copying into every bridge,
    with its **two readings**, which do not substitute for one another:
    - the **edge queue** (`pushKey` / `readKey`): *the player pressed*. At most
      **one event per `input()`** — that cap is semantics, not detail: it is what
      stops a repeated key from walking through three menu items in one frame.
    - the **held state** (`pushHeldKey` / `isHeld` / `heldAxis`): *the key is
      down right now*. A ship does not move on edges; it moves while the arrow
      is held, every frame.
  - `clearHeldKeys()` for focus loss — without it the KEYUP never arrives and
    the ship flies away on its own when the window comes back.

### Why now (the gate)

Task 20 had been parked behind an explicit gate ("scenes are per-platform, so a
shared vocabulary buys nothing"). Three things changed:

- the `Key` enum got copied a **fourth** time, into `platform-theforge-common` —
  the bridge that is now shared by every The-Forge game;
- ADR 0002's Amendment 1 made **frozen games count as evidence**, so the 8puzzle
  and Space Invaders copies do count;
- Asteroids **grew the contract** (`Key::Space`, the held state itself), proving
  that it evolves — and that today it evolves in copies, with no owner.

Per Amendment 1's toll, the suite embodies the frozen consumers' cases, with
their origins cited: the 8puzzle's menu navigation (`src/platform/ftxui/
Keyboard.h`, three keys → three frames) and the Space Invaders cannon
(`spaceinvaders@bb4e9b1`, `ForgeUi.h:62-76`), whose pre-chewed
`pushHeldState(moveAxis, fireHeld)` pair is shown to be expressible by the
generic per-key mechanism — if it weren't, the promotion would be wrong.

## [0.7.1] - 2026-07-14

Provenance in the tests. No API change — `collision2d` is byte-for-byte the same
module; only its suite and the ADR changed.

### Changed

- **Real-consumer tests now cite their origin** (ADR 0002, Amendment 1 — new
  *provenance rule*): repository @ commit, file and line the scene was
  transcribed from, with the game's own values instead of invented ones.
  - **This caught a lie.** The 0.7.0 tests wrote the Space Invaders player shot
    as `1x4`; the frozen game uses **`3x7`** (`World.cpp:15-16`). The test
    passed, and misrepresented the very case it existed to prove. All Space
    Invaders measurements are now transcribed from
    `spaceinvaders@bb4e9b1` (the commit the game is parked at — the repo has no
    tags, and frozen code makes for stable citations).
  - Without provenance a test *asserts* a use case; with it, the test **proves**
    one, and any reader can open the cited file and check.
- **New test: the frozen game's edge contract is preserved.** `si::Rect::
  intersects` uses strict comparisons (touching boxes don't collide) and so does
  `cengine::collision2d::intersects` — i.e. Space Invaders would not change
  behavior if it were ever migrated. That is the promise a promotion makes, and
  now it is executable.

## [0.7.0] - 2026-07-14

2D collision **detection** as an opt-in module (task 17), and the promotion rule
that let it in (ADR 0002, Amendment 1).

> **Not a breaking release.** `cengine::collision2d` is a new opt-in module; it
> does not depend on `core` or `routing` and changes neither. Consumers that
> don't link it are unaffected.

### Added

- **`cengine::collision2d`** (`CENGINE_BUILD_COLLISION2D`, default `ON`) — pure
  geometry, no dependency on `core` or `routing`:
  - `Aabb`, `Circle`, `Vec2`;
  - `intersects(Aabb, Aabb)`, `intersects(Circle, Circle)`,
    `intersects(Circle, Aabb)` and the symmetric overload.
  - Edge contract, documented in the header: **touching boxes do not
    intersect** (zero overlap area), **tangent circles do** (a shot must not
    graze a rock without hitting it). Circle × box uses the true distance to the
    closest point on the box, so a circle near a corner does not falsely hit.
  - The engine **detects**; the game still owns entities, decides what a hit
    means, scores, kills and ends the run.
  - **The world's shape is not the engine's business.** A wrapping arena is game
    policy: the consumer computes the shortest delta in its own topology and
    asks with an already-corrected position (Asteroids' torus does exactly this;
    Space Invaders, whose arena doesn't wrap, asks directly).

### Changed

- **ADR 0002 — Amendment 1: a parked game still counts as evidence.** Criterion
  2 ("≥ 2 real consumers") was being read as "≥ 2 consumers that will *link* the
  module". Since both finished games are parked at 0.5.0 (ADR 0003), that reading
  would veto every future promotion — and would turn "living documentation" into
  plain abandonment. Freezing a repository suspends its *maintenance*, not the
  *learning* it produced. The criterion now measures **evidence of need**.
  - **The toll:** a promotion backed by frozen evidence must have the engine's
    own suite **embody the frozen consumer's use case** — `collision2d`'s tests
    reproduce Space Invaders' shot × invader and bomb × cannon over the module,
    proving the mechanism expresses that game's real situation without
    unfreezing it. If the module ever stops serving those tests, it has stopped
    serving the learning that justified it.
  - The amendment does **not** loosen criterion 1: mechanism × policy is still
    cut by concept. Shapes go up; the arena's topology stays in the game.
- `project(cengine VERSION ...)` now tracks the release (it had been left at
  `0.5.0` through the 0.6.0 cycle).

## [0.6.0] - 2026-07-13

Explicit construction modes (tasks 21 and 19 of the
[improvement plan](.ai/task/README.md)): no more `nullptr` in the public API.
The engine's two modes — owned (`start()`) and hosted (`frame(dt)`) — are now
expressed by named factories at the call site, and the navigation-facade
mechanic repeated by the consumer games ships as an opt-in `FlowRouter<TFlow>`
helper in the routing module.

> **Breaking release.** The `EngineManager` constructor left the public API —
> construction goes through `EngineManager::owned(...)` or
> `EngineManager::hosted(...)`. See _Migrating from 0.5.0_ below.
>
> **Consumer note:** 8puzzle and spaceinvaders are **parked at 0.5.0** as
> living documentation (ADR 0003) and are not migrated; the asteroids game is
> the validating consumer of this release.

### Changed

- **`EngineManager` is constructed by named factories** (task 21, option B of
  the task — decision recorded there):
  - `EngineManager::owned(windowManager, gameManager, ...)` — engine-owned
    mode: the engine owns the window and the loop (`start()`). Throws
    `std::invalid_argument` on a null window or game manager.
  - `EngineManager::hosted(gameManager, ...)` — hosted mode: the host owns
    window, message pump and pacing, and drives the engine with `frame(dt)`.
    Takes no window at all; throws `std::invalid_argument` on a null game
    manager.
  - `start()` on a `hosted()` engine throws `std::logic_error` with a clear
    message (previously: silent undefined behavior via the null window).
  - The `windowManager == nullptr` convention of 0.4.0 is gone from the public
    API; internally the mode guarantees are established at construction, so
    the per-call null branches in the loop were removed.

### Added

- **`cengine::routing::FlowRouter<TFlow>`** (task 19, header-only, opt-in):
  the mechanic of the domain navigation facade — holds the `IRouter`,
  `current()` returns the current state downcast to the game's flow type
  (throwing `std::runtime_error` on a foreign state), and `setNextState()`
  delegates to the two-phase `requestState()`. The game inherits it and
  writes only its vocabulary (`menu()`, `gameOver()`, ...). Promotion passes
  the anti-deposit filter (ADR 0002): pure mechanism, duplicated in two real
  consumers, tested in-engine.
- **Construction/mode tests**: factories reject null collaborators and
  non-positive timing; `start()` on a hosted engine throws; `FlowRouter`
  covered for cast, foreign-state error, delegation and the facade pattern.

### Migrating from 0.5.0

- Engine-owned mode: replace the constructor with the factory —
  ```cpp
  auto engine = cengine::core::EngineManager::owned(
      std::move(windowManager), std::move(gameManager));
  ```
- Hosted mode: replace `EngineManager{nullptr, game}` with —
  ```cpp
  auto engine = cengine::core::EngineManager::hosted(std::move(gameManager));
  ```
- Optionally, replace a hand-rolled `GameRouter` facade mechanic with
  `cengine::routing::FlowRouter<YourFlow>` and keep only the vocabulary
  methods.

## [0.5.0] - 2026-07-10

Frame-end hook on the window (task 16 of the
[improvement plan](.ai/task/README.md)): GPU platforms need work AFTER the
game's `render()` — close the command buffer, submit, present. The
engine-owned frame gains a symmetric begin/end around the game phases,
enabling phase 2 of the 8Puzzle The-Forge PoC (cengine owns the loop,
The-Forge as a library).

> **Breaking release** (small). `IWindowManager` gains a pure virtual
> `present()` — every window manager must implement it (an empty body is
> fine for platforms with no present concept). Hosted mode (`frame(dt)`)
> is untouched.

### Added

- **`IWindowManager::present()`**: end of the frame — close and present what
  the game phases drew (GPU: endCmd, submit, queuePresent; terminal: print
  the screen — or empty). Called by `run()` every iteration after
  `render()`/`onExit()`, including the last frame (what was drawn when the
  game requested exit is presented before `cleanup()`). The engine-owned
  frame is now:

  ```
  window.update()   // OS events, frame setup (acquire, begin cmd...)
  game phases       // onEnter -> input -> update(fixedDt) 0..N -> render -> onExit
  window.present()  // close & present the frame
  ```

- **Documented ownership contract** on `present()`: it runs after the scene
  switch is committed (`onExit()` may destroy the scene that rendered), so
  every resource referenced by the recorded frame (fonts, buffers,
  swapchain...) must belong to the platform — scenes are pure logic and draw
  through the platform bridge, never owning GPU resources. Same ordering the
  hosted mode already validates (the host presents after `frame()` returns).

### Migrating from 0.4.0

Add `present()` to your `IWindowManager` implementations. Platforms with no
frame-end work (plain terminal, or when presentation already happens in the
scenes' draw) implement it empty:

```cpp
void present() override {}
```

## [0.4.0] - 2026-07-09

Hosted loop mode (task 15 of the [improvement plan](.ai/task/README.md)):
frameworks with inversion of control (The-Forge's `IApp`, editors, browsers)
own the loop and cannot call the blocking `start()` — they now drive the
engine with `frame(dt)`, one call per frame, keeping every fixed-timestep
guarantee from 0.3.0. Design validated against a real host (phase 1 of the
8Puzzle The-Forge PoC).

**Non-breaking release** — purely additive; no consumer changes required.

### Added

- **`EngineManager::frame(Seconds frameTime)`**: runs ONE complete frame
  (`onEnter` → `input` → `update(fixedDt)` 0..N times → `render` → `onExit`)
  consuming the host-measured `frameTime` in the internal fixed-timestep
  accumulator (now a member — the remainder persists across calls). Returns
  `false` when the game routed to the exit state; shutdown is the host's
  decision, and the host calls `cleanup()` on its teardown.
- **Hosted-mode tests** (6): phase order, short frame (0 updates, render still
  runs), accumulator persisting across `frame()` calls, `maxFrameTime` clamp,
  exit condition, `cleanup()` without a window manager.
- **README section** on hosted mode with the per-frame callback recipe.

### Changed

- **`run()` is now a consumer of `frame()`** — both modes share the exact same
  frame logic; the existing call-log suite passes unchanged.
- **`windowManager == nullptr` is supported** (hosted mode has no engine
  window: the host owns window, message pump and pacing). `run()` gained the
  missing null guard; `frame()` never touches the window by design.
- `maxFrameTime` clamping composes harmlessly with a host-side `dt` clamp
  (documented); the injectable clock is not consulted in hosted mode.

## [0.3.0] - 2026-07-08

Time in the loop (task 14 of the [improvement plan](.ai/task/README.md)): the
game loop gains a time concept — `update(dt)` with a **fixed timestep** (the
"fix your timestep" pattern). This closes the last structural gap in the core
flagged by ADR 0001 and provides the contract a future `cengine::physics`
module requires.

> **Breaking release.** `IScene` and `IGameManager` gain a pure virtual
> `update(Seconds dt)` — every consumer must implement it (an empty body is
> fine). See _Migrating from 0.2.0_ below.

### Added

- **`cengine/core/Time.hpp`** with
  `using Seconds = std::chrono::duration<double>;` — `std::chrono` end to end
  in the public API (no raw `double` with ambiguous units).
- **`IScene::update(Seconds dt)` / `IGameManager::update(Seconds dt)`**: the
  simulation phase. Fixed-timestep contract: called **0..N times per frame,
  always with the same `dt`** — scenes must not assume one call per frame nor
  measure time themselves.
- **Fixed-timestep loop** in `EngineManager::run()`: frame time is measured
  with the monotonic `steady_clock` and consumed by an accumulator in constant
  `fixedDt` steps; the remainder carries over to the next frame. Frame time is
  clamped to `maxFrameTime` (anti death-spiral).
- **Configurable timing** in the `EngineManager` constructor: `fixedDt`
  (default 1/60 s) and `maxFrameTime` (default 250 ms); non-positive values
  throw `std::invalid_argument` at construction.
- **Injectable time source** (`clockNow`, default `steady_clock::now`) — loop
  tests advance a fake clock manually; nothing sleeps or depends on real time.

### Changed

- **Frame order** is now `window.update → onEnter → input → update(dt)* →
  render → onExit` (README section _The frame and time_ documents the
  contract).

### Migrating from 0.2.0

- Add `void update(cengine::core::Seconds dt) override {}` to every `IScene`
  implementation (and to direct `IGameManager` implementations, if any) —
  mechanical change; leave the body empty if the scene has no simulation.
- Move any time-dependent logic (animations, timers, stopwatches) into
  `update(dt)` instead of measuring time inside the scene.
- Optionally tune the loop:
  `EngineManager{window, game, Seconds{1.0 / 120.0}}` for a 120 Hz simulation.

## [0.2.0] - 2026-07-08

API design cycle (tasks 12–13 of the [improvement plan](.ai/task/README.md)):
leaner ports and single-owner responsibilities in the routing module.

> **Breaking release.** The `IScene`, `IState` and `ISceneRepository` contracts
> changed, and so did the `RouterInMemory` wiring. See _Migrating from 0.1.0_
> below.

### Changed

- **`IScene` slimmed to 4 methods** (`onEnter` / `input` / `draw` / `onExit`):
  the activation bookkeeping pair `onEnterExecuted()` / `isOnEnterExecuted()`
  was removed from the port. The "runs exactly once per activation" guarantee
  is now enforced by `cengine::routing::GameManager` (tracked by state code,
  reset when a navigation is committed) — scenes no longer carry a flag.
- **`RouterInMemory` owns the state machine**: the current/next state pair
  moved from the repository into the router, and `hasPendingStateChange()` now
  means "a next state is scheduled" (null check) instead of a code comparison.
- **`RouterInMemory` takes exclusive ownership of the repository**: the
  constructor now receives `unique_ptr<ISceneRepository>` plus the initial
  state. No external reference can unload scenes behind the navigation cycle
  (closes the active-scene-eviction hole found in review).
- **Requesting the current state's code is now a valid transition** — a
  deliberate reload: the scene is unloaded, recreated and re-entered.
  Previously this was indistinguishable from "no pending change".
- **`ISceneRepository` is a pure scene provider**: `registerFactory` /
  `getScene` / `unloadScene` / `unloadAll`. The state accessors
  (`getCurrentStateGame`, `getNextStateGame`, `persisteCurrentState`,
  `persistNextState`, `hasPendingStateChange`) were removed with the
  responsibility.

### Removed

- **`IState::clone()`** — states are moved (`unique_ptr`) into the router; the
  Prototype pattern is no longer required from consumers.

### Added

- **Integration regression tests**: navigating A→B→A re-activates the returning
  scene, and requesting the current state reloads + re-activates it (both meant
  to run under the ASan CI job).

### Migrating from 0.1.0

- Remove `onEnterExecuted()` / `isOnEnterExecuted()` (and the backing flag)
  from your `IScene` implementations.
- Remove `clone()` from your `IState` implementations.
- Update the assembly: register factories on the repository, then **move** it
  into the router along with the initial state:
  ```cpp
  auto repository = std::make_unique<routing::SceneRepository>();
  repository->registerFactory("main_menu", [] { return std::make_unique<MenuScene>(); });

  auto router = std::make_shared<routing::RouterInMemory>(
      std::move(repository), std::make_unique<MyState>("main_menu"));
  auto gameManager = std::make_unique<routing::GameManager>(router);
  ```
- If your game kept a reference to the repository for domain navigation (e.g.,
  a facade), wrap the `IRouter` instead — the repository is engine-internal
  after the move.

## [0.1.0] - 2026-07-06

First release after the architecture overhaul guided by the improvement plan in
[`.ai/`](.ai/) (see [ADR 0001](.ai/decisions/0001-modular-core-vs-modules.md)).
The engine is now a **graphics-agnostic game loop + ports**, with scene/state
routing as an **opt-in module**.

> **Breaking release.** The library layout, target names, namespaces, include
> paths and parts of the public API changed. Consumers of `0.0.1` must update —
> see _Migrating from 0.0.1_ below.

### Added

- **Modular multi-target layout**: `cengine::core` (game loop + ports) and the
  optional `cengine::routing` (scene/state routing) module. Consumers link only
  what they need.
- **CMake options** to gate the build: `CENGINE_BUILD_ROUTING` (default `ON`) and
  `CENGINE_BUILD_TESTS` (default `ON`). The core builds standalone without routing.
- **C++23 requirement propagated** via `target_compile_features(... PUBLIC
  cxx_std_23)` — anything linking the targets inherits the standard.
- **Portable CMake presets** (`debug`, `release`, `asan`) in `CMakePresets.json`;
  machine-specific presets now live in a git-ignored `CMakeUserPresets.json`.
- **CI sanitizer job** (Linux, AddressSanitizer + UndefinedBehaviorSanitizer).
- **`StateCodes.hpp`** with `cengine::routing::kExitStateCode`, replacing the
  `"exit"` magic string.
- **Scene-lifetime regression test** (`SceneLifetimeTest`) exercising
  navigate → unload against the real implementations, meant to run under ASan.
- **Documentation**: Doxygen comments on all public headers; `Building` and
  `Usage` sections in the README (FetchContent consumption + minimal assembly).
- **`.ai/` working context**: improvement plan, ADR 0001, and verified
  build & test notes.

### Changed

- **Namespaces**: public types now live under `cengine::core` and
  `cengine::routing`.
- **Include paths**: headers are included as `<cengine/core/...>` and
  `<cengine/routing/...>`.
- **`IRouter` redesigned** to a leaner two-phase navigation contract
  (`requestState` / `hasPendingStateChange` / `commitStateChange` /
  `currentState` / `currentScene`).
- **`IState`** now belongs to the `cengine::routing` module (it is a routing
  concept, not a core one).

### Removed

- **`EngineManager::input()`** removed from the public API (dead code; the loop
  drives input through the game manager).

### Fixed

- **Scene-lifetime dangling window** in `GameManager::onExit()`: the scene
  reference could outlive the `commitStateChange()` that unloads it. The
  reference is now scoped so it cannot survive the unload, the lifetime contract
  is documented on `IRouter::currentScene()` / `ISceneRepository::getScene()`,
  and a regression test guards it.
- **Inverted names**: `IGameManager::shouldExist()` → `shouldExit()`, plus the
  correspondingly misleading test names.
- **Logging removed from the library** (no more `std::cout` in `GameManager`).
- **Dead code and stray includes** removed across core and routing.
- **CMake preset schema** lowered from `version: 8` (needs CMake ≥ 3.28) to
  `version: 4`, matching the declared `cmake_minimum_required(VERSION 3.23)`.

### Migrating from 0.0.1

- Replace the single `cengine_lib` link with the modular targets:
  ```cmake
  target_link_libraries(my_game PRIVATE
      cengine::core        # game loop + ports (always)
      cengine::routing     # scene/state routing (optional)
  )
  ```
- When pulling CEngine via `FetchContent`, disable its test suite before
  `FetchContent_MakeAvailable(cengine)` to avoid fetching GoogleTest at configure
  time: `set(CENGINE_BUILD_TESTS OFF)`.
- Update includes to the `<cengine/core/...>` / `<cengine/routing/...>` layout
  and qualify types with the `cengine::core` / `cengine::routing` namespaces.
- Rename `shouldExist()` to `shouldExit()` in your `IGameManager` implementations.
- Replace the `"exit"` state string with `cengine::routing::kExitStateCode`.

## [0.0.1] - Initial

- Initial single-target (`cengine_lib`) engine: game loop, scene/state
  management, and in-memory router.

[0.10.0]: https://github.com/cengine-dev/cengine/compare/0.9.0...0.10.0
[0.9.0]: https://github.com/cengine-dev/cengine/compare/0.8.0...0.9.0
[0.8.0]: https://github.com/cengine-dev/cengine/compare/0.7.1...0.8.0
[0.7.1]: https://github.com/cengine-dev/cengine/compare/0.7.0...0.7.1
[0.7.0]: https://github.com/cengine-dev/cengine/compare/0.6.0...0.7.0
[0.6.0]: https://github.com/cengine-dev/cengine/compare/0.5.0...0.6.0
[0.5.0]: https://github.com/cengine-dev/cengine/compare/0.4.0...0.5.0
[0.4.0]: https://github.com/cengine-dev/cengine/compare/0.3.0...0.4.0
[0.3.0]: https://github.com/cengine-dev/cengine/compare/0.2.0...0.3.0
[0.2.0]: https://github.com/cengine-dev/cengine/compare/0.1.0...0.2.0
[0.1.0]: https://github.com/cengine-dev/cengine/compare/0.0.1...0.1.0
[0.0.1]: https://github.com/cengine-dev/cengine/releases/tag/0.0.1
