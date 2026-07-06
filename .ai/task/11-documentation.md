# 11 — Documentação (Doxygen nos headers + uso no README)

- **Status:** done
- **Prioridade:** 🟢 Baixa
- **Categoria:** Documentação
- **Depende de:** todas as anteriores (documentar a API **já estabilizada**, para
  não escrever doc de código que vai mudar)

## Problema

A documentação atual cobre "como buildar/testar", mas quase nada do "o que a API
faz e como usá-la":

- Os headers públicos (`include/engine/*.hpp`) **não têm comentários de API**
  (nem Doxygen). O contrato do ciclo de vida das cenas
  (`onEnter → draw/input → onExit`) e a ordem esperada de chamadas não estão
  documentados em lugar nenhum.
- O `README.md` da raiz descreve a filosofia do projeto, mas **não mostra um
  exemplo de uso** — nem que o consumo é via `FetchContent` (o 8Puzzle faz isso).
- Não existe pasta `documentation/` (o 8Puzzle tem uma; inconsistência entre os
  dois projetos).

## Objetivo

Um consumidor (inclusive "você do futuro") consegue entender e usar a engine só
pela documentação, sem ler a implementação.

## Passos

1. **Doc-comments (Doxygen) nos headers públicos** de `include/engine/`:
   - Contrato e ordem do ciclo de vida em `IScene`
     (`onEnter`/`onEnterExecuted`/`isOnEnterExecuted`/`draw`/`input`/`onExit`).
   - Semântica de `IGameManager` (incluindo `shouldExit`, já renomeado na
     tarefa 02).
   - `IWindowManager`, `IState` (incluindo o porquê do `clone()` — prototype),
     e a `IRouter` já enxuta (tarefa 05).
2. **Seção "Uso" no README** com exemplo mínimo:
   - como declarar a dependência via `FetchContent` (espelhando o 8Puzzle);
   - montar `EngineManager` com um `WindowManager` e um `GameManager`;
   - registrar cenas via factory e navegar por estados.
3. (Opcional) criar `documentation/` com um diagrama das camadas
   (EngineManager → GameManager → Router → SceneRepository → Scene/State),
   alinhando com o padrão do 8Puzzle.
4. (Opcional) configurar geração Doxygen no CMake/CI.

## Critérios de aceite

- [x] Todos os headers públicos com doc-comment explicando contrato.
- [x] README com exemplo de consumo via FetchContent e montagem da engine.
- [x] Nenhum código comentado remanescente (coordenar com tarefa 09).

## Implementação

- **Doxygen** nos headers públicos de `core` (`IScene`, `IGameManager`,
  `IWindowManager`, `EngineManager`) e `routing` (`IState`, `IRouter`,
  `ISceneRepository`, `GameManager`, `RouterInMemory`, `SceneRepository`):
  brief de classe + contrato/ordem de cada método (ciclo de vida da cena,
  navegação em duas fases, `clone()` = Prototype, contratos de tempo de vida da
  tarefa 06).
- **README**: nova seção *Usage* com consumo via `FetchContent` (espelhando o
  8Puzzle, com nota de que os targets modulares vivem no `main`) e um exemplo
  mínimo de montagem (`SceneRepository → RouterInMemory → GameManager →
  EngineManager`, registro de cenas por factory e navegação por estados).
- **`.ai/build-and-test.md`**: atualizado — cobertura inclui `SceneLifetimeTest`,
  observações do snapshot marcadas como resolvidas, link morto corrigido.
- Os paths na descrição original desta tarefa (`include/engine/*.hpp`) eram
  pré-05a; a doc foi aplicada na estrutura atual (`core/…`, `modules/routing/…`).
- (Opcional) pasta `documentation/` e geração Doxygen no CMake/CI: **não feitos**
  — ficam como melhoria futura, sem bloquear o critério de aceite.

## Riscos

Nenhum funcional. O cuidado é **cronológico**: documentar por último, quando a
API (especialmente o Router) já estiver estável, para não retrabalhar a doc.
