# 27 - Mouse como porta de input

- **Status:** ESTACIONADA — **1/2.** O gate NAO disparou.
- **Categoria:** Arquitetura (porta, irma da task 20)
- **Registrada em:** 2026-07-28 (revisao pos-Bulwark, 9o jogo)

## A candidata

O vocabulario de ponteiro como porta da engine, do mesmo jeito que a task 20
subiu o vocabulario de teclado: um tipo puro que diz "houve um clique deste
botao nesta posicao", sem saber de The-Forge nem de Win32.

Hoje ele vive no casco, em `platform-theforge-common` 0.6.0
(`src/TheForgeCommon/ForgeUi.h:65-131`): `MouseButton`, `MouseClick{button,x,y}`,
`mouseX()`, `mouseY()`, `readMouseClick()`.

## Evidencia (1/2)

- **bulwark @ 4554c8d** (degrau 05), unico consumidor. Nenhum outro jogo do
  ecossistema le mouse — conferido por busca em todos os `src/`.

## Por que espera, e por que a espera tem numero

**O discriminador e historico, e e o melhor que temos:** o enum `Key` so subiu
na **4a copia identica** (task 20, 0.8.0). O mouse tem uma. Promover agora
seria dar a engine uma opiniao sobre ponteiro derivada de um unico jogo — e
justamente um jogo de grade, que provavelmente enviesaria a API para "clique
em celula".

O mouse viver no `forgeui` com vocabulario local **e o estado correto**, nao um
provisorio incomodo: foi assim que o teclado viveu por quatro jogos, e foi
essa espera que produziu uma porta boa.

## O que ja se aprendeu, e que deve sobreviver a promocao

Tres decisoes do degrau 05 do Bulwark que valem para qualquer porta futura:

1. **Posicao e ESTADO, clique e EDGE.** Onde o ponteiro esta pode ser lido a
   qualquer momento; um clique acontece uma vez e precisa de fila, senao dois
   leitores no mesmo quadro veem coisas diferentes.
2. **O clique carrega a posicao DO MOMENTO em que aconteceu** — nao a atual. O
   ponteiro anda entre o aperto e o quadro em que a cena le.
3. **A camada de baixo entrega PIXELS e nao sabe o que e celula.** A traducao
   e do jogo (ver task 26).

## O que destrava

Um **segundo jogo com ponteiro**. Ele fecha esta candidata junto com a 26
(grade celula <-> pixel) e com o limite de roteamento espacial da task 18 — as
tres sao a mesma aposta, declarada em
`bulwark/.ai/task/09-revisao-candidatas.md`.

E vale registrar o encadeamento: **se o mouse subir, o limite da 18 precisa
subir junto.** Uma engine que entrega clique mas nao sabe dizer qual camada o
recebeu entrega meio mecanismo — e o `IScene::input()` hoje nao reporta
consumo.
