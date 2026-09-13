# 30 - a cengine se publica para UM consumidor só (e tem dois)

- **Status:** **FECHADA EM NEGATIVO para os 12 jogos** (decisao do dono,
  2026-09-13). O residuo util ja esta feito — ver "A decisao que fechou", no fim.
- **Categoria:** Divida de plataforma (build), nao candidata a promocao
- **Registrada em:** 2026-09-07, na revisao arquitetural
  (`c++/revisao-arquitetural.md`, achado 1.1)
- **Reescrita em:** 2026-09-13 — **o escopo original estava no repo errado**; ver
  a secao "A correcao de escopo", abaixo
- **Task irma:** `platform-theforge-common/.ai/task/23-o-props-declara-o-que-exige.md`

## O fato, medido

Cada jogo declara a cengine **duas vezes**, por dois caminhos que nao conversam:

| lado | como acha a cengine | versao |
|---|---|---|
| CMake (dominio + testes) | `FetchContent` do GitHub | **pinada por `GIT_TAG`** |
| `.vcxproj` (o executavel) | `$(GameRoot)..\cengine\` | **o que estiver em disco** |

E o `.vcxproj` **nao linka a copia pinada**: ele enumera e compila os `.cpp` da
arvore de trabalho. Do `bulwark`, os dois lados lado a lado:

```
GIT_TAG 0.13.0                                      <- o que a suite prova
CengineRoot)core\src\EngineManager.cpp              <- o que o .exe roda
CengineRoot)modules\routing\src\GameManager.cpp
CengineRoot)modules\routing\src\RouterInMemory.cpp
CengineRoot)modules\routing\src\SceneStack.cpp
CengineRoot)modules\input\src\Keyboard.cpp
...
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

### Isto ja aconteceu

As correcoes da revisao (`RouterInMemory`, `GameManager`, `SceneRepository`,
`SceneStack`, `Keyboard`) entraram no caminho de compilacao do executavel dos 12
jogos no instante em que foram salvas. A varredura por padroes que as guardas
novas recusariam (`requestState(nullptr)`, factory vazia, construcao com router
nulo) nao achou ocorrencia, e a unica mudanca com efeito observavel
(`SceneStack`) alcanca bulwark e delve.

**Deu certo por sorte, e nao por desenho.**

## A correcao de escopo (2026-09-13)

A primeira versao desta task propunha, entre as saidas, *"o `.vcxproj` passa a
apontar o `CengineRoot` para `_deps`"* — uma edicao em 12 `.vcxproj` de jogos.

O dono apontou o problema: **`.vcxproj` e artefato do The-Forge, nao da
cengine.** Ele existe porque o jogo linka contra a cadeia de build do The-Forge
— `TF_Shared.props`, o `LibraryPath` da solution `Unit_Tests`, o `fsl.targets`,
os exports do Agility SDK. Nada disso e assunto desta engine.

**Medido:** a cengine nao tem nenhum artefato MSBuild escrito a mao. Os
`.vcxproj` sob `build/` sao gerados pelo CMake, em pasta ignorada. Ela publica
**uma** forma de ser consumida, e tem **duas** classes de consumidor.

> O fato do achado continua inteiro. O que estava errado era **de quem e o
> mecanismo** — e a task prescrevia trabalho em arquivos que este repo nao
> possui.

### A regra que decide, e ela ja existia

A task 16 do casco fechou com uma regra que vale aqui:

> **Quem tem os arquivos e quem os descreve.**

O casco descreve o casco (`TheForgeCommon.props`). Pela mesma regra, a cengine
descreve a cengine — e o fato de a descricao ser MSBuild e consequencia de
**quem consome**, e nao propriedade de **quem possui**.

**Isto nao contraria o ADR 0001.** Ele proibe a cengine de escolher biblioteca
grafica; um `.props` que lista fontes e include paths nao e decisao grafica. A
engine ja publica um descritor de build (CMake); publicar o segundo nao e
mudanca de natureza.

**O custo honesto:** a cengine passa a carregar um arquivo especifico de
Windows/MSBuild, e ela e feita para ser portatil. O arquivo e inerte em qualquer
outra plataforma, mas esta la.

