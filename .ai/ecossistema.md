# Retrato do ecossistema

Fonte unica do estado do ecossistema inteiro — engine, casco e os jogos que a
alimentam. Existe porque o aprendizado se espalha por 11 repositorios e nenhum
deles sozinho responde "onde estamos".

> **Como manter:** este arquivo e atualizado na **revisao de candidatas**, que
> acontece ao fechar cada jogo (mesmo momento em que o sweep entra em
> `.ai/task/README.md`). Fechou jogo, revisou candidatas, atualiza aqui. Se a
> data abaixo estiver mais velha que o ultimo jogo fechado, o retrato mentiu.
>
> Ha uma **versao visual** do mesmo retrato em `ecossistema.html` (publicada
> como artefato). Os dois carregam os mesmos numeros: **atualize os dois na
> mesma revisao**, senao um deles vira mentira silenciosa. Este `.md` e a
> fonte legivel no terminal e no diff; o `.html` e a vista para olhar.

**Ultima atualizacao:** 2026-07-28, apos a revisao do **Bulwark** (9o jogo).

---

## Os repositorios

| repo | papel | estado |
|---|---|---|
| `cengine` | a engine | 27 tasks, 6 modulos, 16 releases, 3 ADRs |
| `platform-theforge-common` | casco Windows/The-Forge | 7 tasks, todas done, 9 releases |
| `The-Forge` | dependencia externa (D3D12) | vendorizada |
| `creative-lab` | orquestrador criativo (arte 2d, audio, 3d) | ex-`2d-art-lab`; nucleo neutro + pack `2d-art`; usado 1x (menu do Star Force) |
| 9 jogos | consumidores de validacao | 150 commits somados |

## Os jogos

| # | jogo | commits | ultimo | cengine | testes | fronteira que ele provou |
|---|---|---|---|---|---|---|
| 1 | 8puzzle | 52 | 07-18 | 0.5.0 (congelado) | 2 | routing, modo hospedado |
| 2 | spaceinvaders | 16 | 07-15 | 0.5.0 (congelado) | 2 | sprites, colisao (1a evidencia) |
| 3 | asteroids | 16 | 07-18 | 0.8.0 | 3 | colisao (2a evidencia) |
| 4 | breakout | 12 | 07-17 | 0.8.0 | 3 | porta de input, audio (1a) |
| 5 | mario-bros | 11 | 07-17 | 0.9.0 | 5 | audio (2a), camera (1a), anim (1a) |
| 6 | zelda | 10 | 07-19 | 0.10.0 | 5 | camera (2a), anim (2a), IA que navega |
| 7 | starforce | 14 | 07-22 | 0.10.0 | 7 | massa de entidades, roteiro por scroll |
| 8 | delve | 11 | 07-28 | 0.10.0 | 4 (64 casos) | pilha de camadas, jogo por TURNOS |
| 9 | bulwark | **8** | 07-28 | 0.13.0 | 4 (52 casos) | **MOUSE** (1o do ecossistema), economia, pilha VALIDADA |

O 8puzzle tem 52 commits porque apanhou junto com a engine nascendo. Do
terceiro em diante o custo estabiliza em ~11-16 commits por jogo — e isso e a
medida mais direta do que a plataforma economiza.

**O Bulwark e o mais barato de todos (8) sendo o de mais fronteiras novas.**
Ele estreou o mouse, estreou economia e ainda produziu SETE releases de lib.
Custou menos porque nao improvisou: tres das extracoes ja estavam decididas
antes da primeira linha, e o resto do jogo consumiu engine pronta.

## A engine, por natureza de task

| faixa | natureza | done | abertas |
|---|---|---|---|
| 01-09 | higiene e estrutura interna | 9 | 0 |
| 10-16 | arquitetura do loop e do routing | 7 | 0 |
| **17-27** | **candidatas vindas dos jogos** | **8** | **3** |

A ultima linha e a que importa para julgar o filtro de promocao: **das 11
candidatas nascidas em jogos, 8 subiram.**

## Quem pariu cada modulo

```
0.6.0   routing / FlowRouter   <- 8puzzle
0.6.0   IWindowManager         <- 8puzzle + spaceinvaders
0.7.0   collision2d            <- spaceinvaders + asteroids
0.8.0   input (porta)          <- breakout
0.9.0   audio (porta)          <- breakout + mario
0.10.0  camera2d               <- mario + zelda  (codigo identico linha a linha)
0.10.0  anim                   <- mario + zelda
0.11.0  routing/SceneStack     <- delve (escreveu) + bulwark (validou)
0.12.0  anim: loop/finished    <- starforce + bulwark
0.13.0  IWindowManager::       <- o X da janela: buraco de CONTRATO, achado
        shouldClose()             jogando o bulwark, valia para os 9 jogos
```

No casco: `ForgeUi`, `ForgeLineUi`, `ForgeSpriteUi` (0.1-0.4), depois
`forgeaudio` e `Write-Dds` (0.5.0, validados pelo zelda), o **mouse** (0.6.0)
e o `Paint.ps1` (0.7.0, extraido de 5 copias a mao) — os dois do bulwark.

