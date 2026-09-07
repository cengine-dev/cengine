# 29 - camera3d: a metade de MUNDO ganha a terceira dimensao

- **Status:** **DONE (0.16.0)** — 2026-09-06. Extracao autorizada, pela formula "o proximo consumidor
  EXTRAI, nao copia" (precedente: bulwark, 9o jogo, 7 releases em 8 commits).
- **Categoria:** Candidata vinda de um consumidor (metade de MUNDO)
- **Registrada em:** 2026-09-03, ao abrir o lab `diorama`

## A candidata

A camera 3D como **mecanismo puro**: as matrizes de vista e de projecao, e a
volta -- de um ponto da TELA para um ponto do MUNDO.

Nada de The-Forge, nada de GPU, nada de vocabulario de jogo. Matematica que a
suite da cengine testa sozinha, como `camera2d` e `grid2d` ja sao testados.

```cpp
namespace cengine::camera3d {

struct Orbit {                   // o que o Vigil descobriu ser o minimo
    Vec3  target;                // para onde olha
    float distance;
    float yaw;                   // o GIRO -- e ele que faltava
    float pitch;                 // a inclinacao
};

Mat4 view(const Orbit&);
Mat4 orthographic(float halfWidth, float aspect, float near, float far);
Mat4 perspective(float fovY, float aspect, float near, float far);

// A mira: tela -> mundo. Afim se inverte com conta fechada.
Vec3 unproject(const Mat4& viewProj, float screenX, float screenY,
               float viewportW, float viewportH, float planeY);

} // namespace cengine::camera3d
```

## Evidencia (1/2), e por que a extracao ja esta autorizada

**Consumidor 1 -- vigil @ `e35f67a`, escrito a mao**, degrau 24:
`src/vigil/app/Camera.h` mais a metade de camera do
`src/platform/theforge/src/VigilForge/scene/ForgeMalha.h`. Consumidor **pausado**,
o que pela **Emenda 1 do ADR 0002** continua valendo como evidencia -- e paga o
pedagio de sempre: *a suite da cengine deve encarnar o caso de uso dele*.

**Consumidor 2 -- `diorama`, degrau 05.** Ele **extrai**, nao copia. E a mesma
formula que fechou cinco ciclos completos (a ultima foi o drag, `0.15.0`), e ela
existe justamente por causa do ponto cego conhecido do metodo: *"enviesado para
EXTRACAO, e por isso estruturalmente cego para abstracao que so paga se desenhada
antes de existir"*.

## Os tres criterios do ADR 0002

1. **Mecanismo puro?** Sim. `view`, `projection`, `unproject`. Nenhum `player`,
   nenhum `horda`, nenhum `taberneiro`. O nome mais especifico e `Orbit`, que e
   geometria.
2. **Duas evidencias reais?** Uma escrita a mao (vigil) e uma extraindo
   (diorama). O padrao exato das cinco promocoes anteriores.
3. **Testavel dentro da cengine?** Sim, e **melhor** que a maioria: sao matrizes.
   Um teste que projeta e desprojeta o mesmo ponto e fecha o circuito nao precisa
   de janela nenhuma.

## O que o Vigil ja MEDIU, e que deve sobreviver a promocao

Tres fatos duros, e nenhum deles foi suposto:

1. **A camera precisa de GIRO, e nao so de inclinacao.** Sem giro, um quadrado do
   chao projeta num retangulo alinhado -- indistinguivel de 2D chapado. 45 graus o
   transforma em losango. *Diablo 2 e inclinacao MAIS giro.* **Gira primeiro,
   achata depois** -- a ordem importa, e uma API que aceite so `pitch` esta errada.
2. **Ortografica por causa da MIRA.** O dominio recebe o ponteiro em MUNDO;
   projecao afim se inverte com conta fechada. Perspectiva faria o mesmo pixel
   virar pontos diferentes conforme a altura. Por isso `orthographic` e
   `perspective` sao irmas, e nao uma flag.
3. **A classe de defeito que a camera revelou** -- e esta e a mais importante:

   > O giro expos quatro lugares que misturavam **MUNDO com TELA** (a mira, o
   > taberneiro, o traco do golpe, os circulos de alcance). Todos funcionavam
   > **por acaso** enquanto o giro era zero. O dos circulos **mentia sobre a
   > REGRA**: raio de mundo desenhado redondo diz que a aura alcanca mais do que
   > alcanca.
   >
   > *Desenhar e acertar tem que usar os mesmos numeros* deixou de ser
   > recomendacao e virou regra dura.

   Isso e argumento **a favor** de a camera ser da engine: enquanto a conversao
   mora no jogo, cada jogo tem a sua chance de errar por acaso.

## O pedagio da Emenda 1 (evidencia congelada)

A suite da cengine deve encarnar o caso do Vigil: **a camera Diablo 2 com giro de
45 graus, um ponto do chao, e a mira voltando ao mesmo ponto.** Se o teste nao
reproduzir o losango, a promocao nao esta paga.

## O que esta task NAO promove, e por que

- **Nada de renderizacao.** Pipeline, depth buffer, shader, material e skinning
  sao do casco (`platform-theforge-common`, tasks 09-14). O ADR 0001 e explicito:
  a cengine **nao escolhe biblioteca grafica**.
- **Nada de grafo de cena.** Zero evidencia. Seria especulacao, que e o que o
  ADR 0002 existe para impedir.
