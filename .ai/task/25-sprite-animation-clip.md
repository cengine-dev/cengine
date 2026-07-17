# 25 - Clip de animacao de sprite (frames sobre tempo)

- **Status:** estacionada (NAO implementar — gate ADR 0002 nao disparou:
  1 de 2 evidencias, e ha sinal de divergencia)
- **Prioridade:** baixa
- **Categoria:** Arquitetura (modulo opt-in, candidato `cengine::anim`?)
- **Registrada em:** 2026-07-17 (revisao pos-0.9.0, antes do 6o jogo)

## A candidata

Um "clip" de animacao dirigido pelo TEMPO: uma lista de frames + fps + loop, e
um cursor que avanca com `dt` e devolve o frame atual. Trocar de clip reseta o
cursor. E o nucleo do `PlayerAnimator` do mario-bros — a parte que NAO conhece
Idle/Walk/Jump.

## Evidencias (1/2)

- **mario-bros** (`src/mario/anim/PlayerAnimator.*`, task 03 do jogo): ciclo de
  frames sobre tempo, 1-frame para Idle/Jump, ciclo de 2 para Walk, troca de
  clip reseta para o frame zero. C++ puro, testado sem GPU (8 testes).

## Contra-evidencia (o sinal de divergencia, como na task 22)

- **spaceinvaders** anima SEM relogio: a pose do invasor deriva do passo da
  marcha (`World::animFrame()` = paridade de `m_steps`). Nao ha clip, nem fps,
  nem cursor — o "tempo" da animacao e o ritmo do proprio dominio. Se a engine
  tivesse um clip por tempo, o spaceinvaders NAO o usaria.
- breakout e asteroids nao tem animacao de frames (sprites estaticos /
  wireframe).

## O corte (se o gate um dia disparar)

- **Mecanismo (subiria):** clip = frames + fps + loop; cursor que avanca com
  `dt`; troca de clip reseta. Nenhum vocabulario de jogo.
- **Politica (fica no jogo):** QUAL clip tocar (Idle/Walk/Jump e a maquina de
  estados que escolhe), facing/espelho, o que e um "frame" na tabela de regioes
  do atlas (a tabela e do jogo, como decidido no forgesprite).

## Gate para comecar (ADR 0002)

Um 2o jogo com ciclo de frames dirigido pelo TEMPO, com o MESMO mecanismo
(discriminador do input: copias identicas, nao formas parecidas). Se o proximo
jogo animar por regra propria (como o spaceinvaders), e mais um voto de que
animar e politica — e esta task fecha como vetada.
