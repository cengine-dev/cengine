# Plano de Melhoria — CEngine

Este diretório contém o plano de melhoria da engine, derivado de uma avaliação
técnica (arquitetura + boas práticas). Cada arquivo é uma tarefa independente,
numerada na **ordem de desenvolvimento recomendada**.

## Princípio da ordenação

A mudança de maior impacto é **tirar o roteamento do core** e reorganizar em
módulos (`cengine::core` + `cengine::routing`, ver [ADR 0001](../decisions/0001-modular-core-vs-modules.md)).
Como é grande e *breaking*, ela é feita cedo — porém **precedida de uma rede de
segurança barata** e **separando "mover" de "redesenhar"**.

**Ordem de execução recomendada** (não é estritamente a ordem numérica):

1. **Rede de segurança (01):** adicionar `override` — trivial e endurece o
   compilador **antes** da grande movimentação de código.
2. **Reorganização estrutural (05a):** mover para `core/` + `modules/routing/`,
   CMake multi-target, namespaces por camada. **Move puro, sem mudar
   comportamento** (absorve a antiga tarefa 03 de namespace).
3. **Redesenho do roteador (05b):** enxugar o `IRouter`, unificar Scene/Screen,
   desacoplar — já isolado dentro de `cengine::routing`.
4. **Correções de qualidade (02, 04, 06):** nomes invertidos, `const`/logging e
   ciclo de vida das cenas — agora no lugar final.
5. **Ecossistema/build (07–08):** padrão C++, presets, CI.
6. **Limpeza final e documentação (09–11).**
7. **Ciclo 0.2.0 — design de API (12 → 13):** primeiro tirar a contabilidade
   de `onEnter` da `IScene` (12, cirurgia pequena), depois separar Router
   (máquina de estados) de Repository (provedor de cenas) (13). As duas são
   *breaking* — agrupar no bump 0.2.0. ✅ (release 0.2.0 publicado)
8. **Ciclo 0.3.0 — tempo no loop (14):** `update(dt)` com fixed timestep no
   `EngineManager` — a última lacuna estrutural do core e o pré-requisito do
   futuro `cengine::physics` (ADR 0001). *Breaking* nas portas
   `IGameManager`/`IScene` — âncora do bump 0.3.0. ✅ (release 0.3.0 publicado)
9. **Ciclo 0.4.0 — modo hospedado (15):** `frame(dt)` para hosts com inversão
   de controle (The-Forge). Desenho validado pela fase 1 da PoC The-Forge no
   8Puzzle; adaptador `8PuzzleForge` migrado. ✅ (release 0.4.0 publicado)
10. **Ciclo 0.5.0 — fim do quadro na janela (16):** `IWindowManager::present()`
    para o modo próprio fechar/apresentar o quadro depois do `render()` —
    pré-requisito da fase 2 da PoC The-Forge (task 02 do 8Puzzle, modo
    biblioteca). *Breaking* pequeno na interface — âncora do bump 0.5.0.
    ✅ (release 0.5.0 publicado; validado pelo degrau 2 da fase 2 —
    `TheForgeWindowManager` com o quadro de GPU no par `update()`/`present()`)
11. **Ciclo 0.6.0 — janela obrigatória (21) + FlowRouter de carona (19):**
    remover a hipótese do `nullptr` no `EngineManager` (*breaking*, âncora
    do bump — saiu pela **opção B**: factories `owned()`/`hosted()`) e
    extrair a mecânica da fachada de navegação (`FlowRouter<TFlow>`,
    opt-in) na mesma visita. O aceite mudou em relação ao proposto:
    **8puzzle e spaceinvaders foram estacionados na 0.5.0 como documentação
    viva (ADR 0003)** — quem valida o novo desenho é a suíte da cengine e o
    asteroids, que nasce como terceiro consumidor. A task 20 segue
    **estacionada** (gate não disparou — ver ADR 0002).
    ✅ (release 0.6.0 publicado; suíte 49/49 verde)

