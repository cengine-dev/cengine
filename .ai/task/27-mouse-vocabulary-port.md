# 27 - Mouse como porta de input

- **Status:** **PROMOVIDA (0.14.0, 2026-07-30)** — `cengine::input::Mouse`,
  com o Tactics como segundo consumidor e validador.
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

## O que destravou (2026-07-30)

O segundo jogo com ponteiro veio: **tactics, degrau 06**. E o argumento nao foi
a contagem — foi que ele **usou a forma do primeiro sem mudar nada**. Quando o
segundo consumidor nao pede nenhuma alteracao na API, o que se esta extraindo
ja e a forma certa; e o sinal mais forte que este filtro consegue dar.

Subiu o vocabulario inteiro, sem invencao: `MouseButton` (so os dois botoes com
consumidor), `MouseClick{button,x,y}` e a classe `Mouse` com posicao (estado) e
fila de cliques (edges). Mesmo desenho do `Keyboard` da task 20, e de proposito:
duas portas de input com formatos diferentes seriam duas coisas para aprender.

O casco (common 0.9.0) passou a guardar a instancia e delegar, exatamente como
faz com o teclado desde a 0.8.0; `forgeui::MouseButton` e `forgeui::MouseClick`
continuam existindo como ALIAS, entao nenhum jogo precisou mudar uma linha.

### A ressalva sobre a task 18, corrigida

Estava escrito aqui: *"se o mouse subir, o limite da 18 precisa subir junto"*.
**Nao precisa, e o degrau 07 do tactics mostrou por que.** Subir o vocabulario
de ponteiro nao basta para a pilha rotear por posicao: falta ainda a cascata
(reportar consumo) E resolver que o input e PUXADO de uma fila global — numa
cascata, a primeira camada a olhar comeria o clique antes de decidir que nao era
dela. A 18 depende desta task, mas o inverso nao vale: esta fecha sozinha.
