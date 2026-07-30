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

**Ultima atualizacao:** 2026-07-30, apos a revisao do **Tactics** (10o jogo).

---

## Os repositorios

| repo | papel | estado |
|---|---|---|
| `cengine` | a engine | 27 tasks, 7 modulos, 17 releases, 3 ADRs |
| `platform-theforge-common` | casco Windows/The-Forge | 7 tasks, todas done, 10 releases |
| `The-Forge` | dependencia externa (D3D12) | vendorizada |
| `creative-lab` | orquestrador criativo (arte 2d, audio, 3d) | ex-`2d-art-lab`; nucleo neutro + pack `2d-art`; usado 1x (menu do Star Force) |
| 10 jogos | consumidores de validacao | 162 commits somados |

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
| 9 | bulwark | 9 | 07-28 | 0.13.0 | 4 (52 casos) | **MOUSE** (1o do ecossistema), economia, pilha VALIDADA |
| 10 | tactics | 10 | 07-30 | 0.14.0 | 4 (65 casos) | **iniciativa e orcamento de acao**; jogo so de PONTEIRO; 1o pathfinding |

O 8puzzle tem 52 commits porque apanhou junto com a engine nascendo. Do
terceiro em diante o custo estabiliza em ~11-16 commits por jogo — e isso e a
medida mais direta do que a plataforma economiza.

**O Bulwark e o mais barato de todos (9 commits) sendo o de mais fronteiras
novas.** Ele estreou o mouse, estreou economia e ainda produziu SETE releases
de lib. Custou menos porque nao improvisou: tres das extracoes ja estavam
decididas antes da primeira linha, e o resto do jogo consumiu engine pronta.

O Tactics manteve o patamar (10 commits) com o dominio mais denso do
ecossistema — 65 casos de teste, contra 52 do Bulwark e 64 do Delve.

## A engine, por natureza de task

| faixa | natureza | done | abertas |
|---|---|---|---|
| 01-09 | higiene e estrutura interna | 9 | 0 |
| 10-16 | arquitetura do loop e do routing | 7 | 0 |
| **17-27** | **candidatas vindas dos jogos** | **10** | **2** |

A ultima linha e a que importa para julgar o filtro de promocao: **das 11
candidatas nascidas em jogos, 10 subiram** — e as duas que restam nao esperam
mais consumidor: esperam DESENHO (18) e um caso que quatro jogos com colisao
nao produziram (22).

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
0.14.0  grid2d                 <- delve (ida) + bulwark e tactics (ida e volta)
0.14.0  input/Mouse            <- bulwark + tactics  (o 2o nao pediu mudanca
                                  nenhuma na API do 1o — o sinal mais forte)
0.12.0  anim: loop/finished    <- starforce + bulwark
0.13.0  IWindowManager::       <- o X da janela: buraco de CONTRATO, achado
        shouldClose()             jogando o bulwark, valia para os 9 jogos