> Regra prática: manter a suíte de testes **verde a cada tarefa**. Nenhuma
> tarefa deve ser mergeada com testes quebrados.
>
> **Nota sobre a tarefa 03:** o namespace deixou de ser tarefa isolada — ele
> nasce junto da reorganização em módulos (05a), como `cengine::core` /
> `cengine::routing`. O arquivo 03 permanece como referência técnica.

## Decisões de arquitetura (ADRs)

O plano é guiado por decisões registradas em [`../decisions/`](../decisions/).
Ler antes de executar as tarefas de arquitetura:

- [ADR 0001 — Core mínimo (loop) + módulos opcionais](../decisions/0001-modular-core-vs-modules.md):
  o core é só o game loop + portas; roteamento e física são módulos opt-in no
  mesmo repo. Reformula a tarefa 05 (dividida em 05a/05b) e absorve a 03.
- [ADR 0002 — Critério de promoção (filtro anti-depósito)](../decisions/0002-criterio-de-promocao-anti-deposito.md):
  código só entra na engine se for mecanismo puro (sem vocabulário de jogo),
  com ≥ 2 **evidências reais** de necessidade e testável na própria cengine.
  Governa as tarefas 17–21 e toda promoção futura.
  **Emenda 1 (2026-07-14):** um jogo estacionado continua valendo como
  evidência — congelar suspende a manutenção, não o aprendizado. O pedágio é
  que a suíte da engine precisa **encarnar o caso de uso do jogo congelado**
  (foi o que destravou a tarefa 17).
- [ADR 0003 — Consumidores estacionados como documentação viva](../decisions/0003-consumidores-estacionados-documentacao-viva.md):
  8puzzle e spaceinvaders ficam pinados na cengine 0.5.0 e saem dos
  critérios de aceite; o asteroids é o consumidor de validação do 0.6.0 em
  diante.

## Índice

