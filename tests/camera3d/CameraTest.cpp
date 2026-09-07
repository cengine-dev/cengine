#include <gtest/gtest.h>

#include <cmath>

#include <cengine/camera3d/Camera.hpp>

// A metade MECANISMO da camera 3D (task 29): vista, projecao e a mira inversa.
// O seguimento (para onde a camera olha) NUNCA sobe — o mesmo corte da task 23.
//
// **A ultima secao e o pedagio da Emenda 1 do ADR 0002.** O consumidor 1
// (`vigil`) esta pausado, e por isso a suite desta engine tem de ENCARNAR o caso
// de uso dele: os numeros de la estao transcritos, com a origem citada.

using cengine::camera3d::Mat4;
using cengine::camera3d::Orbit;
using cengine::camera3d::Vec3;
using cengine::camera3d::degenerada;
using cengine::camera3d::eye;
using cengine::camera3d::multiply;
using cengine::camera3d::orthographic;
using cengine::camera3d::perspective;
using cengine::camera3d::unproject;
using cengine::camera3d::view;

namespace {

constexpr float kPi = 3.14159265358979323846f;

/// Projeta um ponto do mundo para NDC. Nao esta no modulo de proposito: quem
/// desenha usa a GPU para isso. Aqui ele existe para fechar o circuito com o
/// `unproject`, que e a unica prova que nao depende de olhar a tela.
Vec3 projetar(const Mat4& viewProj, const Vec3& mundo)
{
    const float* m = viewProj.m;
    const float  x = m[0] * mundo.x + m[4] * mundo.y + m[8] * mundo.z + m[12];
    const float  y = m[1] * mundo.x + m[5] * mundo.y + m[9] * mundo.z + m[13];
    const float  z = m[2] * mundo.x + m[6] * mundo.y + m[10] * mundo.z + m[14];
    const float  w = m[3] * mundo.x + m[7] * mundo.y + m[11] * mundo.z + m[15];
    return { x / w, y / w, z / w };
}

/// NDC -> pixel, com a origem no canto SUPERIOR esquerdo (a convencao das
/// pontes de desenho deste ecossistema).
void paraPixel(const Vec3& ndc, const float w, const float h, float& sx, float& sy)
{
    sx = (ndc.x + 1.0f) * 0.5f * w;
    sy = (1.0f - ndc.y) * 0.5f * h;
}

} // namespace

// =============================================================================
// eye — a orbita vira posicao
// =============================================================================

TEST(Camera3dTest, PitchDeNoventaGrausPoeACameraAPrumo)
{
    const Orbit o{ .target = { 1.0f, 2.0f, 3.0f }, .distance = 10.0f, .yaw = 0.7f, .pitch = kPi * 0.5f };

    const Vec3 e = eye(o);
    EXPECT_NEAR(e.x, 1.0f, 1e-4f);
    EXPECT_NEAR(e.y, 12.0f, 1e-4f); // 2 + 10, direto acima
    EXPECT_NEAR(e.z, 3.0f, 1e-4f);
}

TEST(Camera3dTest, PitchZeroPoeACameraNoPlanoDoAlvo)
{
    const Orbit o{ .target = {}, .distance = 5.0f, .yaw = 0.0f, .pitch = 0.0f };

    const Vec3 e = eye(o);
    EXPECT_NEAR(e.y, 0.0f, 1e-5f);
    EXPECT_NEAR(e.z, 5.0f, 1e-5f); // yaw 0 = atras, no +Z
}

TEST(Camera3dTest, AOrbitaMantemADistanciaSejaQualForOAngulo)
{
    for (float pitch = 0.0f; pitch < kPi * 0.5f; pitch += 0.31f)
    {
        for (float yaw = 0.0f; yaw < 2.0f * kPi; yaw += 0.71f)
        {
            const Orbit o{ .target = { -2.0f, 4.0f, 7.0f }, .distance = 12.0f, .yaw = yaw, .pitch = pitch };
            const Vec3  e = eye(o);
            const float dx = e.x - o.target.x;
            const float dy = e.y - o.target.y;
            const float dz = e.z - o.target.z;
            EXPECT_NEAR(std::sqrt(dx * dx + dy * dy + dz * dz), 12.0f, 1e-3f);
        }
    }
}

