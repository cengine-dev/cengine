# 28 - Arrastar como parte da porta de ponteiro

- **Status:** PROMOVIDA (0.15.0, 2026-08-03)
- **Prioridade:** media - o segundo consumidor chegou e nao pediu mudanca.
- **Categoria:** Modulo `input`
- **Depende de:** task 27 (a porta de ponteiro).

## O gate, e como ele foi cumprido

O ledger registrava a candidata em **1/2, sem task aberta**, e escrevia a frase
que a destrancaria:

> Fica la, pelo caminho que o mouse ja percorreu (task 27): viveu no `forgeui`
> por dois jogos e so subiu quando o 2o consumidor usou a forma do 1o **sem pedir
> mudanca de API**.

O segundo consumidor chegou: **cue, degrau 06 — a tacada por arrasto.**

E o placar foi o mais limpo que este filtro consegue produzir:

| a leitura | klondike (1o) | cue (2o) |
|---|---|---|
| `drag()` — estado | desenhar a carta na mao, todo quadro | desenhar o taco e a mira, todo quadro |
| `readDrop()` — edge | soltar a carta, uma vez por gesto | dar a tacada, uma vez por gesto |

**As duas leituras, os quatro campos de cada uma, e nenhuma mudanca de API
pedida.**

## Os tres criterios do ADR 0002

**1. Mecanismo puro, zero vocabulario de jogo.** `DragState` e `Drop` falam de
botao, posicao e comeco/fim. Nao ha carta, nem taco, nem alvo.

**2. Duas evidencias reais, escritas a mao em dois jogos.** Klondike (common
0.10.0) e cue (degrau 06). **Nunca existiu uma segunda copia manual** — este
degrau e a extracao, e ela aconteceu COM o consumidor.

**3. Testavel dentro da cengine, sem jogo e sem GPU.** Este e o ganho concreto: o
estado e a fila viviam numa unidade de traducao do casco do The-Forge, e nao
tinham como ser testados sem uma janela. Agora ha nove testes, e dois deles
**encarnam os casos dos dois jogos**, com repositorio, commit e linha citados.

## O que a promocao NAO levou, e nunca foi candidato

O **`SetCapture`**. O ledger ja tinha antecipado isso, e continua valendo: sem
capturar o ponteiro no aperto, soltar o botao fora da janela nao gera
`WM_LBUTTONUP` nenhum e o gesto fica pendurado para sempre. Mas isso e Win32, e o
**WndProc E o casco** — nao ha o que subir. O que subiu foi o `cancelDrag()`, que
e a resposta em vocabulario de porta ao que o casco descobre em vocabulario de
janela.

## A recusa a decidir foi TESTADA por um segundo caso

O Klondike escreveu que a folga em pixels e politica do jogo:

> Depende do tamanho do alvo e de quanto tremor de mao se perdoa. Um casco que
> decidisse isso estaria opinando sobre alvos que ele nao conhece.

Ate aqui isso era uma boa razao. **Agora e um fato medido**: o Klondike usa 8
pixels para uma carta de 90; o cue usa 12 para uma bola de 20 — e o que os dois
medem nem e a mesma pergunta ("errou o alvo?" contra "quis mesmo tacar?"). Uma
porta que tivesse fixado 8 estaria errada no segundo jogo.

**E a primeira vez no ecossistema que uma recusa a opinar e validada em vez de so
declarada.** Ela vale como precedente: quando o mecanismo tem um numero de
tolerancia, o numero fica no consumidor.

## A terceira porta com a mesma forma

O ledger previu isto tambem, e acertou:

> A forma provavel ja tem precedente — o par estado+edge e o MESMO do teclado
> (`isHeld` + `readKey`, 0.8.0), e tres portas de input com a mesma forma seria
> evidencia forte.

Sao tres, agora:

| porta | estado | edge |
|---|---|---|
| teclado (0.8.0) | `isHeld` | `readKey` |
| ponteiro (0.14.0) | `x`/`y` | `readClick` |
| arrastar (0.15.0) | `drag` | `readDrop` |

E o arrastar e o caso em que a dualidade **nao e conveniencia, e sim
necessidade**: o MESMO aperto abre as duas leituras, e nao da para saber qual das
duas o jogador quis ate ele soltar.

## O achado que NAO virou mudanca

O segundo consumidor notou uma falta e decidiu nao pedir: **o `Drop` carrega as
duas pontas, e nao carrega o instante em que o gesto comecou.**

O caso do cue: puxar a branca enquanto as bolas rolam e soltar logo depois de
tudo parar — a tacada sai sem nunca ter mostrado a mira.

Fica **fora**, e o motivo e o filtro funcionando: para recusar esse gesto o jogo
precisa de informacao que ele MESMO tem (um `bool` no primeiro quadro em que o
arrasto fica ativo). Um `startedAt` no `DragState` seria API maior para um
problema de um jogo so — e a porta nao carrega o relogio de ninguem.

## O casco depois da promocao

`platform-theforge-common` 0.12.0: o `forgeui` perdeu o estado e a fila, e
delega. `forgeui::DragState` e `forgeui::Drop` continuam existindo como **alias**,
e `forgeui::drag()`/`readDrop()` continuam sendo a ergonomia global das cenas.

**Nenhum jogo mudou uma linha** — exatamente como na task 27.

## Criterios de Aceite

- [x] `DragState`, `Drop`, `pushDown/pushUp/cancelDrag`, `drag()`, `readDrop()`
      na `cengine::input::Mouse`.
- [x] A posicao move o gesto em andamento, e so ele.
- [x] Soltar sem ter apertado aqui nao gera nada.
- [x] Fila com teto, descartando o NOVO (mesma politica das outras duas).
- [x] Suite da engine verde, com os casos dos DOIS jogos citando origem.
- [x] Casco delegando, com alias; nenhum jogo mudou.
- [x] Ledger e CHANGELOG atualizados.