| # | Tarefa | Prioridade | Categoria |
|---|--------|------------|-----------|
| 01 | [Adicionar `override` em todas as sobrescritas](01-add-override.md) | 🔴 Alta | Boas práticas |
| 02 | [Corrigir nomes invertidos (`shouldExit`, `isNextState…`)](02-fix-inverted-names.md) | 🔴 Alta | Boas práticas |
| 03 | [Introduzir namespaces](03-namespace.md) *(absorvida pela 05a)* | 🟡 Média | Arquitetura |
| 04 | [`const`-correctness + remover logging da lib](04-const-and-logging.md) | 🟡 Média | Boas práticas |
| 05a | [Reorganização estrutural: `core` + `routing`](05a-restructure-modules.md) | 🔴 Alta (arq.) | Arquitetura |
| 05b | [Redesenhar o `IRouter`](05b-redesign-irouter.md) | 🔴 Alta (arq.) | Arquitetura |
| 06 | [Corrigir ciclo de vida / referências de cena](06-scene-lifetime.md) | 🟡 Média | Arquitetura |
| 07 | [Alinhar padrão C++ entre cengine e consumidores](07-cpp-standard.md) | 🟡 Média | Ecossistema |
| 08 | [Presets portáveis + CI (Debug + sanitizers)](08-presets-and-ci.md) | 🟢 Baixa | Ecossistema |
| 09 | [Higiene de código (includes, código morto)](09-code-hygiene.md) | 🟢 Baixa | Boas práticas |
| 10 | [Eliminar magic string e estados *stringly-typed*](10-typed-states.md) | 🟢 Baixa | Boas práticas |
| 11 | [Documentação (Doxygen + uso no README)](11-documentation.md) | 🟢 Baixa | Documentação |
| 12 | [Tirar a contabilidade de ativação (`onEnter`) da `IScene`](12-scene-activation-bookkeeping.md) | 🟡 Média | Arquitetura |
| 13 | [Separar responsabilidades: Router × Repository](13-router-repository-responsibilities.md) | 🔴 Alta (arq.) | Arquitetura |
| 14 | [Tempo no loop: `update(dt)` separado de `render()`](14-time-in-the-loop.md) | 🔴 Alta (arq.) | Arquitetura |
| 15 | [Modo hospedado: dirigir o loop de fora (`frame(dt)`)](15-hosted-loop-mode.md) | 🟡 Média | Arquitetura |
| 16 | [Fim do quadro na janela: `IWindowManager::present()`](16-window-present-hook.md) | 🟡 Média | Arquitetura |
| 17 | [Colisão 2D: detecção opt-in (AABB + círculo)](17-collision2d-detection.md) ✅ 0.7.0 | 🟡 Média | Arquitetura |
| 18 | [Scene stack e overlays](18-scene-stack-overlays.md) ✅ 0.11.0 | 🟡 Média (extraída do Delve, validada pelo Bulwark) | Arquitetura |
| 19 | [FlowRouter: extrair a mecânica da fachada de navegação](19-flow-router-facade.md) ✅ 0.6.0 | 🟢 Baixa (carona) | Arquitetura |
| 20 | [Vocabulário de input como porta](20-input-vocabulary-port.md) ✅ 0.8.0 | 🟡 Média | Arquitetura |
| 21 | [`IWindowManager` obrigatório: remover a hipótese do `nullptr`](21-window-manager-mandatory.md) ✅ 0.6.0 | 🟡 Média (breaking, 0.6.0) | Arquitetura |
| 22 | [Colisão 2D: resolução (recorte a decidir)](22-collision2d-resolution.md) | 🟢 Baixa/Média (2/2 para eixo-separado: mario + zelda; 0 consumidores de penetração/MTV — comparar antes de promover) | Arquitetura |
| 23 | [Câmera / viewport (mundo→tela + culling)](23-camera-viewport.md) | ✅ done (0.10.0 — `cengine::camera2d`: transformada+culling; seguimento ficou nos jogos; zelda valida, mario pinado 0.9.0) | Arquitetura |
| 24 | [Áudio como porta (`play(id)`), backend na plataforma](24-audio-port.md) ✅ 0.9.0 | 🟡 Média (gate disparou com 2/2: breakout + mario@0fab493; mario valida a 0.9.0) | Arquitetura |
| 25 | [Clip de animação de sprite (frames sobre tempo)](25-sprite-animation-clip.md) | ✅ done (0.10.0 — `cengine::anim`: máquina clip+frame+acumulador; seleção/vocabulário ficam nos jogos; zelda valida, mario pinado 0.9.0; spaceinvaders segue sem linkar — opt-in) | Arquitetura |

## Candidatas e estado dos gates

> O retrato do ecossistema INTEIRO (engine + casco + os 8 jogos, quem pariu
> cada módulo, o que está aberto e o que foi vetado) vive em
> [`.ai/ecossistema.md`](../ecossistema.md) — **atualizado junto com o sweep
> de cada jogo**, na mesma revisão de candidatas que alimenta esta seção.

Tasks que registram candidatas a crescer a engine para o aprendizado não se
perder entre os projetos. Um gate disparado autoriza desenhar a extração; não
autoriza promover política nem implementar uma API diferente da evidência real.
Ver [ADR 0002](../decisions/0002-criterio-de-promocao-anti-deposito.md).

