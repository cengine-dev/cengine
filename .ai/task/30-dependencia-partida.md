# 30 - a dependencia PARTIDA: o `.exe` e os testes veem cengines diferentes

- **Status:** ABERTA — precisa de decisao do dono antes de virar codigo
- **Categoria:** Divida de plataforma (build), nao candidata a promocao
- **Registrada em:** 2026-09-07, na revisao arquitetural (`c++/revisao-arquitetural.md`, achado 1.1)

## O fato, medido

Cada jogo declara a cengine **duas vezes**, por dois caminhos que nao conversam:

| lado | como acha a cengine | versao |
|---|---|---|
| CMake (dominio + testes) | `FetchContent` do GitHub | **pinada por `GIT_TAG`** |
| `.vcxproj` (o executavel) | `$(GameRoot)..\cengine\` | **o que estiver em disco** |

E o `.vcxproj` **nao linka a copia pinada**: ele enumera e compila os `.cpp` da
arvore de trabalho. Do `bulwark/CMakeLists.txt` e do `.vcxproj` dele, lado a
lado:

```
GIT_TAG 0.13.0                                      <- o que a suite prova
CengineRoot)core\src\EngineManager.cpp              <- o que o .exe roda
CengineRoot)modules\routing\src\GameManager.cpp
CengineRoot)modules\routing\src\RouterInMemory.cpp
CengineRoot)modules\routing\src\SceneRepository.cpp
CengineRoot)modules\routing\src\SceneStack.cpp
CengineRoot)modules\input\src\Keyboard.cpp
CengineRoot)modules\collision2d\src\Intersects.cpp
CengineRoot)modules\anim\src\Animator.cpp
```

**A suite de um jogo prova o comportamento de uma versao da engine, e o
executavel dele roda outra.**

Nao e "falta pinagem". E **pinagem que existe e nao cobre o binario** — o que e
pior, porque a pinagem existente da a impressao de que o assunto esta resolvido.

### O levantamento completo

Pinado por `GIT_TAG` no lado CMake: 8puzzle e spaceinvaders 0.5.0; asteroids e
breakout 0.8.0; mario-bros 0.9.0; zelda, starforce e delve 0.10.0; bulwark e
tactics 0.13.0; klondike e counter 0.14.0.

Sem cengine no CMake (so no `.vcxproj`, portanto **so** pela arvore): cue, fold,
vigil, diorama.

**O casco e outro caso, e nele nao ha pinagem nenhuma:** `CommonRoot =
$(GameRoot)..\platform-theforge-common\` nos 14 `.vcxproj`. Ele e a unica
dependencia do ecossistema sem nenhuma forma de versao.

## Por que isto nao tinha aparecido

Porque o ADR 0003 resolve o problema **por disciplina**: jogos estacionados nao
sao recompilados, entao a divergencia nunca e observada. A regra funciona
enquanto ninguem abre um jogo antigo — e ela e exatamente o tipo de regra que a
[[projetos-referencia-nao-tocar]] existe para lembrar.

Mas a regra protege o jogo, nao a ENGENHARIA: quem mexe na cengine hoje nao tem
como saber o que quebrou, porque a unica suite que existe e a da propria cengine.
A suite de cada jogo esta olhando para uma versao congelada no GitHub.

**Isto ja aconteceu nesta sessao.** As correcoes do Grupo 1 da revisao
(`RouterInMemory`, `GameManager`, `SceneRepository`, `SceneStack`, `Keyboard`)
entraram no caminho de compilacao do executavel dos 12 jogos no instante em que
foram salvas. A varredura por padroes que as guardas novas recusariam
(`requestState(nullptr)`, factory vazia, construcao com router nulo) nao achou
ocorrencia nenhuma, e a unica mudanca com efeito observavel (`SceneStack`)
alcanca bulwark e delve. Deu certo — **por sorte, e nao por desenho**.

## As tres saidas

### A. O `.vcxproj` passa a consumir o que o CMake ja pina

O jogo ja baixa a cengine na versao certa (`_deps/cengine-src`). O `.vcxproj`
apontaria o `CengineRoot` para la, em vez de `..\cengine`.

- **A favor:** uma versao so por jogo, sem inventar mecanismo novo. A pinagem que
  ja existe passa a valer para o binario.
- **Contra:** o `.exe` passa a depender de o CMake ter rodado antes (a pasta
  `_deps` so existe depois do configure). Hoje os dois builds sao independentes.
- **Custo:** uma linha por `.vcxproj`, em 12 jogos — **e todos sao REFERENCIA**.
  Nao da para fazer sem quebrar a fronteira do workspace, entao seria "cada
  projeto adota quando alguem voltar a toca-lo", como a task 16 do casco fez.

### B. A cengine ganha `install()` + `find_package`

O caminho canonico de CMake: `install(TARGETS ... EXPORT)`, `cengineConfig.cmake`,
e o consumidor pede `find_package(cengine 0.17 REQUIRED)`.

- **A favor:** resolve de verdade, e resolve para qualquer consumidor futuro.
- **Contra:** **nao resolve o `.vcxproj`**, que e onde o problema esta. MSBuild
  nao fala `find_package`. Ficaria bonito no CMake e nao tocaria no binario.
- **Veredito preliminar:** sozinha, ela nao responde a esta task.

### C. Um arquivo de pinagem que o build CONFERE

Um `cengine.pin` (ou uma propriedade no `.props`) declarando a versao esperada, e
uma checagem que FALHA o build quando a arvore nao bate.

- **A favor:** e a unica das tres que serve para o CASCO tambem — e o casco e o
  caso sem pinagem nenhuma. Nao exige mudar o modelo de build de ninguem.
- **Contra:** e mecanismo caseiro; a conferencia precisa de um numero que a
  cengine publique (o `VersionTest`/`cengine_VERSION` da 0.17.0 ja da isso).
- **Nota:** casa com a task irma do casco (a versao dele hoje so existe no
  `README.md`).

## A pergunta que fica para o dono

> O executavel dos jogos deve seguir a versao pinada (A), ou a arvore deve
> declarar sua versao e ser conferida (C)?

Sao respostas diferentes para "o que e a verdade sobre qual cengine este jogo
usa": em (A) a verdade e o `GIT_TAG`; em (C) e a arvore, e o `GIT_TAG` passa a
ser o que esta errado.

**O que NAO se deve fazer e escolher (B) achando que fecha o assunto.**

## Criterios de aceite (quando a decisao existir)

1. Abrir um jogo estacionado e compilar o `.exe` usa a MESMA cengine que a suite
   dele — ou o build falha dizendo qual e a divergencia.
2. O mesmo vale para o casco, que hoje nao tem versao em lugar nenhum alem de uma
   linha do `README.md`.
3. A prova nao e "compilei um jogo antigo" (proibido pela fronteira do
   workspace): e ler o arquivo de pinagem e o que o build resolve.
