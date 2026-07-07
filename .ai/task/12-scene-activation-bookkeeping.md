# 12 — Tirar a contabilidade de ativação (`onEnter`) da `IScene`

- **Status:** done ✅ (opção 1, 2026-07-07, branch `feature/12-scene-activation-bookkeeping`)
- **Prioridade:** 🟡 Média
- **Categoria:** Arquitetura / design de API
- **Depende de:** 05b (Router estável), 06 (lifetime das cenas definido)
- **Relacionada:** 13 (redesenho Router/Repository — executar 12 antes, é a
  cirurgia menor)
- **Breaking:** sim — coordenar com bump de versão (0.2.0) junto da tarefa 13.

## Problema

A porta `IScene` (`core/include/cengine/core/IScene.hpp`) tem 6 métodos, mas
apenas 4 pertencem à cena (`onEnter`, `input`, `draw`, `onExit`). O par

```cpp
virtual void onEnterExecuted() = 0;
[[nodiscard]] virtual bool isOnEnterExecuted() const = 0;
```

é **contabilidade da engine vazando para o implementador**: serve só para o
`GameManager::onEnter()` saber se `onEnter()` já rodou nesta ativação. Efeitos:

1. **Boilerplate obrigatório em todo jogo.** Cada cena precisa carregar um
   boolean e implementar os dois métodos corretamente (no 8Puzzle, toda a
   hierarquia `TerminalScene` paga esse custo).
2. **Erro silencioso.** Se o implementador esquecer de resetar/retornar o flag
   certo, `onEnter()` roda toda iteração (ou nunca) — sem erro de compilação.
3. **Acoplamento escondido com a política de unload.** O flag só "reseta"
   porque `commitStateChange()` **destrói** a cena que sai. Se um dia houver
   keep-alive de cenas (ver pendência da tarefa 13), `isOnEnterExecuted()`
   permanece `true` e `onEnter()` nunca mais roda ao voltar para a cena — bug
   garantido por construção.

Quem sabe "a cena atual já foi ativada neste ciclo?" é o **orquestrador**
(`GameManager`), não a cena. Tell, don't ask.

## Objetivo

`IScene` com 4 métodos coesos; a garantia "onEnter roda exatamente uma vez por
ativação" passa a ser responsabilidade da engine, invisível para o jogo.

## Opções (decidir na tarefa)

1. **Engine-side tracking no `GameManager` (recomendada).** O `GameManager`
   guarda o código do estado cuja cena já recebeu `onEnter`:

   ```cpp
   class GameManager : public core::IGameManager {
       std::shared_ptr<IRouter> m_routerService;
       std::string m_enteredStateCode; // vazio = nenhuma cena ativada
   public:
       void onEnter() override {
           const auto code = m_routerService->currentState().getCode();
           if (code != m_enteredStateCode) {
               m_routerService->currentScene().onEnter();
               m_enteredStateCode = code;
           }
       }
       void onExit() override {
           // ... fluxo atual ...
           m_routerService->commitStateChange();
           m_enteredStateCode.clear(); // próxima cena será ativada no próximo frame
       }
   };
   ```

   `IScene` perde os dois métodos; nenhum estado novo na engine além de uma
   string no `GameManager`. Funciona com qualquer política de unload futura,
   porque o reset acontece **no commit**, não na destruição da cena.

2. **Classe base `SceneBase` com o flag.** Mantém a interface como está e
   oferece uma base que implementa o par de métodos. Menos breaking, mas não
   resolve o problema — a interface continua com 6 métodos e o contrato
   continua violável por quem não herdar da base.

3. **Ativação no provisionamento (repository chama `onEnter` ao instanciar).**
   Elegante à primeira vista (cena "nasce ativada"), mas mistura provisionamento
   com timing de ciclo de vida: `onEnter` rodaria no meio de `commitStateChange`
   ou de um `getScene()` qualquer, fora da fase `onEnter` do loop. Descartada.

> Recomendação: opção 1. A opção 2 só se quisermos uma migração em duas etapas
> (base agora, remoção dos métodos depois) — para um projeto de estudo com um
> único consumidor conhecido, não compensa.

## Passos

1. Remover `onEnterExecuted()`/`isOnEnterExecuted()` de `IScene`.
2. Implementar o tracking no `GameManager` (opção 1), resetando no commit.
3. Atualizar `MockScene` e os testes de `GameManagerTest` que cobrem
   `OnEnter_WhenSceneNotInitialized_*` / `OnEnter_WhenSceneAlreadyInitialized_*`
   — a expectativa passa a ser sobre o número de chamadas a `scene.onEnter()`,
   não sobre o flag.
4. Adicionar teste novo: após uma navegação (commit), a cena do novo estado
   recebe `onEnter()` no frame seguinte (garante o reset).
5. Adicionar teste de regressão para o acoplamento escondido: navegar A→B→A
   deve reativar A (hoje passa por acidente via unload; o teste protege o
   comportamento quando a política de unload mudar).
6. Atualizar o Doxygen de `IScene` e `IGameManager` (o contrato "uma vez por
   ativação" passa a ser garantido pela engine).

## Critérios de aceite

- [x] `IScene` com exatamente 4 métodos: `onEnter`, `input`, `draw`, `onExit`.
- [x] Nenhuma menção a `onEnterExecuted` no repositório (código, mocks, docs).
- [x] Teste cobrindo "onEnter roda 1x por ativação" e "reativação após A→B→A".
- [x] Suíte verde.

## Resultado da execução

- `IScene` reduzida a 4 métodos; Doxygen atualizado (a garantia "1x por
  ativação" é do orquestrador).
- `GameManager` ganhou `m_enteredStateCode` (código do estado ativado),
  comparado em `onEnter()` e limpo após `commitStateChange()` em `onExit()`.
- `MockScene` e `TrackedScene` (SceneLifetimeTest) enxugados; `TrackedScene`
  agora conta ativações (`onEnterCount`).
- Testes novos: `OnEnter_SameStateOnNextIterations_DoesNotReenter`,
  `OnEnter_AfterCommittedNavigation_ReentersEvenWithSameStateCode` (unidade) e
  `SceneLifetimeTest.NavigatingBackReactivatesScene` (integração A→B→A).
- Suíte: **30/30 testes passaram** (`ctest --preset msys2-mingw`).

## Riscos

Baixo. A mudança é localizada (`IScene`, `GameManager`, mocks e testes). O
único consumidor conhecido (8Puzzle) precisa remover o boilerplate das cenas —
simplificação, não reescrita.

## Pendências fora do escopo

- Atualizar o 8Puzzle: remover flag/métodos das cenas de terminal
  (`TerminalScene` e derivadas) ao consumir a versão 0.2.0.