## O escopo desta task

**1. `cengine.props` — a engine se descreve para MSBuild.**

Espelho do `TheForgeCommon.props`, com uma diferenca que importa: os modulos da
cengine sao **opt-in** (ADR 0001), entao a lista nao pode ser plana. Mesmo
mecanismo que o casco usou para o `ForgeAudio.cpp`:

```xml
<ItemGroup Condition="'$(CengineRouting)' == 'true'">
  <ClCompile Include="$(CengineDir)modules\routing\src\*.cpp" />
</ItemGroup>
```

O consumidor liga o que usa, como ja faz hoje enumerando a mao — a diferenca e
que passa a ligar por NOME de modulo, e nao por caminho de arquivo.

**2. A versao, em forma conferivel por MSBuild.**

A 0.17.0 ja publica `cengine_VERSION` no CMake e o `VersionTest` que confere o
CHANGELOG. Falta o mesmo numero alcancavel de fora do CMake — uma propriedade no
`.props`, ou um `Version.hpp` gerado.

**3. A pinagem, que e a pergunta aberta.** Ver abaixo.

## As duas saidas que restam

A saida **B** da versao original (`install()` + `find_package`) segue descartada
pela mesma razao: **MSBuild nao fala `find_package`**. Ela ficaria bonita no
CMake e nao tocaria no binario, que e onde o problema esta.

### A. O `.exe` passa a consumir o que o CMake ja pina

O jogo ja baixa a cengine na versao certa (`_deps/cengine-src`); o `CengineDir`
apontaria para la.

- **A favor:** uma versao so por jogo. A pinagem que ja existe passa a valer para
  o binario.
- **Contra:** o `.exe` passa a depender de o CMake ter rodado antes. Hoje os dois
  builds sao independentes.
- **Custo:** uma linha por `.vcxproj`, em 12 jogos — **todos REFERENCIA**. Entao
  a adocao e opt-in, projeto a projeto, como a task 16 do casco fez.

### C. A arvore declara sua versao, e o build CONFERE

O `.props` da cengine declara a versao que a arvore tem; o consumidor declara a
que espera; o build **falha** quando divergem.

- **A favor:** e a unica das duas que serve para o **casco** tambem — e o casco e
  o caso sem pinagem nenhuma (`CommonRoot` nos 14 `.vcxproj`, versao so numa
  linha do `README.md`). Nao exige mudar o modelo de build de ninguem.
- **Contra:** e mecanismo caseiro, e move a verdade para a arvore — o `GIT_TAG`
  passa a ser o que pode estar errado.

## A pergunta que fica para o dono

> O executavel deve seguir a versao **pinada** (A), ou a **arvore** deve declarar
> sua versao e ser conferida (C)?

Sao respostas diferentes para *"o que e a verdade sobre qual cengine este jogo
usa"*. Em (A) a verdade e o `GIT_TAG`; em (C) e a arvore.

**Os itens 1 e 2 do escopo valem nas duas** — a engine precisa se descrever e
publicar sua versao de qualquer jeito. So o item 3 depende da resposta.

## Criterios de aceite (quando a decisao existir)

1. Abrir um jogo estacionado e compilar o `.exe` usa a MESMA cengine que a suite
   dele — ou o build falha dizendo qual e a divergencia.
2. O mesmo vale para o casco (ver a task irma, que fecha a outra metade).
3. Um consumidor MSBuild liga um modulo da cengine por **nome**, e nao
   enumerando `.cpp`.
4. A prova nao e "compilei um jogo antigo" (proibido pela fronteira do
   workspace): e ler o arquivo de pinagem e o que o build resolve.

## O primeiro consumidor da saida (A) — `diorama`, 2026-09-13

O dono mandou publicar as tags e apontar o lab para elas. Feito, e fechou a
metade do achado que um consumidor consegue fechar sozinho.

**As tags sairam:** `cengine 0.17.0`, e no casco `0.13.0`, `0.14.0`, `0.15.0` e
`0.22.1` (tres retroativas, em commits cujas mensagens ja nomeavam a versao).