// =============================================================================
// A CONVENCAO — trocar uma peca produz imagem plausivel e errada
// =============================================================================

TEST(Camera3dTest, ProfundidadeVaiDeZeroAUmEntreOsPlanos)
{
    const Orbit o{ .target = {}, .distance = 10.0f, .yaw = 0.0f, .pitch = 0.0f };
    const Mat4  vp = multiply(perspective(kPi / 3.0f, 16.0f / 9.0f, 1.0f, 100.0f), view(o));

    // Um ponto no plano PROXIMO da camera (a 1 unidade dela, no eixo de visada)
    // e outro no distante.
    const Vec3 perto = projetar(vp, { 0.0f, 0.0f, 9.0f });   // 1 a frente do olho (z=10)
    const Vec3 longe = projetar(vp, { 0.0f, 0.0f, -90.0f }); // 100 a frente

    EXPECT_NEAR(perto.z, 0.0f, 1e-3f);
    EXPECT_NEAR(longe.z, 1.0f, 1e-3f);
}

TEST(Camera3dTest, OrtograficaTambemVaiDeZeroAUm)
{
    const Orbit o{ .target = {}, .distance = 10.0f, .yaw = 0.0f, .pitch = 0.0f };
    const Mat4  vp = multiply(orthographic(5.0f, 1.0f, 1.0f, 100.0f), view(o));

    EXPECT_NEAR(projetar(vp, { 0.0f, 0.0f, 9.0f }).z, 0.0f, 1e-4f);
    EXPECT_NEAR(projetar(vp, { 0.0f, 0.0f, -90.0f }).z, 1.0f, 1e-4f);
}

TEST(Camera3dTest, ACenaNaoSaiESPELHADA)
{
    // **O teste que pega a troca de mao.** Uma matriz canhota sobre dado destro
    // desenha uma imagem espelhada — que continua parecendo certa ate alguem
    // comparar com o original. Aqui: com a camera atras (yaw=0, no +Z) olhando
    // para a origem, um ponto no +X do mundo tem de cair a DIREITA da tela.
    const Orbit o{ .target = {}, .distance = 10.0f, .yaw = 0.0f, .pitch = 0.0f };
    const Mat4  vp = multiply(perspective(kPi / 3.0f, 1.0f, 0.1f, 100.0f), view(o));

    EXPECT_GT(projetar(vp, { 1.0f, 0.0f, 0.0f }).x, 0.0f);
    EXPECT_LT(projetar(vp, { -1.0f, 0.0f, 0.0f }).x, 0.0f);
    // E o +Y do mundo sobe na tela (NDC cresce para cima).
    EXPECT_GT(projetar(vp, { 0.0f, 1.0f, 0.0f }).y, 0.0f);
}

// =============================================================================
// unproject — a mira, e ela e exata na ortografica
// =============================================================================

TEST(Camera3dTest, MiraFechaOCircuitoNaOrtografica)
{
    const Orbit o{ .target = { 2.0f, 0.0f, -1.0f }, .distance = 12.0f, .yaw = kPi / 4.0f, .pitch = kPi / 6.0f };
    const Mat4  vp = multiply(orthographic(7.0f, 800.0f / 600.0f, 0.1f, 100.0f), view(o));

    for (const Vec3 alvo : { Vec3{ 0.0f, 0.0f, 0.0f }, Vec3{ 3.0f, 0.0f, -2.0f }, Vec3{ -1.5f, 0.0f, 4.0f } })
    {
        float sx = 0.0f, sy = 0.0f;
        paraPixel(projetar(vp, alvo), 800.0f, 600.0f, sx, sy);

        Vec3 volta{};
        ASSERT_TRUE(unproject(vp, sx, sy, 800.0f, 600.0f, 0.0f, volta));
        EXPECT_NEAR(volta.x, alvo.x, 1e-3f);
        EXPECT_NEAR(volta.y, 0.0f, 1e-5f);
        EXPECT_NEAR(volta.z, alvo.z, 1e-3f);
    }
}

