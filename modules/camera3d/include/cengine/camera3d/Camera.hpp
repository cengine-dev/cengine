#pragma once

#include <cstddef>

// cengine::camera3d (task 29): a metade MECANISMO da camera 3D — as matrizes de
// vista e de projecao, e a volta (um pixel vira ponto do mundo).
//
// Extraida do `vigil` (@ `e35f67a`, `src/vigil/app/Camera.h` mais a metade de
// camera do `ForgeMalha.h`), que a escreveu a mao, e do lab `diorama`, que a
// EXTRAI em vez de copiar (degrau 05). Duas evidencias reais, como manda o
// ADR 0002 — e o vigil esta pausado, o que pela **Emenda 1** continua valendo,
// pagando o pedagio de sempre: *a suite desta engine encarna o caso de uso
// dele*. Ver `tests/camera3d/CameraTest.cpp`.
//
// ## O que NAO sobe, e nunca sobe
//
// **Para onde a camera olha.** Seguir um corpo, look-ahead, limites do nivel,
// suavizacao — isso e FEEL de cada jogo, e e o mesmo corte da `camera2d`
// (task 23). Este modulo recebe uma `Orbit` pronta e devolve matrizes.
//
// **Renderizacao.** Nao ha `Renderer`, nao ha GPU, nao ha The-Forge. Sao
// matrizes; um teste que projeta e desprojeta o mesmo ponto fecha o circuito
// sem abrir janela nenhuma.
//
// ## A CONVENCAO, e ela e a parte que morde
//
// Toda matriz de camera vive numa convencao, e trocar uma peca produz **uma
// imagem plausivel e errada** — que e a pior classe de defeito. As deste modulo
// estao aqui, por escrito:
//
// | | |
// |---|---|
// | mundo | **+Y para cima**, destro (a convencao do glTF) |
// | vista | a camera olha para **-Z**; +X a direita, +Y para cima |
// | recorte | X e Y em [-1,1]; **profundidade Z em [0,1]** |
//
// **Profundidade 0..1, e nao -1..1.** E o que D3D12, Vulkan e Metal esperam;
// o -1..1 e do OpenGL, e nenhum consumidor deste ecossistema usa OpenGL.
//
// **DESTRO, e nao canhoto.** O dado que este ecossistema carrega vem de glTF,
// que e destro. Uma matriz canhota sobre malha destra desenha uma imagem
// ESPELHADA — que continua parecendo certa ate alguem comparar com o original.
// Consequencia para quem desenha: a face da frente do glTF e anti-horaria, e
// com estas matrizes ela continua anti-horaria na tela.
//
// ## Por que ORTOGRAFICA importa mais do que parece
//
// Projecao ortografica e AFIM: a mesma matriz vale para o plano inteiro, e
// inverte-la e conta fechada. Por isso o `unproject` e exato e barato, sem
// interseccao de raio com plano.
//
// O vigil registrou por escrito por que isso nao e detalhe: *"em perspectiva, o
// mesmo pixel vira pontos diferentes conforme a altura do corpo — e a mira
// passaria a depender de onde a camera esta"*. O `unproject` daqui aceita as
// duas, mas so a ortografica devolve a resposta que nao depende da altura.

namespace cengine::camera3d {

/// Ponto/vetor 3D. Sem operadores: este modulo nao e uma biblioteca de algebra
/// linear — e o vocabulario minimo para descrever camera. Mesma escolha do
/// `collision2d::Vec2`.
struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/// Matriz 4x4 em **coluna-maior**: `m[coluna * 4 + linha]`.
///
/// Coluna-maior porque e o que os `mat4` do The-Forge e do glTF usam, e porque
/// permite que o consumidor faca `memcpy` direto para o constant buffer — sem
/// transpor, que e onde se erra.
struct Mat4
{
    float m[16] = {};

    [[nodiscard]] static Mat4 identity();
};

/// A camera em ORBITA: ela olha para um alvo, de uma distancia, por um angulo.
///
/// E a forma minima que o vigil descobriu ser suficiente — e ele descobriu
/// depois de faltar. O relato foi *"o tablado nao tem profundidade, parece nao
/// estar inclinado"*, e o que faltava era o `yaw`.
struct Orbit
{
    /// Para onde a camera olha, em unidades de mundo.
    Vec3 target = {};

    /// Quanto ela esta longe do alvo.
    float distance = 10.0f;

    /// **O GIRO em torno do eixo vertical (+Y), em RADIANOS.**
    ///
    /// Sem ele, um quadrado do chao projeta num RETANGULO alinhado aos eixos —
    /// e retangulo alinhado e indistinguivel de desenho 2D chapado. A
    /// inclinacao existe e nao APARECE.
    ///
    /// Com 45 graus o quadrado vira losango e a grade vira diamantes, e e ai
    /// que o olho le "plano no espaco". A camera do Diablo 2 nao e so
    /// inclinacao: e inclinacao **mais giro**.
    float yaw = 0.0f;

