# 21 - IWindowManager obrigatorio: remover a hipotese do nullptr

- **Status:** done (0.6.0 — opcao B; consumidores estacionados, ver ADR 0003)
- **Prioridade:** media - os dois jogos consumidores ja dominam o modo
  proprio (8puzzle fase 2 e spaceinvaders task 02); o nullptr deixou de ser
  o caminho principal e virou caso especial mal expresso na API.
- **Categoria:** Arquitetura / core
- **Depende de:** 15 done (modo hospedado), 16 done (present no fim do
  quadro). Consumidores a migrar: ~~8puzzle (alvos fase 1 e fase 2) e
  spaceinvaders (ja em modo proprio)~~ — estacionados na 0.5.0 por decisao
  do ADR 0003; o asteroids valida o novo desenho.
- **Breaking:** sim - muda o construtor do `EngineManager`; ancora de um
  bump de versao (0.6.0).

## Decisao (registrada em 2026-07-13)

**Opcao B — modos separados por construcao.** Factories nomeadas
`EngineManager::owned(window, game, ...)` e `EngineManager::hosted(game, ...)`;
o construtor saiu da API publica. Racional contra a opcao A: o null-object
(`HostedWindowManager`) preservava a API, mas deixava `start()` numa config
hospedada rodando em silencio (loop girando sem ninguem apresentar). A B
conta a verdade no tipo — o modo hospedado NAO tem janela do lado da engine
e `start()` nele lanca `std::logic_error` com mensagem clara. O custo (mudar
a forma de construir nos call sites do modo proprio) foi neutralizado pela
decisao de estacionar os consumidores (ADR 0003).

## Contexto

Hoje o `EngineManager` aceita `windowManager == nullptr` para o modo
hospedado (task 15): o host e dono da janela/loop e chama `frame(dt)`. O
desenho funcionou e foi validado, mas expressa um contrato por convencao,
nao por tipo:

- `start()` com nullptr e um erro que so aparece em runtime;
- todo ponto que toca `m_windowManager` carrega um branch implicito;
- o doc do construtor precisa explicar "pode ser nullptr, mas so se...".

Com o spaceinvaders migrado para o modo proprio (task 02 daquele repo) e o
8puzzle fase 2 tambem, o dominio da etapa existe: da para exigir o
`IWindowManager` sempre e expressar o modo hospedado de forma explicita.

## Objetivo

Nenhum `nullptr` na API publica do `EngineManager`. Duas formas candidatas
(decidir no desenho):

### Opcao A - null-object explicito

```cpp
namespace cengine::core {

// Janela do modo hospedado: o HOST e dono da janela real; todas as fases
// de janela sao no-ops declarados.
class HostedWindowManager final : public IWindowManager {
public:
    void init() override {}
    void update() override {}
    void present() override {}
    void cleanup() override {}
};

} // namespace
```

O host constroi `EngineManager{ make_unique<HostedWindowManager>(), game }`
e segue chamando `frame(dt)`. O parametro vira obrigatorio (assert/throw em
nullptr) e os branches internos somem.

### Opcao B - modos separados por construcao

Factories nomeadas deixando o modo explicito no call site:

```cpp
auto owned  = EngineManager::owned(std::move(window), std::move(game));
auto hosted = EngineManager::hosted(std::move(game)); // frame(dt) only
```

`hosted()` nao guarda janela nenhuma; `start()` simplesmente nao existe
nesse caminho (ou lanca com mensagem clara).

Criterio de decisao: A e menor e preserva a API; B expressa melhor que
`start()` e `frame()` sao modos diferentes. Avaliar contra o uso real dos
dois jogos.

## Escopo Proposto

1. Decidir A ou B (registrar a decisao na task ou em ADR).
2. Implementar + testes na suite da cengine (construtor rejeita nullptr;
   modo hospedado continua com as garantias da task 15).
3. Migrar os consumidores: 8puzzle fase 1 (hosted), 8puzzle fase 2 e
   spaceinvaders (proprio) - apagar o `nullptr` dos call sites.
4. Atualizar docs do EngineManager/IWindowManager (o "pode ser nullptr"
   sai).

## Fora do Escopo

- Remover o modo hospedado (task 15 continua valida - editores/hosts com
  inversao de controle sao caso real; so a EXPRESSAO do modo muda).
- Mudar o contrato update()/present() do IWindowManager (task 16).

## Criterios de Aceite

- [x] API publica sem nullptr: construtor/factories exigem os dois
      colaboradores (`owned`/`hosted` lancam `std::invalid_argument`).
- [x] `start()` com configuracao hospedada falha de forma clara
      (`std::logic_error` com mensagem apontando `frame(dt)`).
- [x] Testes cobrem os dois modos e a rejeicao de configuracao invalida
      (suite 49/49 verde, build sem warnings).
- [x] ~~8puzzle e spaceinvaders migrados sem nullptr nos call sites~~ —
      substituido pelo ADR 0003: jogos estacionados na 0.5.0; o asteroids
      nasce nos factories como consumidor de validacao.

## Riscos

- Baixo: mudanca pequena e mecanica; o unico cuidado e nao quebrar as
  garantias de fixed timestep compartilhadas entre start() e frame()
  (cobertas por testes existentes da task 14/15).

## Relacionado

- Task 15 - origem do nullptr (modo hospedado).
- Task 16 - update()/present() que o modo proprio exercita.
- spaceinvaders task 02 e 8puzzle task 02 (fase 2) - os consumidores que
  deram dominio da etapa.