```

No casco: `ForgeUi`, `ForgeLineUi`, `ForgeSpriteUi` (0.1-0.4), depois
`forgeaudio` e `Write-Dds` (0.5.0, validados pelo zelda), o **mouse** (0.6.0)
e o `Paint.ps1` (0.7.0, extraido de 5 copias a mao) — os dois do bulwark. Na
0.9.0 o mouse SAIU daqui para a engine, e a ponte voltou a so capturar.

**Nenhum modulo subiu com menos de dois jogos.** Varios subiram porque o
codigo ja era literalmente igual nos dois — a duplicacao apareceu sozinha e
foi recolhida.

## O que esta aberto

**Pela primeira vez, o que sobrou nao espera CONSUMIDOR — espera outra coisa.**

- **18 (roteamento espacial do clique)** — dois consumidores ja bateram
  (bulwark e tactics) e resolveram com contornos de FORMATOS DIFERENTES. Isso
  nao e "falta evidencia": e sinal de que o PROBLEMA esta maduro e a solucao
  nao. E a correcao obvia (o `input()` reportar consumo) **nao funcionaria
  sozinha**, porque o input e PUXADO de uma fila global — numa cascata, a
  primeira camada a olhar comeria o clique antes de decidir que nao era dela.
  Espera um desenho que resolva as tres metades juntas (cascata +
  espiar-sem-consumir + posicao) e um jogo que sofra o bastante para pagar o
  breaking.
- **22 (resolucao de colisao / MTV)** — 2/2 para eixo-separado, 0 para
  penetracao/MTV. Quatro jogos com colisao nao produziram o caso.

## O que foi levantado e MORTO, com argumento

- **Pool de efeitos transitorios** (revisao do Bulwark): parecia 2/2, mas a
  duplicacao real ja tinha subido na 0.12.0. Sobraram ~6 linhas de
  `std::vector`, e nem iguais. *Uma promocao bem feita esvazia a proxima
  candidata.*
- **Busca em largura sobre grade** (revisao do Tactics): o primeiro
  pathfinding do ecossistema, mecanismo puro e testavel — e vetado, porque os
  dois jogos com IA que navega **recusaram pathfinding por escrito no proprio
  codigo** (zelda `World.cpp:394`, delve `Dungeon.cpp:268`). Nenhum deixou de
  fazer por preguica: os dois DECIDIRAM que perseguicao gulosa era o
  comportamento certo — no Delve, encalhar e ate ferramenta do jogador. Nao e
  "ainda nao ha dois": e "o segundo caso existe e escolheu o contrario".

## A outra peneira: vetadas como politica

Seis candidatas foram explicitamente recusadas por serem **vocabulario de
jogo**, com a marca "nao reabrir sem argumento novo":

`Recordes` · `PlaySession` · `Wrap-around/toro` · `Fisica de plataforma` ·
`Events por quadro` · `Formatacao de tempo` · `Cooldown do som de tiro` ·
`Busca em largura sobre grade`

O `Cooldown do som de tiro` entrou fechando **2/2** (starforce previu, bulwark
confirmou) e mesmo assim **nao sobe**: quantos tiros por segundo o ouvido
aguenta e politica de APRESENTACAO. Registrar que uma previsao se confirmou
importa tanto quanto registrar uma promocao.

O `Events por quadro` e o caso mais instrutivo: zelda e Delve tem os dois, na
mesma forma (struct de bools). **Passa no criterio 2 e morre no criterio 1** —
dois consumidores nao salvam uma abstracao cujo conteudo e nome de coisa do
jogo. O bulwark deu a ele uma REGRA, paga com bug: **quem para de atualizar
tambem tem de parar de reportar**. E o **tactics mostrou que aquilo era o
SINTOMA**: a causa e o tempo de vida do evento estar amarrado ao RELOGIO.
Sendo por turnos, ele nao tem `update` — entao os eventos acumulam e a LEITURA
os consome, e o problema deixa de ser POSSIVEL em vez de evitado. A regra sobe
um nivel:

> O tempo de vida de um evento deve estar amarrado a **leitura**, e nao ao
> relogio. Quando depender do relogio, vale o corolario do bulwark.

A nova nao substitui a antiga — **diz quando cada uma vale**. Amarrar a leitura
custa UM LEITOR SO; com dois, limpar no relogio volta a ser o certo. Nenhuma
das duas vira tipo da engine.

## Leitura

O funil tem duas peneiras e as duas trabalham: o **criterio 2** (dois
consumidores) segura 2 candidatas hoje; o **criterio 1** (mecanismo x
politica) matou 8. Nenhum modulo entrou por especulacao.

E o Tactics mostrou o filtro respondendo TRES coisas diferentes no mesmo jogo:
a 26 subiu porque a copia era identica linha a linha; a 27 subiu por um motivo
melhor que contagem (**o 2o consumidor nao pediu nenhuma mudanca de API**); e a
18 nao subiu porque duas copias que DIVERGEM sao sinal de problema maduro e
solucao imatura. Um filtro que so soubesse contar teria promovido as tres.

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
O tactics foi escolhido para fechar tres, e fechou duas — com o parecer da
terceira valendo mais que a promocao teria valido.

**E aqui a pergunta chega ao seu limite, pela primeira vez.** Depois do
tactics nao sobra candidata que um jogo feche *por existir*: a 18 espera
DESENHO e a 22 espera um caso que quatro jogos com colisao nao produziram.
Escolher o 11o jogo por divida seria fingir que ha uma. O criterio precisa
mudar — e reconhecer isso em voz alta e mais util que forcar a pergunta velha.

Um contraponto que o tactics deixou, util para a proxima escolha: **um jogo
completo pode NAO USAR modulos disponiveis, e isso e informacao, nao lacuna.**
Ele ignorou `anim` (um tabuleiro por turnos nao anima) e `collision2d` (celulas
inteiras nao fazem geometria). Medir um modulo por quantos jogos o linkam
mediria a coincidencia de generos, e nao a qualidade do corte.

## Referencias

- [ADR 0001](decisions/0001-modular-core-vs-modules.md) — core x modulos
- [ADR 0002](decisions/0002-criterio-de-promocao-anti-deposito.md) — os tres
  criterios de promocao (o filtro anti-deposito)
- [ADR 0003](decisions/0003-consumidores-estacionados-documentacao-viva.md) —
  jogo congelado como documentacao viva
- [`.ai/task/README.md`](task/README.md) — o ledger de candidatas e os sweeps
  por jogo
