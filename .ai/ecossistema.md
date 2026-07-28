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

**Ultima atualizacao:** 2026-07-28, apos a revisao do **Delve** (8o jogo).

---

## Os repositorios

| repo | papel | estado |
|---|---|---|
| `cengine` | a engine | 25 tasks, 6 modulos, 12 releases, 3 ADRs |
| `platform-theforge-common` | casco Windows/The-Forge | 6 tasks, todas done, 5 releases |
| `The-Forge` | dependencia externa (D3D12) | vendorizada |
| `2d-art-lab` | laboratorio de arte | usado 1x (menu do Star Force) |
| 8 jogos | consumidores de validacao | 142 commits somados |

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

O 8puzzle tem 52 commits porque apanhou junto com a engine nascendo. Do
terceiro em diante o custo estabiliza em ~11-16 commits por jogo — e isso e a
medida mais direta do que a plataforma economiza.

## A engine, por natureza de task

| faixa | natureza | done | abertas |
|---|---|---|---|
| 01-09 | higiene e estrutura interna | 9 | 0 |
| 10-16 | arquitetura do loop e do routing | 7 | 0 |
| **17-25** | **candidatas vindas dos jogos** | **7** | **2** |

A ultima linha e a que importa para julgar o filtro de promocao: **das 9
candidatas nascidas em jogos, 7 subiram.**

## Quem pariu cada modulo

```
0.6.0   routing / FlowRouter   <- 8puzzle
0.6.0   IWindowManager         <- 8puzzle + spaceinvaders
0.7.0   collision2d            <- spaceinvaders + asteroids
0.8.0   input (porta)          <- breakout
0.9.0   audio (porta)          <- breakout + mario
0.10.0  camera2d               <- mario + zelda  (codigo identico linha a linha)
0.10.0  anim                   <- mario + zelda
```

No casco: `ForgeUi`, `ForgeLineUi`, `ForgeSpriteUi` (0.1-0.4), depois
`forgeaudio` e `Write-Dds` (0.5.0, validados pelo zelda).

**Nenhum modulo subiu com menos de dois jogos.** Varios subiram porque o
codigo ja era literalmente igual nos dois — a duplicacao apareceu sozinha e
foi recolhida.

## O que esta aberto

- **18 (scene stack/overlays)** — PRONTA PARA EXTRAIR, esperando consumidor de
  validacao. O Delve cumpriu as tres condicoes, escreveu a pilha e validou o
  desenho. O proximo jogo que precisar de camadas **extrai esta pilha**, nao
  escreve a propria.
- **22 (resolucao de colisao / MTV)** — 0/2. Quatro jogos com colisao e todos
  resolveram por eixo separado; o caso de penetracao/MTV nunca apareceu.

## A outra peneira: vetadas como politica

Seis candidatas foram explicitamente recusadas por serem **vocabulario de
jogo**, com a marca "nao reabrir sem argumento novo":

`Recordes` · `PlaySession` · `Wrap-around/toro` · `Fisica de plataforma` ·
`Events por quadro` · `Formatacao de tempo`

O `Events por quadro` e o caso mais instrutivo: zelda e Delve tem os dois, na
mesma forma (struct de bools). **Passa no criterio 2 e morre no criterio 1** —
dois consumidores nao salvam uma abstracao cujo conteudo e nome de coisa do
jogo.

## Leitura

O funil tem duas peneiras e as duas trabalham: o **criterio 2** (dois
consumidores) segurou 2 candidatas; o **criterio 1** (mecanismo x politica)
matou 6. Nenhum modulo entrou por especulacao.

O ponto cego conhecido: o metodo e enviesado para EXTRACAO, e por isso
estruturalmente cego para abstracao que so paga se desenhada antes de existir
(uma pilha de cenas e desse tipo). O contrapeso e a formula **"o proximo jogo
extrai, nao copia"**, que antecipa a extracao para o momento do segundo
consumidor em vez de esperar a segunda copia. Tamanho do problema, medido:
**uma candidata em nove.**

## Referencias

- [ADR 0001](decisions/0001-modular-core-vs-modules.md) — core x modulos
- [ADR 0002](decisions/0002-criterio-de-promocao-anti-deposito.md) — os tres
  criterios de promocao (o filtro anti-deposito)
- [ADR 0003](decisions/0003-consumidores-estacionados-documentacao-viva.md) —
  jogo congelado como documentacao viva
- [`.ai/task/README.md`](task/README.md) — o ledger de candidatas e os sweeps
  por jogo