    /// **A ELEVACAO acima do plano do chao, em RADIANOS.**
    ///
    ///     pi/2   de cima, a prumo — o chao nao encolhe
    ///     pi/6   (30 graus) a proporcao 2:1 do Diablo 2: `sin 30 = 0.5`,
    ///            entao o chao encolhe exatamente pela metade na vertical
    ///     0      de lado, e o chao vira uma linha (sem inversa)
    ///
    /// A prumo (`pitch = +-pi/2`) a base da vista degenera, e o modulo resolve
    /// pelo LIMITE: a orientacao continua sendo a que o `yaw` manda, sem salto.
    ///
    /// **Medida a partir do CHAO, e nao do zenite.** As duas convencoes
    /// existem no mundo real — o rig de captura do `diorama` usa a do zenite —,
    /// entao quem traduz de fora converte com `pi/2 - x` e o faz uma vez so, num
    /// lugar testado.
    float pitch = 0.0f;
};

/// Onde a camera esta, no mundo. Derivado da orbita.
///
/// Publico porque e o numero que um rig externo permite CONFERIR: o
/// `.txt` de cada captura do `diorama` guarda a posicao que o Blender calculou,
/// e comparar com esta e o que impede a traducao de rig ficar plausivel e
/// errada.
[[nodiscard]] Vec3 eye(const Orbit& orbit);

/// A matriz de VISTA: mundo -> espaco da camera (olhando para -Z).
[[nodiscard]] Mat4 view(const Orbit& orbit);

/// **Entrada degenerada devolve a matriz ZERADA**, e nao uma projecao plausivel.
///
/// Uma matriz zerada leva todo vertice a `w = 0`: nada desenha. E de proposito.
/// A alternativa -- grampear o parametro ruim num valor qualquer -- produziria
/// uma imagem, e uma imagem errada que parece certa e a classe de defeito que
/// este modulo inteiro existe para evitar. Tela vazia manda procurar a camera;
/// enquadramento estranho nao manda procurar nada.
///
/// (Nao ha `bool` de retorno porque nao ha, hoje, consumidor que saiba o que
/// fazer com a recusa; o dia em que houver, a assinatura muda com ele.)
[[nodiscard]] bool degenerada(const Mat4& m);

/// Projecao ORTOGRAFICA. `halfWidth` e a metade da largura visivel em unidades
/// de mundo; a altura sai de `aspect` (largura/altura).
///
/// Degenerada (ver acima) quando `halfWidth <= 0`, `aspect <= 0` ou
/// `farPlane <= nearPlane`.
[[nodiscard]] Mat4 orthographic(float halfWidth, float aspect, float nearPlane, float farPlane);

/// Projecao em PERSPECTIVA. `fovY` em radianos (angulo vertical total).
///
/// Degenerada (ver acima) quando `fovY` sai de `(0, pi)`, `aspect <= 0`,
/// `nearPlane <= 0` ou `farPlane <= nearPlane`.
[[nodiscard]] Mat4 perspective(float fovY, float aspect, float nearPlane, float farPlane);

/// Produto de matrizes: `multiply(projection, view)` da a `viewProj`.
[[nodiscard]] Mat4 multiply(const Mat4& a, const Mat4& b);

/// **A MIRA: um pixel vira ponto do mundo**, no plano horizontal `planeY`.
///
/// `screenX`/`screenY` em pixels, com a origem no canto SUPERIOR esquerdo — a
/// mesma convencao que as pontes de desenho deste ecossistema usam para o mouse
/// e para o texto. A inversao do Y acontece aqui dentro, uma vez.
///
/// E a razao de o cursor continuar mirando onde o olho ve: o jogo entrega ao
/// dominio um ponto de MUNDO, e nao um pixel.
///
/// Devolve `false` (sem tocar em `outWorld`) em tres casos, e os tres sao "nao ha
/// resposta", nunca "tome um numero grande":
///
///  1. **viewport zerada** — nao ha pixel que converter;
///  2. **raio paralelo ao plano** — camera exatamente na horizontal;
///  3. **o encontro fica ATRAS da camera** — mirar acima do horizonte em
///     perspectiva, ou pedir um `planeY` mais alto do que o olho. O ponto
///     existe e esta no plano pedido; ele so nao esta na frente de ninguem.
///     (Corrigido depois da 0.16.0: ate ali este caso devolvia `true`.)
[[nodiscard]] bool unproject(const Mat4& viewProj, float screenX, float screenY, float viewportW, float viewportH,
                             float planeY, Vec3& outWorld);

} // namespace cengine::camera3d