TEST(Camera3dTest, MiraRespeitaAAlturaDoPlano)
{
    const Orbit o{ .target = {}, .distance = 10.0f, .yaw = 0.3f, .pitch = kPi / 5.0f };
    const Mat4  vp = multiply(orthographic(6.0f, 4.0f / 3.0f, 0.1f, 50.0f), view(o));

    Vec3 noChao{}, maisAlto{};
    ASSERT_TRUE(unproject(vp, 400.0f, 300.0f, 800.0f, 600.0f, 0.0f, noChao));
    ASSERT_TRUE(unproject(vp, 400.0f, 300.0f, 800.0f, 600.0f, 2.0f, maisAlto));

    EXPECT_NEAR(noChao.y, 0.0f, 1e-5f);
    EXPECT_NEAR(maisAlto.y, 2.0f, 1e-5f);
    // O MESMO pixel da pontos diferentes em alturas diferentes — e por isso que
    // o vigil escolheu ortografica e um plano fixo para a mira.
    EXPECT_GT(std::fabs(maisAlto.z - noChao.z), 1e-3f);
}

TEST(Camera3dTest, CameraNaHorizontalNaoTemResposta)
{
    // `pitch = 0`: o chao inteiro projeta numa linha, e todo pixel dela
    // corresponde a infinitos pontos. Devolver `false` e a unica alternativa a
    // inventar um alvo.
    const Orbit o{ .target = {}, .distance = 10.0f, .yaw = 0.0f, .pitch = 0.0f };
    const Mat4  vp = multiply(orthographic(5.0f, 1.0f, 0.1f, 50.0f), view(o));

    Vec3 fora{};
    EXPECT_FALSE(unproject(vp, 400.0f, 300.0f, 800.0f, 600.0f, 0.0f, fora));
}

TEST(Camera3dTest, MiraAcimaDoHorizonteNaoTemResposta)
{
    // Em PERSPECTIVA, um pixel na parte de cima da tela aponta para o ceu: o
    // raio so encontra o plano do chao ATRAS da camera. O ponto existe, e nao
    // serve para nada -- devolve-lo seria mirar num alvo que esta as costas de
    // quem clicou.
    //
    // Ate a 0.16.0 este teste falhava: `unproject` devolvia `true`.
    const Orbit o{ .target = {}, .distance = 10.0f, .yaw = 0.0f, .pitch = kPi / 12.0f };
    const Mat4  vp = multiply(perspective(1.0f, 800.0f / 600.0f, 0.1f, 100.0f), view(o));

    Vec3 fora{};
    EXPECT_FALSE(unproject(vp, 400.0f, 4.0f, 800.0f, 600.0f, 0.0f, fora));

    // E o mesmo pixel, mirando um plano que a camera ainda VE, continua valendo:
    // a guarda nova nao pode ter comido o caso bom.
    Vec3 dentro{};
    EXPECT_TRUE(unproject(vp, 400.0f, 560.0f, 800.0f, 600.0f, 0.0f, dentro));
}

TEST(Camera3dTest, MiraNumPlanoACIMADaCameraNaoTemResposta)
{
    // A camera esta a 5 unidades do chao (`sin(pi/6) * 10`); pedir o plano
    // `y = 20` e pedir um encontro que fica para tras.
    const Orbit o{ .target = {}, .distance = 10.0f, .yaw = 0.4f, .pitch = kPi / 6.0f };
    const Mat4  vp = multiply(orthographic(6.0f, 4.0f / 3.0f, 0.1f, 50.0f), view(o));

    Vec3 fora{};
    EXPECT_FALSE(unproject(vp, 400.0f, 300.0f, 800.0f, 600.0f, 20.0f, fora));
}

TEST(Camera3dTest, ViewportZeradaNaoDivideePorZero)
{
    const Mat4 vp = Mat4::identity();
    Vec3       fora{};
    EXPECT_FALSE(unproject(vp, 10.0f, 10.0f, 0.0f, 600.0f, 0.0f, fora));
}

// =============================================================================
// o POLO — a prumo, a base degenera e o limite tem de valer
// =============================================================================

