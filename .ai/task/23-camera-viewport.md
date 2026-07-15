# 23 - Camera / viewport (projecao mundo -> tela + culling)

- **Status:** estacionada - **1 de 2 evidencias** (mario-bros degrau 4 nasceu,
  2026-07-15). Gate nao disparado: falta um 2o consumidor com o MESMO modelo de
  projecao. Nao implementar ainda. Ver "Avaliacao do gate".
- **Prioridade:** baixa - so faz sentido depois de existir um nivel maior que a
  tela, e de um SEGUNDO consumidor com o mesmo modelo de camera.
- **Categoria:** Arquitetura / possivel modulo novo opt-in (ou fica no jogo).
- **Depende de:** nada estrutural. Depende de EVIDENCIA (consumidores reais).
- **Breaking:** nao. Nasceria como modulo opt-in (ex.: `cengine::camera2d`) ou
  simplesmente ficaria no jogo — a decisao e do gate.

## Por que esta task existe agora (sem evidencia)

O mario-bros foi escolhido, entre os plataformas, **pela fronteira da camera**:
e a unica lacuna arquitetural que nenhum jogo do ecossistema tocou. 8puzzle,
spaceinvaders, asteroids e breakout cabem TODOS numa tela — nenhum projetou
mundo -> tela nem fez culling. O mario e o primeiro cujo nivel PRECISA exceder a
janela (senao a camera nunca aparece).

Esta task nao manda construir nada: ela **registra a candidata** para o
aprendizado nao se perder entre os projetos, do mesmo jeito que a task 18 (scene
stack) ficou parada esperando o caso real.

## Avaliacao do gate (2026-07-15) - o mario ganhou camera, e ela ficou no jogo

O degrau 4 do mario-bros trouxe a **1a evidencia**: nivel 64 colunas (maior que a
tela), camera que rola suave seguindo o jogador, com culling. Ela NAO destravou a
promocao, por dois motivos:

**1. Uma evidencia nao sao duas (ADR 0002, criterio 2).** E o primeiro e unico
jogo do ecossistema com nivel maior que a tela. Contagem: UM.

**2. A camera ja NASCEU partida em duas metades, e so uma e candidata.** O
`mario::Camera` separou de proposito:
  - a **transformada mundo->janela + culling** — mecanismo puro, agnostico. E o
    que poderia subir.
  - o **seguimento** (ancora do foco, travar nos limites do nivel) — FEEL. O
    mario rola suave e horizontal; um metroidvania saltaria por sala; um jogo de
    nave centraria diferente. Promover um seguimento daria a engine uma opiniao
    sobre o genero.

Ou seja: mesmo com um 2o consumidor, so a transformada+culling subiria; o
seguimento de cada jogo fica no jogo. Enquanto houver so o mario, nem isso — a
copia (uma projecao por jogo) e o custo aceito.

**Evidencia 1/2 registrada:** mario-bros @ commit `4a8f825`,
`src/mario/camera/Camera.{h,cpp}` (a transformada + `visible()` de culling; o
`follow()` e o feel) e `ForgeGameScene::viewport/drawSprite` (a projecao subtrai
a camera). 7 testes de camera no jogo.

**Novo gate:** comecar quando um **2o** consumidor precisar de projecao/culling
com o MESMO modelo; o seguimento nunca sobe.

## Contexto

Hoje a projecao "mundo -> pixels de tela" vive em cada jogo (ex.: o `toScreen`
das cenas Forge do breakout, o `viewport` da cena do mario no degrau 2). Enquanto
o mundo cabe na tela, isso e so um enquadramento fixo (escala + centralizacao),
e nao ha nada a promover.

A camera vira mecanismo de verdade quando o nivel excede a janela: uma
transformada mundo -> tela com **deslocamento** (a rolagem), mais **culling** do
que esta fora do enquadramento. A transformada e pura e reutilizavel; o
comportamento de COMO a camera se move nao e.

## Objetivo (SE o gate disparar)

Oferecer a **transformada mundo -> tela pura** (talvez com culling de retangulos
visiveis), sem decidir COMO a camera segue o alvo:

```cpp
namespace cengine::camera2d {
// Converte um ponto/retangulo do mundo para a tela dado o alvo da camera e o
// tamanho da viewport. Puro: nao decide para onde a camera olha.
struct Camera { Vec2 target; float zoom; /* ... */ };
[[nodiscard]] Vec2 worldToScreen(const Camera&, Vec2 world, Vec2 viewport);
[[nodiscard]] bool visible(const Camera&, Aabb worldRect, Vec2 viewport);
}
```

O jogo continua dono de PARA ONDE a camera olha (seguir o jogador, look-ahead,
deadzone, travar nos limites do nivel) — isso e feel, e feel e politica.

## Fora do Escopo (o corte mecanismo x politica)

- **O comportamento de seguimento.** Follow suave, look-ahead na direcao do
  movimento, deadzone, travar nas bordas do nivel, "camera de sala" que salta —
  cada sub-genero tem o seu (foi por isso que o mario, e nao um metroidvania,
  foi escolhido: para ter UM modelo de camera claro). Promover um seguimento
  seria dar a engine uma opiniao sobre o genero do jogo.
- Parallax, multiplas camadas, split-screen, shake, zoom dinamico.
- Ownership de entidades ou do alvo.

## Criterios Para Comecar

Nao implementar imediatamente. Comecar apenas quando:

- existir um consumidor real com nivel > tela e rolagem (mario degrau 4) — 1a
  evidencia; **e**
- um **segundo** consumidor precisar do MESMO modelo de projecao/culling (nao de
  outro modelo de camera). Se os modelos divergirem, a transformada ate pode
  subir, mas o SEGUIMENTO fica no jogo.

## Criterios de Aceite (quando/se subir)

- [ ] Transformada mundo -> tela pura (e culling), opt-in, testavel sem GPU.
- [ ] **Regra de proveniencia:** testes de consumidor real citam a origem
      (repo @ commit, arquivo, linha) e transcrevem os valores do jogo.
- [ ] A engine NAO decide para onde a camera olha: nenhum seguimento/deadzone
      embutido.
- [ ] O modelo de tela unica (enquadramento fixo) continua trivial para jogos
      que nao rolam.

## Riscos

- Promover o SEGUIMENTO (feel) de um jogo como se fosse universal.
- Construir a camera antes de existir o primeiro nivel que rola (especulacao — o
  que a ADR 0002 barra). Por isso esta task nasce ESTACIONADA, com 0 evidencias.

## Relacionado

- ADR 0002 - criterio de promocao (2 evidencias, mecanismo x politica).
- mario-bros, degrau 4 (a fazer) - sera a evidencia 1/2; a fronteira-titulo do
  jogo.
- Task 18 (scene stack) - outra candidata que ficou estacionada esperando o caso
  real; mesmo padrao.