- **18 (scene stack/overlays)** — **PROMOVIDA (0.11.0, 2026-07-28)**:
  `cengine::routing::SceneStack`, EXTRAÍDA do `delve::LayerStack` com o
  Bulwark como consumidor de validação. Duas das três políticas do desenho
  original não subiram (zero evidência em dois consumidores) e uma operação
  que faltava subiu (`replaceBottom`). **Limite registrado:** a regra "só a
  primeira camada ativa do topo recebe input" é correta para overlay MODAL e
  insuficiente para overlay clicável NÃO-modal — com ponteiro, "quem recebe o
  clique" é espacial, e `IScene::input()` não reporta consumo. Não corrigido
  por ter UM consumidor (critério 2), não por custo de migração — o ADR 0003
  pina os jogos, então um breaking não obrigaria ninguém a migrar.
  **Revisão do Bulwark (2026-07-28): segue com UM consumidor.** É a dívida
  declarada do 10º jogo, condicionada: se ele tiver ponteiro, extrai o mouse
  (27) e corrige o roteamento espacial; se não tiver, a candidata espera.
- **22 (resolução de colisão)** — **2/2 para o padrão eixo-separado**: mario e
  zelda movem/resolvem X e depois Y. Isso dispara a comparação, mas não a API
  de penetração/MTV originalmente imaginada, que segue com 0 consumidores. A
  task permanece estacionada até identificar um núcleo puro que ambos usariam;
  reflexão do breakout, `grounded`, one-way e dano continuam política.
- **23 (câmera/viewport)** — **GATE DISPARADO (2/2)**: mario e zelda usam a
  mesma transformada mundo→janela e o mesmo culling; o Zelda adiciona rolagem
  vertical sem mudar o mecanismo. A candidata está pronta para desenho da
  extração. O SEGUIMENTO (âncora/limites/eixos) é feel e fica nos jogos.
- **24 (áudio como porta)** — **PROMOVIDA (0.9.0, 2026-07-16)**: o gate
  disparou com 2/2 (breakout + mario `0fab493`, cópias quase idênticas — o
  discriminador do input) e a porta subiu: `cengine::audio::Player` com
  `play(id)` e mais nada, backend segue nas plataformas. É a prova de que uma
  estacionada não é uma recusa: é uma espera com critério.
- **26 (grade em pixels: célula ↔ pixel)** — **1/2, levantada na revisão do
  Bulwark (2026-07-28)**. Delve (`@f9cd31b`, `ForgeWorldLayer.cpp:61`) e
  Bulwark (`ForgeWorldLayer.cpp:38`) escrevem a MESMA conta de grade centrada
  (`gridW = cols*cell`, `origin = (screen - grid) * 0.5`). Mas pelo precedente
  da **camera2d** — origem é política do jogo, só a projeção sobe — a metade
  de IDA que restaria é `origin + col*cell`: uma multiplicação e uma soma, fina
  demais para virar módulo. A metade que PAGA é a VOLTA (`cellAt`: pixel →
  célula com rejeição de borda, incluindo o teste de negativo ANTES do cast
  para `uint32_t`, que é UB) — e essa tem **um** consumidor, porque só o
  Bulwark tem ponteiro. Mesma candidata que o `screen → world` do degrau 05,
  vista de outro ângulo. **Vizinhança, não sobreposição:** a `camera2d` declara
  no cabeçalho que "escala, letterbox e centralização em PIXELS ficam nas
  cenas" — este candidato mora no espaço que ela recusou de propósito.
- **27 (mouse como porta de input)** — **1/2**. Vive no `forgeui` do common
  (0.6.0) com vocabulário local, como o teclado viveu antes da task 20. O
  discriminador é o histórico: o enum `Key` só subiu na **4ª cópia idêntica**;
  o mouse tem uma. Fecha junto com a 26 e com o limite espacial da 18 — as
  três são a mesma aposta, e todas dependem do 10º jogo ter ponteiro.
- **`Path`/waypoint** — **1/2 e sem task aberta**: busca no ecossistema inteiro
  não achou outro jogo com movimento por waypoint. Registrado para não se
  perder; não vira task até existir o segundo.