TEST(Camera3dTest, ACameraAPrumoAindaRESPEITAOGiro)
{
    // A prumo o produto vetorial que monta a base degenera. Isso NAO libera a
    // escolha: fora do polo a base depende do yaw, entao no polo ela tem de
    // continuar dependendo -- senao a imagem SALTA ao cruzar 90 graus.
    //
    // Ate a 0.16.0 este teste falhava: a base caia numa constante e as duas
    // matrizes abaixo saiam identicas.
    const Orbit semGiro{ .target = {}, .distance = 8.0f, .yaw = 0.0f, .pitch = kPi * 0.5f };
    const Orbit comGiro{ .target = {}, .distance = 8.0f, .yaw = kPi * 0.5f, .pitch = kPi * 0.5f };

    const Mat4 a = view(semGiro);
    const Mat4 b = view(comGiro);

    float maiorDiferenca = 0.0f;
    for (int i = 0; i < 16; ++i)
    {
        maiorDiferenca = std::fmax(maiorDiferenca, std::fabs(a.m[i] - b.m[i]));
    }
    EXPECT_GT(maiorDiferenca, 0.9f) << "girar a camera a prumo nao mudou nada na vista";
}

TEST(Camera3dTest, ACameraNaoSALTAAoChegarNoPolo)
{
    // O teste que fecha o anterior: nao basta o polo depender do yaw, ele tem de
    // depender do MESMO jeito que a vizinhanca. Um decimo de grau antes do polo
    // e o polo tem de dar quase a mesma matriz.
    const float quaseLa = kPi * 0.5f - 0.0001f;
    const Orbit perto{ .target = { 1.0f, 0.0f, -2.0f }, .distance = 8.0f, .yaw = 0.7f, .pitch = quaseLa };
    const Orbit noPolo{ .target = { 1.0f, 0.0f, -2.0f }, .distance = 8.0f, .yaw = 0.7f, .pitch = kPi * 0.5f };

    const Mat4 a = view(perto);
    const Mat4 b = view(noPolo);

    for (int i = 0; i < 16; ++i)
    {
        EXPECT_NEAR(a.m[i], b.m[i], 1e-3f) << "descontinuidade no elemento " << i;
    }
}

// =============================================================================
// as PROJECOES recusam entrada degenerada em vez de inventar uma camera
// =============================================================================

TEST(Camera3dTest, ACameraEmCIMADoAlvoNaoTemVISTA)
{
    // `distance = 0` poe o olho no alvo, e ai nao ha direcao para onde olhar. A
    // base degenera, e a versao anterior devolvia uma matriz SINGULAR calada.
    EXPECT_TRUE(degenerada(view(Orbit{ .target = { 1.0f, 2.0f, 3.0f }, .distance = 0.0f })));

    // E o caso bom continua bom: a guarda nova nao pode ter comido a vizinhanca.
    EXPECT_FALSE(degenerada(view(Orbit{ .target = { 1.0f, 2.0f, 3.0f }, .distance = 0.01f })));
}

TEST(Camera3dTest, OrtograficaRecusaEntradaDegenerada)
{
    EXPECT_TRUE(degenerada(orthographic(0.0f, 1.0f, 0.1f, 50.0f)));   // largura zero
    EXPECT_TRUE(degenerada(orthographic(-3.0f, 1.0f, 0.1f, 50.0f)));  // largura negativa
    EXPECT_TRUE(degenerada(orthographic(6.0f, 0.0f, 0.1f, 50.0f)));   // janela sem altura
    EXPECT_TRUE(degenerada(orthographic(6.0f, 1.0f, 50.0f, 50.0f)));  // near == far
    EXPECT_TRUE(degenerada(orthographic(6.0f, 1.0f, 60.0f, 50.0f)));  // far atras do near

    EXPECT_FALSE(degenerada(orthographic(6.0f, 4.0f / 3.0f, 0.1f, 50.0f)));
}

TEST(Camera3dTest, PerspectivaRecusaEntradaDegenerada)
{
    EXPECT_TRUE(degenerada(perspective(0.0f, 1.0f, 0.1f, 50.0f)));   // abertura zero
    EXPECT_TRUE(degenerada(perspective(kPi, 1.0f, 0.1f, 50.0f)));    // abertura de meia volta
    EXPECT_TRUE(degenerada(perspective(1.0f, 0.0f, 0.1f, 50.0f)));   // janela sem altura
    EXPECT_TRUE(degenerada(perspective(1.0f, 1.0f, 0.0f, 50.0f)));   // near no olho
    EXPECT_TRUE(degenerada(perspective(1.0f, 1.0f, 50.0f, 50.0f)));  // near == far

    EXPECT_FALSE(degenerada(perspective(1.05f, 4.0f / 3.0f, 0.1f, 60.0f)));
}