> **As versoes que NAO ganharam tag, e por que.** A `cengine 0.16.0` e as
> `0.16.0`–`0.22.0` do casco nunca existiram como pontos separados: um commit
> carrega varias. Uma tag apontando para codigo que ja e a versao seguinte
> mentiria sobre o que marca, e esta escrito dentro da mensagem das duas tags.

**Como o `diorama` consome:** o `FetchContent` do `CMakeLists.txt` do lab, com
`SOURCE_DIR` explicito em `deps/cengine`. O caminho estavel e o que permite ao
MSBuild participar — o `_deps` padrao vive dentro do diretorio de build, que
muda com o preset. O `.vcxproj` aponta para a mesma pasta.

**Medido:** o comando do compilador passou a citar
`diorama\deps\cengine\core\src\EngineManager.cpp`. Suite do lab 15/15, `.exe`
verde.

### O que isto ensina para a decisao que resta

O custo previsto na saida (A) — *"o `.exe` passa a depender de o CMake ter
rodado antes"* — se confirmou, e tem remedio barato: um `<Target>` com `<Error>`
no `.vcxproj` que diz **qual comando rodar**, em vez de deixar o build falhar
com "cabecalho nao encontrado" trinta linhas depois.

E apareceu um custo que a task nao tinha previsto, e ele decide QUANDO pinar:

> **Pinar quebra a edicao direta.** Editar a arvore ao lado e rebuildar deixa de
> testar a mudanca. Para um consumidor que so USA a dependencia, isso nao custa
> nada. Para um que a esta CONSTRUINDO junto, custa uma tag por iteracao.

Foi por isso que o casco ficou de fora no `diorama`: as tasks 14, 15b e 18 mexem
no `forgemesh` ao mesmo tempo que os degraus 08 e 09 o consomem. A tag `0.22.1`
existe e espera.

**A regra que sai disto, e ela serve para os 12 jogos:** *pina-se o que se
consome, nao o que se constroi.* Jogo estacionado nunca constroi a engine —
entao para eles a saida (A) e so ganho, e o obstaculo e o custo de adocao (uma
linha por `.vcxproj`, todos REFERENCIA), e nao o desenho.

## A decisao que fechou (2026-09-13)

> *"os 12 jogos ja feitos nao serao tocados. Servem de historico de estudo."*

**Isto fecha o achado para eles, e fecha em NEGATIVO** — a mesma forma com que a
task 18, a 22b e o broadphase fecharam neste ecossistema: a pergunta deixa de
ter resposta porque deixa de ter caso.

A divergencia entre a suite e o `.exe` de um jogo so se manifesta se alguem
**recompilar** aquele jogo. O ADR 0003 ja dizia que jogo estacionado nao e
recompilado; o que mudou hoje e que isso deixou de ser suposicao operacional e
virou **decisao declarada**. Nao ha, nem havera, o evento que expoe o defeito.

### O que NAO fecha junto

**O padrao, para quem vier depois.** Ele ja existe, medido e em uso no
`diorama`:

| dependencia | mecanismo | por que |
|---|---|---|
| o que se **consome** | tag (`FetchContent` + `SOURCE_DIR` estavel) | a versao e a verdade |
| o que se **constroi junto** | conferencia (`...ExpectedVersion` no `.props`) | uma linha para subir, em vez de um release por iteracao |

**O item 1 do escopo (`cengine.props`) continua valendo, e por outro motivo.**
Nao pela pinagem: porque o `.vcxproj` do `diorama` ainda **enumera os `.cpp` da
engine** (`EngineManager.cpp`, `Keyboard.cpp`, `Mouse.cpp`, `Camera.cpp`). No
dia em que um modulo ganhar um segundo `.cpp`, o link quebra — que e exatamente
o defeito que a task 09 do casco documentou e que a task 16 de la resolveu para
o casco.

Com **um** consumidor MSBuild, o risco e pequeno e conhecido. Fica registrado
aqui em vez de virar trabalho agora.

### O item 2 (a versao alcancavel por MSBuild)

Feito do lado do casco (task 21, item 3: `TheForgeCommonVersion` + conferencia
opt-in). Do lado da cengine, ele so tem uso junto com o item 1 — e espera com
ele.