- **Pool de efeitos transitórios** — **candidata MORTA na revisão do Bulwark,
  com argumento.** Star Force e Bulwark mantêm os dois uma lista de efeitos que
  nasce de evento, envelhece e some — parece 2/2. Não é: **a duplicação real já
  subiu na 0.12.0** (o embrulho que sabia quando a animação acabou virou
  `ClipDesc::loop` + `Animator::finished()`). O que sobrou dos dois lados são
  ~6 linhas de `std::vector`, e nem iguais (swap-and-pop vs `remove_if`), com
  payloads diferentes (`Vec2` vs distância escalar). **Lição para as próximas
  revisões: uma promoção bem feita esvazia a próxima candidata — o que sobra
  parece candidato pela silhueta e já é só idioma. Se não dá para descrever a
  candidata sem citar `std::vector`, é biblioteca padrão, não engine.**

- **25 (clip de animação de sprite)** — 1/2: o mario trouxe o `PlayerAnimator`
  (ciclo de frames dirigido pelo TEMPO: clip = frames + fps + loop). O
  spaceinvaders anima SEM relógio (a pose deriva do passo da marcha,
  `animFrame()`) — forma diferente, o mesmo sinal de divergência da 22. Espera
  um 2º jogo com ciclo de frames dirigido pelo tempo; a ESCOLHA do clip
  (Idle/Walk/Jump, facing) é política e fica no jogo.

## Consideradas e vetadas (política de jogo — NÃO reabrir sem argumento novo)

Padrões que se REPETEM entre os jogos e, ainda assim, **não sobem** — o veredito
já foi dado (ADR 0002: a duplicação é o custo aceito quando a forma se repete mas
o SIGNIFICADO é do jogo). Registrado aqui para ninguém reabrir "e isto, não é
candidato?" daqui a alguns jogos. O discriminador: o input subiu porque as 4
cópias eram o MESMO dado puro (o enum `Key`); os itens abaixo têm cópias
estruturalmente parecidas mas **semanticamente diferentes**. Semelhança de forma
≠ identidade de mecanismo.

- **Recordes** (`Record` + `RecordService` + `RecordRepository` + `FileRecordRepository`)
  — em 4 jogos (8puzzle, spaceinvaders, asteroids, breakout), a MAIOR duplicação
  do ecossistema. Vetado explicitamente (asteroids task 05, breakout task 07):
  o que é um recorde, quantos guardar, a ordem (movimentos↓ do 8puzzle vs score↑
  dos arcades) e onde persistir são decisões DO JOGO. Confirmado por diff: as
  portas DIFEREM entre jogos — não é o mesmo mecanismo.
- **`PlaySession`** (carregador do resultado da última partida entre cenas) — em
  3 jogos, ~10 linhas, quase idêntico (score+wave/level). É um struct de valor,
  não um mecanismo: promover seria "um `shared_ptr` que o composition root já
  segura". O conteúdo (o que é um "resultado") é política.
- **Wrap-around / toro** (arena que dá a volta) — ficou no asteroids; fora de
  escopo declarado na task 17. Formato do mundo é política; sem 2º consumidor (o
  mario não dá a volta). Promover daria à engine uma opinião sobre o formato do
  mundo.