TEST(Camera3dTest, MatrizDegeneradaNAODESENHANADA)
{
    // Por que a matriz zerada e a resposta certa: ela nao produz uma imagem
    // torta, produz `w = 0` -- e `w = 0` e ausencia de imagem. O sintoma manda
    // procurar a camera, em vez de deixar o observador achar que o enquadramento
    // e que ficou esquisito.
    const Mat4 zerada = orthographic(0.0f, 1.0f, 0.1f, 50.0f);

    const Vec3  mundo{ 1.0f, 2.0f, 3.0f };
    const float w = zerada.m[3] * mundo.x + zerada.m[7] * mundo.y + zerada.m[11] * mundo.z + zerada.m[15];
    EXPECT_FLOAT_EQ(w, 0.0f);
}

// =============================================================================
// PROVENIENCIA — o caso de uso do vigil, encarnado (Emenda 1 do ADR 0002)
// =============================================================================
//
// Consumidor 1: `vigil` @ `e35f67a`, `src/vigil/app/Camera.h`. Consumidor
// PAUSADO — pela Emenda 1 continua valendo como evidencia, e o pedagio e este
// bloco. Os numeros abaixo estao transcritos dos comentarios de la.

TEST(Camera3dTest, VigilTrintaGrausAchataOChaoPelaMetade)
{
    // Transcrito de `vigil/src/vigil/app/Camera.h`:
    //
    //   "30  a proporcao 2:1 classica do Diablo 2 (`sin 30 = 0.5`)"
    //   "com ele o chao encolhe exatamente pela metade na vertical, que e o
    //    que faz um quadrado do tablado virar um losango 2:1"
    //
    // Aqui: sem giro, um segmento de 1 unidade NA PROFUNDIDADE do chao projeta
    // com metade da altura de um segmento de 1 unidade na horizontal.
    const Orbit o{ .target = {}, .distance = 20.0f, .yaw = 0.0f, .pitch = kPi / 6.0f };
    const Mat4  vp = multiply(orthographic(10.0f, 1.0f, 0.1f, 100.0f), view(o));

    const Vec3 origem = projetar(vp, { 0.0f, 0.0f, 0.0f });
    const Vec3 lateral = projetar(vp, { 1.0f, 0.0f, 0.0f }); // 1 na horizontal
    const Vec3 fundo = projetar(vp, { 0.0f, 0.0f, -1.0f });  // 1 na profundidade

    const float larguraNaTela = std::fabs(lateral.x - origem.x);
    const float alturaNaTela = std::fabs(fundo.y - origem.y);

    // `sin 30 = 0.5`: a profundidade encolhe exatamente pela metade.
    EXPECT_NEAR(alturaNaTela / larguraNaTela, 0.5f, 1e-4f);
}

TEST(Camera3dTest, VigilSemGiroUmQuadradoDoChaoViraRetanguloALINHADO)
{
    // Transcrito do mesmo arquivo, sobre o campo `giro`:
    //
    //   "Sem giro, um quadrado do chao projeta num RETANGULO alinhado aos
    //    eixos, e retangulo alinhado e indistinguivel de desenho 2D chapado. A
    //    inclinacao existia e nao APARECIA."
    //
    // Este teste FIXA o defeito: com yaw=0 os quatro cantos formam um retangulo
    // alinhado — dois pares de X iguais e dois pares de Y iguais.
    const Orbit o{ .target = {}, .distance = 20.0f, .yaw = 0.0f, .pitch = kPi / 6.0f };
    const Mat4  vp = multiply(orthographic(10.0f, 1.0f, 0.1f, 100.0f), view(o));

    const Vec3 a = projetar(vp, { -1.0f, 0.0f, -1.0f });
    const Vec3 b = projetar(vp, { 1.0f, 0.0f, -1.0f });
    const Vec3 c = projetar(vp, { 1.0f, 0.0f, 1.0f });
    const Vec3 d = projetar(vp, { -1.0f, 0.0f, 1.0f });

    EXPECT_NEAR(a.y, b.y, 1e-5f); // o lado de tras e horizontal na tela
    EXPECT_NEAR(c.y, d.y, 1e-5f);
    EXPECT_NEAR(a.x, d.x, 1e-5f); // o lado esquerdo e vertical na tela
    EXPECT_NEAR(b.x, c.x, 1e-5f);
}