- **`transform3d` separado?** Nao ate haver caso. Se a matriz de modelo por
  objeto aparecer escrita a mao em dois lugares, ai sim -- e ai e a task 30.

## A pergunta que este ciclo responde, e que estava aberta desde o Cue

O retrato do ecossistema registra que a metade de MUNDO responde a **quatro
perguntas independentes**, e que "continuo" nunca foi o eixo certo. O `diorama`
faz uma quinta, que nenhum dos quatorze jogos fez:

> **O mundo tem TRES eixos?**

E ela e a primeira pergunta da metade de mundo que nao tem resposta 2D nenhuma.

## Fechada em 2026-09-06 — o que ficou diferente do esboco

**1. `Orbit::pitch` e medido do CHAO, e nao do zenite.** As duas convencoes
existem: o vigil mede do chao (*"90 de cima, a prumo"*), o rig de captura do
Blender mede do zenite. Escolhida a do vigil, que e a evidencia 1 — e quem vem
de fora converte com `pi/2 - x`, **uma vez so, num lugar testado**
(`diorama/src/diorama/app/RigDoBlender`).

**2. O modulo declara os proprios `Vec3`/`Mat4`.** A cengine nao tinha nenhum
tipo 3D. Sao o vocabulario minimo, sem operadores — a mesma escolha do
`collision2d::Vec2` (*"nao e uma biblioteca de algebra linear"*). `Mat4` e
coluna-maior porque o consumidor faz `memcpy` direto para o constant buffer, e
transpor e onde se erra.

**3. A CONVENCAO virou parte da entrega.** Mundo +Y para cima e **destro**
(glTF), vista olhando -Z, profundidade de recorte em **[0,1]**. Esta escrito no
cabecalho e ha um teste so para a mao (`ACenaNaoSaiESPELHADA`), porque matriz
canhota sobre malha destra desenha a cena espelhada — e espelhado continua
parecendo certo ate haver com o que comparar.

O consumidor 2 mediu isso na pratica: ele vinha usando `lookAtLH`/
`perspectiveLH` do The-Forge e precisou de `FRONT_FACE_CW` para o descarte de
face funcionar. **Aquele `CW` era o sintoma da cena espelhada**; com as matrizes
destras daqui, o `CCW` nativo do glTF volta a valer.

**4. `unproject` devolve `bool`.** O esboco devolvia `Vec3`. Camera na
horizontal nao tem resposta — o chao inteiro projeta numa linha, e todo pixel
dela corresponde a infinitos pontos. O vigil devolvia o centro; aqui a resposta
e "nao", e quem chama decide.

## Os tres criterios do ADR 0002 — cumpridos

| | |
|---|---|
| mecanismo puro | sem `Renderer`, sem GPU, sem vocabulario de jogo |
| duas evidencias | vigil escreveu a mao; diorama extraiu |
| testavel na suite | **15 testes**, nenhum abre janela |

Quatro dos 15 sao o pedagio da **Emenda 1** (consumidor pausado): transcrevem os
numeros do `vigil/src/vigil/app/Camera.h` com a origem citada — o achatamento
2:1 em 30 graus, o retangulo alinhado sem giro, o losango com giro de 45, e a
camera a prumo que nao achata nada.

Suite completa apos a entrada: **167 testes, todos passando**.

## Duas ressalvas registradas na revisao (2026-09-06)

Sairam de uma pergunta do dono: *"algum ponto que ficou mal arquitetado?"*.
Nenhuma das duas pede mudanca de codigo hoje; as duas pedem que a condicao de
reabertura esteja escrita.

### 1. A engine agora tem OPINIAO sobre espaco de recorte

O ADR 0001 diz que a cengine nao escolhe biblioteca grafica, e a `camera3d`
cumpre: nao ha `Renderer`, nao ha GPU, nao ha dependencia. **Mas ela fixa a
profundidade de recorte em [0,1] e a mao como DESTRA**, e isso e uma decisao com
forma de API grafica: o [0,1] e de D3D12/Vulkan/Metal, e o OpenGL usa [-1,1].

E defensavel — convencao nao e dependencia, e esta escrita no cabecalho, que e
mais do que a maioria das engines faz. Mas e a coisa mais proxima da linha que a
cengine ja teve, e o registro serve para nao ser descoberta como surpresa.

> **Condicao de reabertura:** um consumidor com backend OpenGL. Ai a task 29
> volta, e a saida provavel e uma variante de `orthographic`/`perspective` com o
> intervalo de profundidade como parametro — nao um segundo modulo.

### 2. O `unproject` foi promovido com **uma evidencia e meia**

O ADR 0002 pede duas evidencias reais. Para a `Orbit` e as matrizes ha duas: o
vigil escreveu a mao e o diorama extraiu.

**Para o `unproject`, nao.** O vigil usava a mira dele (`paraChao`), mas o
`diorama` **nao chama** `unproject` em lugar nenhum — o lab nao tem interacao de
ponteiro. O que sustenta a funcao hoje sao o consumidor 1 e os testes.

Isso nao invalida a promocao (a Emenda 1 vale para o vigil, e a suite encarna o
caso dele), mas e menos do que a regra pede, e eu nao sinalizei na hora.

> **Condicao de revisao:** se ate o fim do plano do `diorama` (degrau 09) nenhum
> consumidor chamar `unproject`, ele vira candidato a SAIR do modulo — a mesma
> disciplina que ja fechou tasks em negativo (18, 22b, broadphase). Codigo com
> uma evidencia so nao fica de graca por ser util em tese.