- **Física de plataforma** (gravidade/terminal, impulso de pulo, integração,
  resolução sobre grade de tiles — o `World` do mario) — avaliada ao FECHAR o
  mario (2026-07-16), a pedido do dono. Vetada em três camadas: as CONSTANTES
  são feel (kJumpSpeed=340 existe para o pulo dar ~4 tiles "a cara do
  original", e foi retunada em playtest); a INTEGRAÇÃO é trivial e cada jogo
  integra diferente (inércia do asteroids, reflexão do breakout, degraus do
  spaceinvaders, eixo-separado do mario); a RESOLUÇÃO já é a task 22, e a
  evidência inicialmente apontava contra (breakout e mario resolvem
  DIFERENTE). O Zelda depois forneceu a segunda evidência do recorte
  eixo-separado, registrada na task 22, mas não tornou gravidade, pulo,
  `grounded`, tiles one-way ou dano mecanismos genéricos. "Física na engine"
  continuaria congelando política e formato de mundo; apenas o núcleo comum de
  resolução pode ser reavaliado.
- **`Events` por quadro** (o struct de contadores que o World zera e preenche a
  cada `update` — `brk::Events` no breakout, `mario::Events` no mario) — o
  PADRÃO se repete (fatos, não sons; a cena decide o significado) e deve
  continuar se repetindo nos próximos jogos, mas os CAMPOS são o vocabulário de
  cada jogo (tijolos/vidas/fase vs pulo/moeda/pisão/bandeira). Mesmo caso do
  `PlaySession`: struct de valor, não mecanismo. O padrão é disciplina de
  projeto, documentada nas tasks dos jogos; não vira tipo da engine.

  **Emenda (revisão do Bulwark, 2026-07-28) — a disciplina ganhou uma REGRA:
  quem para de atualizar também tem de parar de reportar.** Dois jogos pagaram
  por ela, por caminhos diferentes: no Delve os eventos ficavam pendurados
  entre turnos (resolvido com o `turnCount` do `Dungeon`); no Bulwark
  `Match::update` parava o campo ao fim da partida e, com isso, parava de
  LIMPAR os eventos dele — o abate do último quadro congelava preenchido e o
  som tocava para sempre (`m_field.clearEvents()` na saída). Um problema só:
  **o tempo de vida do evento está amarrado ao do `update`, e todo lugar onde o
  `update` para é um lugar onde o evento mente.** Continua não virando tipo da
  engine — os campos seguem sendo vocabulário do jogo. Vira teste-padrão que
  todo jogo com eventos deveria ter e nenhum tinha: *depois do fim, os eventos
  estão vazios* (`AFinishedMatchStopsReportingFieldEvents`, no Bulwark).
  Detalhe que explica por que nenhuma suíte pegou antes: testes de fim de
  partida leem ESTADO (vida, entidades paradas), e estado parado está certo —
  **o bug só existe quando aparece o primeiro leitor de EVENTO.**
- **Formatação de tempo** — o common já tem `formatMillis` (hh:mm:ss.mmm, do
  8puzzle) e o mario criou `ui::formatTime` (M:SS.cc) SEM reusar: os formatos
  divergem de propósito (cronômetro de puzzle vs HUD de arcade). Formato de
  exibição é feel; duas cópias com saídas diferentes não são o mesmo mecanismo.

Sweep de 2026-07-15 (ao fechar o degrau 2 do mario): nenhuma candidata nova além
das estacionadas acima; nenhum math/Vec2/RNG/timer próprio duplicado (os jogos
usam `collision2d::Vec2`).

Sweep de 2026-07-16 (ao fechar o mario completo — degraus 1-5, com goombas,
recordes por pontos/tempo e bandeira): nenhuma candidata nova; física de
plataforma avaliada e vetada (acima); tasks 22/23 seguem estacionadas com os
gates inalterados (o pisão no goomba REFORÇA a leitura da 22: mais um contato
resolvido com regra própria do jogo). A 24 promoveu logo em seguida (0.9.0).

Sweep de 2026-07-17 (revisão pós-0.9.0, antes do 6º jogo): uma candidata nova
na ENGINE — o clip de animação de sprite (task 25, estacionada 1/2, mario
`PlayerAnimator`). Duas candidatas novas no nível da PLATAFORMA, registradas no
backlog do platform-theforge-common (lá é a casa delas, não aqui): o backend
XAudio2 da porta de áudio (2 cópias idênticas: breakout + mario) e o escritor
de DDS dos geradores de atlas (3 cópias idênticas: spaceinvaders + breakout +
mario). Vetados novos abaixo: `Events` por quadro e formatação de tempo.

Sweep de 2026-07-18 (Zelda tasks 02–03 concluídas): a task 23 atingiu 2/2 e
está liberada para desenhar a extração de transformada+culling. A task 22
também chegou a 2/2 para resolução eixo-separada, mas a evidência não usa o MTV
proposto originalmente; antes de implementar, a candidata precisa ser
recortada pela comparação Mario×Zelda. Seguimento de câmera e regras de colisão
continuam nos jogos.

Sweep de 2026-07-22 (Star Force completo — sétimo jogo, um shmup de rolagem
vertical com massa de entidades e roteiro de conteúdo, os dois ângulos que a
task 06 pediu pra revisar ao fechar):

- **18 (scene stack/overlays)** — sem evidência nova: Star Force não tem
  pausa. Segue 1/2 (breakout).
- **22 (resolução de colisão)** — sem evidência nova: o combate é só
  DETECÇÃO (`collision2d::intersects`, bala mata no primeiro toque, sem
  empurrão/reflexo/grounded). Star Force não usa resolução eixo-separada
  nem MTV — não desloca nenhum dos dois lados do gate. Segue 2/2 pro padrão
  eixo-separado, 0/2 pro MTV, estacionada.
- **Broadphase de `collision2d` — pergunta fechada, resultado NEGATIVO.** A
  task 04 do Star Force (`.ai/task/04-roteiro-ondas-combate.md`) tinha um
  criterio pendente ("dezenas de entidades vivas nos picos sem soluço de
  frame — nada mediu ainda"). Medido agora pelos números do próprio jogo:
  11 torretas no total (`Area.cpp`), no máximo ~4 inimigos por entrada do
  roteiro com no máximo 2 entradas se sobrepondo em tela (`Wave.cpp`,
  gatilhos a cada ~90 unidades / 30 u/s de scroll ≈ 3s de intervalo), e no
  máximo ~5 balas da nave simultâneas em voo (cooldown 0.12s × 0.6s de
  travessia do campo a 300 u/s). `resolveCombat()` é um laço aninhado
  ingênuo bala×(inimigos+torretas) — no pico, algo como 5×19 ≈ 95 pares
  por quadro. Isso está muito longe de "dezenas" o bastante pra doer:
  nenhuma sessão de playtest (3 rodadas completas, tasks 05/06) relatou
  soluço. **Veredito: este jogo não gera evidência de necessidade de
  broadphase.** O naive O(n×m) do `collision2d::intersects` continua
  suficiente pro ecossistema inteiro até agora; a pergunta so volta se um
  jogo futuro tiver ordens de grandeza a mais de entidades simultâneas
  (bullet-hell de verdade, multidão de partículas etc.).
- **Roteiro dirigido por scroll/posição (`WaveScript`, task 04 do
  Star Force) — observação, SEM task própria ainda (0/2, primeira
  evidência).** O mecanismo — uma lista ordenada por limiar, `poll(progress)`
  devolve so as entradas cruzadas desde o último poll, sem redisparar — é
  genérico o bastante pra ser candidato um dia, mas a carga (`Formation`,
  `Edge`, contagem) é vocabulário do jogo, mesmo caso já visto em
  `Events`/`PlaySession`. O próprio código já registra isso (comentário em
  `Wave.h`: "se um segundo jogo dirigido por roteiro aparecer, o PADRÃO
  vira candidata"). Nenhum outro jogo do ecossistema tem esse formato
  (verificado por grep); fica registrado aqui só pra não se perder — não
  abrir task nova sem o segundo consumidor.
- **25 (clip de animação) — reforçada, sem promover nada de novo.** O
  `starforce::ExplosionAnimator` é o 3º consumidor real de `cengine::anim`
  (depois de mario/zelda), confirmando que SELEÇÃO/vocabulário ficam no
  jogo. Mas ele também é a PRIMEIRA evidência de uma forma diferente: o
  `Animator` da engine só sabe fazer loop (o `frame()` embrulha sozinho ao
  cruzar `frameCount`); a explosão precisa tocar as 3 células UMA VEZ e
  travar no último frame. O jogo resolveu por fora (wrapper com relógio
  próprio, `finished()` checado antes de `frame()` pra nunca expor o
  wrap interno) — decisão deliberada da task 05 de não mexer no
  `cengine::anim` por um único consumidor. Registrar como observação
  (1/2 nesta forma especifica de "play once"): se um segundo jogo
  precisar do mesmo "toca e trava", a comparação decide se um modo
  `loop=false` no `ClipDesc` vale a pena ou se o wrapper por fora
  continua sendo a resposta certa.
- **Áudio: nenhuma evidência nova pra porta** (`cengine::audio::Player`
  já promovida, 0.9.0) — Star Force é só mais um consumidor de
  `play(id)`. A receita de "cooldown próprio pro som do tiro pra não
  encher o pool de 8 vozes durante rajada contínua" é política de
  APRESENTAÇÃO do jogo (quando chamar `play`), não do backend
  (`forgeaudio`, que já faz round-robin das 8 vozes por design) — não
  meche no mecanismo, registrado só como nota pro próximo jogo com tiro
  automático, se algum precisar do mesmo ajuste.

Sweep de 2026-07-27 (Delve completo — oitavo jogo, o primeiro por TURNOS e o
primeiro escolhido POR uma task desta lista, a 18):

- **18 (scene stack/overlays) — as três condições do gate CUMPRIDAS, e mesmo
  assim não promove AINDA.** O Delve escreveu uma pilha de verdade
  (`delve::LayerStack`, cinco tipos de camada) e bateu as três: cena de baixo
  rodando atrás, 2+ camadas simultâneas, e overlay que sobrevive à troca da
  cena de baixo (descer um andar troca o mundo por baixo do HUD). Falta o
  **consumidor de validação** do critério 2 do ADR 0002 — mas ele vem por
  EXTRAÇÃO, não por uma segunda cópia manual: o próximo jogo que precisar de
  camadas extrai esta pilha e a exercita, como `forgeaudio` e `Write-Dds`
  fizeram. **O valor que ficou é outro
  e é maior: o desenho proposto na task foi desmontado por um consumidor
  real** — `updatesBelow` e `drawsBelow` têm ZERO evidência (todas as camadas
  sempre atualizam e desenham; congelar pela pilha teria destruído a evidência
  da 1ª condição), faltava `replaceBottom`, e a regra "só o topo recebe input"
  é insuficiente — o primeiro HUD empilhado deixou o jogo inteiro sem resposta,
  com a suíte verde. Detalhe na própria task 18, seção "Avaliação 2026-07-27".
- **22 (resolução de colisão) — não tocada.** O Delve não tem colisão nenhuma:
  movimento é índice de grade e "bater" é comparar coordenadas. Segue 0/2 pro
  MTV.
- **25 (clip de animação) — não avança.** O Delve é o 2º consumidor de
  `cengine::anim` mas usa clip em LOOP (idle de 2 quadros do inimigo). A forma
  "toca uma vez e trava" que o Star Force registrou continua 1/2.
- **Turno sobre o `update(dt)` de passo fixo — pergunta fechada, resultado
  NEGATIVO.** O plano do Delve mandava vigiar se um domínio que só muda de
  estado em bordas de input forçaria alguma mudança no loop. **Não forçou
  nada**: a `Dungeon` não vê `dt`, o `update(dt)` das cenas ficou servindo só
  à animação, e o loop hospedou o gênero sem uma linha alterada. Turno não
  pede nada do laço de tempo.
- **`cengine::audio` (0.9.0) e `cengine::anim` (0.10.0) — 3º e 2º consumidores,
  sem uma linha de mudança.** Confirmam as duas portas num gênero novo.

## Legenda de status

Marque no topo de cada arquivo conforme avança:
`todo` → `in-progress` → `done`