TEST(Camera3dTest, VigilComGiroDeQuarentaECincoOQuadradoViraLOSANGO)
{
    //   "Com 45 graus o quadrado vira losango e a grade vira diamantes -- e e
    //    ai que o olho le 'plano no espaco'."
    //
    // A prova de que virou losango: **nenhum lado e alinhado aos eixos**, e as
    // duas diagonais na tela sao perpendiculares.
    const Orbit o{ .target = {}, .distance = 20.0f, .yaw = kPi / 4.0f, .pitch = kPi / 6.0f };
    const Mat4  vp = multiply(orthographic(10.0f, 1.0f, 0.1f, 100.0f), view(o));

    const Vec3 a = projetar(vp, { -1.0f, 0.0f, -1.0f });
    const Vec3 b = projetar(vp, { 1.0f, 0.0f, -1.0f });
    const Vec3 c = projetar(vp, { 1.0f, 0.0f, 1.0f });
    const Vec3 d = projetar(vp, { -1.0f, 0.0f, 1.0f });

    // Nenhum lado horizontal nem vertical: e o oposto exato do teste anterior.
    EXPECT_GT(std::fabs(a.y - b.y), 1e-3f);
    EXPECT_GT(std::fabs(a.x - d.x), 1e-3f);

    // Os cantos opostos ficam nos extremos: um no topo, um embaixo, um em cada
    // lado. E a leitura de "diamante".
    EXPECT_NEAR((a.x + c.x) * 0.5f, (b.x + d.x) * 0.5f, 1e-4f);
    EXPECT_NEAR((a.y + c.y) * 0.5f, (b.y + d.y) * 0.5f, 1e-4f);
}

TEST(Camera3dTest, VigilACameraAPrumoNaoAchataNada)
{
    //   "90  de cima, a prumo — o chao nao encolhe"
    const Orbit o{ .target = {}, .distance = 20.0f, .yaw = 0.0f, .pitch = kPi * 0.5f };
    const Mat4  vp = multiply(orthographic(10.0f, 1.0f, 0.1f, 100.0f), view(o));

    const Vec3 origem = projetar(vp, { 0.0f, 0.0f, 0.0f });
    const Vec3 lateral = projetar(vp, { 1.0f, 0.0f, 0.0f });
    const Vec3 fundo = projetar(vp, { 0.0f, 0.0f, -1.0f });

    EXPECT_NEAR(std::fabs(fundo.y - origem.y), std::fabs(lateral.x - origem.x), 1e-4f);
}

// =============================================================================
// PROVENIENCIA — o rig de captura do diorama (consumidor 2)
// =============================================================================

TEST(Camera3dTest, DioramaOOlhoBateComAPOSICAOQueOBlenderCALCULOU)
{
    // Consumidor 2: lab `diorama`, degrau 05. O rig de captura esta em
    // `diorama/tools/captura_referencia.py` e o resultado EFETIVO fica gravado
    // em `diorama/art/referencia/orientacao.txt`:
    //
    //   rig      giro 45 graus, inclinacao 55, distancia 12, alvo [0,0,1]
    //   camera   posicao [6.950736, -6.950736, 7.882917]   (eixos do Blender)
    //
    // O Blender e Z-para-cima; o mundo deste modulo e Y-para-cima (glTF). A
    // conversao e `(bx, by, bz) -> (bx, bz, -by)`, e a inclinacao do rig e
    // medida do ZENITE enquanto o `pitch` daqui e medido do CHAO.
    //
    // **Este teste e o que impede a traducao de ficar plausivel e errada.** Um
    // seno trocado por cosseno da uma imagem que parece uma camera — so nao e
    // ESTA camera.
    const float giro = 45.0f * kPi / 180.0f;
    const float inclinacaoDoZenite = 55.0f * kPi / 180.0f;

    const Orbit o{ .target = { 0.0f, 1.0f, 0.0f }, // alvo [0,0,1] do Blender
                   .distance = 12.0f,
                   .yaw = giro,
                   .pitch = kPi * 0.5f - inclinacaoDoZenite };

    const Vec3 e = eye(o);
    EXPECT_NEAR(e.x, 6.950736f, 1e-3f);  // bx
    EXPECT_NEAR(e.y, 7.882917f, 1e-3f);  // bz
    EXPECT_NEAR(e.z, 6.950736f, 1e-3f);  // -by
}