**Nenhum modulo subiu com menos de dois jogos.** Varios subiram porque o
codigo ja era literalmente igual nos dois — a duplicacao apareceu sozinha e
foi recolhida.

## O que esta aberto

**As tres primeiras sao a MESMA aposta: todas fecham se o 10o jogo tiver
ponteiro, e nenhuma fecha se ele nao tiver.**

- **roteamento ESPACIAL de input na `SceneStack`** — 1 consumidor. A pilha
  subiu (0.11.0) com um limite escrito: "so a primeira camada ativa do topo
  recebe input" e correto para overlay MODAL e insuficiente para overlay
  CLICAVEL nao-modal. Com ponteiro, "quem recebe o clique" e espacial, e
  `IScene::input()` nao reporta consumo.
- **27 (mouse como porta)** — 1/2. Vive no `forgeui` com vocabulario local,
  como o teclado antes da task 20 — que so subiu na **4a copia identica**.
- **26 (grade celula <-> pixel)** — 1/2 na metade que paga. Delve e Bulwark
  escrevem a mesma conta de grade centrada, mas pelo precedente da `camera2d`
  (origem e politica) a IDA que restaria e uma multiplicacao e uma soma. A
  VOLTA (pixel -> celula, com o teste de negativo ANTES do cast) tem substancia
  e tem um consumidor so.
- **22 (resolucao de colisao / MTV)** — 2/2 para eixo-separado, 0 para MTV.
  Inalterada pelo bulwark.

E uma que **morreu** nesta revisao, com argumento: o **pool de efeitos
transitorios** (starforce + bulwark). Parecia 2/2, mas a duplicacao real ja
tinha subido na 0.12.0 — o que sobrou sao ~6 linhas de `std::vector`, e nem
iguais. **Uma promocao bem feita esvazia a proxima candidata:** o que resta
parece candidato pela silhueta e ja e so idioma.

## A outra peneira: vetadas como politica

Seis candidatas foram explicitamente recusadas por serem **vocabulario de
jogo**, com a marca "nao reabrir sem argumento novo":

`Recordes` · `PlaySession` · `Wrap-around/toro` · `Fisica de plataforma` ·
`Events por quadro` · `Formatacao de tempo` · `Cooldown do som de tiro`

O `Cooldown do som de tiro` entrou fechando **2/2** (starforce previu, bulwark
confirmou) e mesmo assim **nao sobe**: quantos tiros por segundo o ouvido
aguenta e politica de APRESENTACAO. Registrar que uma previsao se confirmou
importa tanto quanto registrar uma promocao.

O `Events por quadro` e o caso mais instrutivo: zelda e Delve tem os dois, na
mesma forma (struct de bools). **Passa no criterio 2 e morre no criterio 1** —
dois consumidores nao salvam uma abstracao cujo conteudo e nome de coisa do
jogo. O bulwark deu a ele uma REGRA, paga com bug: **quem para de atualizar
tambem tem de parar de reportar** — o tempo de vida do evento esta amarrado ao
do `update`, e todo lugar onde o `update` para e um lugar onde o evento mente.
Nao vira tipo; vira teste-padrao (*depois do fim, os eventos estao vazios*).

## Leitura

O funil tem duas peneiras e as duas trabalham: o **criterio 2** (dois
consumidores) segura 4 candidatas hoje; o **criterio 1** (mecanismo x
politica) matou 7. Nenhum modulo entrou por especulacao.

O ponto cego conhecido: o metodo e enviesado para EXTRACAO, e por isso
estruturalmente cego para abstracao que so paga se desenhada antes de existir
(uma pilha de cenas e desse tipo). O contrapeso e a formula **"o proximo jogo
extrai, nao copia"**, que antecipa a extracao para o momento do segundo
consumidor em vez de esperar a segunda copia.

**O bulwark foi o teste dessa formula, e ela passou.** Ele nasceu devendo tres
extracoes ja prometidas por escrito e executou as tres DENTRO dos degraus, cada
uma no degrau que precisava dela — mais quatro correcoes que sairam de jogar.
Sete releases de lib em um jogo de oito commits. **A consequencia e que a
revisao final dele chegou vazia, e isso e o desenho funcionando, nao a peneira
apertada demais:** as promocoes aconteceram mais cedo. Foi a resposta a duvida
levantada na revisao do Delve ("nao estamos com regras rigorosas demais e
acabando nao levando nada?").

A pergunta que o ecossistema aprendeu a fazer antes de escolher o proximo
jogo, por causa disso, nao e "que genero falta?" — e **"que candidata parada
ele fecharia?"**. O bulwark foi escolhido porque convertia a task 18. Converteu.

## Referencias

- [ADR 0001](decisions/0001-modular-core-vs-modules.md) — core x modulos
- [ADR 0002](decisions/0002-criterio-de-promocao-anti-deposito.md) — os tres
  criterios de promocao (o filtro anti-deposito)
- [ADR 0003](decisions/0003-consumidores-estacionados-documentacao-viva.md) —
  jogo congelado como documentacao viva
- [`.ai/task/README.md`](task/README.md) — o ledger de candidatas e os sweeps
  por jogo
