#include <gtest/gtest.h>

#include <cengine/collision2d/Intersects.hpp>

// A deteccao 2D da engine. Geometria pura: nenhum jogo, nenhuma plataforma.
//
// A ultima secao ("consumidores reais") e o PEDAGIO da Emenda 1 da ADR 0002:
// como uma das duas evidencias que trouxeram este modulo vem de um jogo
// ESTACIONADO (o Space Invaders, que nunca vai linka-lo), a suite da engine tem
// de encarnar o caso de uso dele — provando que o mecanismo expressa a situacao
// real daquele jogo sem descongela-lo. Se um dia o modulo deixar de servir a
// esses testes, ele deixou de servir ao aprendizado que o justificou.

using cengine::collision2d::Aabb;
using cengine::collision2d::Circle;
using cengine::collision2d::intersects;

// =============================================================================
// Retangulo x retangulo
// =============================================================================

TEST(Collision2dTest, OverlappingBoxesIntersect)
{
    EXPECT_TRUE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 5.0f, 5.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, SeparateBoxesDoNotIntersect)
{
    EXPECT_FALSE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 20.0f, 0.0f, 10.0f, 10.0f }));

    // Separados so no eixo Y (alinhados em X): ainda assim, separados.
    EXPECT_FALSE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 0.0f, 20.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, TouchingBoxEdgesDoNotIntersect)
{
    // Borda direita de `a` exatamente na esquerda de `b`: area de sobreposicao
    // zero. Encostar nao conta (contrato documentado no header).
    EXPECT_FALSE(intersects(Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }, Aabb{ 10.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, BoxContainedInAnotherIntersects)
{
    EXPECT_TRUE(intersects(Aabb{ 0.0f, 0.0f, 100.0f, 100.0f }, Aabb{ 40.0f, 40.0f, 5.0f, 5.0f }));
}

TEST(Collision2dTest, BoxIntersectionIsSymmetric)
{
    const Aabb a{ 0.0f, 0.0f, 10.0f, 10.0f };
    const Aabb b{ 5.0f, 5.0f, 10.0f, 10.0f };

    EXPECT_EQ(intersects(a, b), intersects(b, a));
}

// =============================================================================
// Circulo x circulo
// =============================================================================

TEST(Collision2dTest, OverlappingCirclesIntersect)
{
    EXPECT_TRUE(intersects(Circle{ { 0.0f, 0.0f }, 10.0f }, Circle{ { 15.0f, 0.0f }, 10.0f }));
}

TEST(Collision2dTest, SeparateCirclesDoNotIntersect)
{
    EXPECT_FALSE(intersects(Circle{ { 0.0f, 0.0f }, 10.0f }, Circle{ { 21.0f, 0.0f }, 10.0f }));
}

TEST(Collision2dTest, TangentCirclesIntersect)
{
    // Distancia == soma dos raios: tangentes. CONTA (contrato do header) — senao
    // um tiro passaria raspando sem acertar.
    EXPECT_TRUE(intersects(Circle{ { 0.0f, 0.0f }, 10.0f }, Circle{ { 20.0f, 0.0f }, 10.0f }));
}

TEST(Collision2dTest, CircleContainedInAnotherIntersects)
{
    EXPECT_TRUE(intersects(Circle{ { 0.0f, 0.0f }, 50.0f }, Circle{ { 5.0f, 5.0f }, 2.0f }));
}

TEST(Collision2dTest, CircleDistanceIsMeasuredOnBothAxes)
{
    // Perto em X, longe em Y: nao se tocam. (Guarda contra a distancia ser
    // calculada so num eixo.)
    EXPECT_FALSE(intersects(Circle{ { 0.0f, 0.0f }, 5.0f }, Circle{ { 1.0f, 40.0f }, 5.0f }));
}

// =============================================================================
// Circulo x retangulo
// =============================================================================

TEST(Collision2dTest, CircleOverlappingBoxIntersects)
{
    EXPECT_TRUE(intersects(Circle{ { 12.0f, 5.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleAwayFromBoxDoesNotIntersect)
{
    EXPECT_FALSE(intersects(Circle{ { 30.0f, 5.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleNearBoxCornerUsesTrueDistanceNotTheBoundingBox)
{
    // O circulo esta na diagonal do canto (10,10), a ~7.07 de distancia, com
    // raio 5: NAO alcanca. Uma implementacao preguicosa (que so testasse o
    // AABB do circulo contra a caixa) diria que sim — este teste e o guarda.
    EXPECT_FALSE(intersects(Circle{ { 15.0f, 15.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));

    // Mais perto do mesmo canto, agora dentro do alcance.
    EXPECT_TRUE(intersects(Circle{ { 13.0f, 13.0f }, 5.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleInsideBoxIntersects)
{
    EXPECT_TRUE(intersects(Circle{ { 5.0f, 5.0f }, 1.0f }, Aabb{ 0.0f, 0.0f, 10.0f, 10.0f }));
}

TEST(Collision2dTest, CircleBoxIntersectionIsSymmetric)
{
    const Circle circle{ { 12.0f, 5.0f }, 5.0f };
    const Aabb   box{ 0.0f, 0.0f, 10.0f, 10.0f };

    EXPECT_EQ(intersects(circle, box), intersects(box, circle));
}

// =============================================================================
// Consumidores reais — o pedagio da Emenda 1 (ADR 0002)
// =============================================================================
//
// Os testes abaixo NAO inventam numeros: cada cena e transcrita do codigo do
// jogo que a originou, com a fonte citada (repo @ tag, arquivo e linha). E o que
// os torna verificaveis — quem duvidar de um valor abre o arquivo e confere,
// em vez de acreditar em mim. Se o jogo citado for uma referencia congelada, o
// arquivo esta la parado, entao a citacao nao apodrece.

// -----------------------------------------------------------------------------
// Space Invaders — a evidencia AABB
//
//   Fonte: github.com/mrmarmitt/spaceinvaders @ bb4e9b1 (jogo ESTACIONADO na
//          cengine 0.5.0 pela ADR 0003 — nunca vai linkar este modulo; o repo
//          nao tem tags, entao a referencia estavel e o commit em que ele parou.
//          Sendo codigo congelado, as linhas citadas abaixo nao se mexem.)
//   Arena: 224x256, Y para baixo, SEM wrap ...... src/spaceinvaders/game/World.h:48-49
//   Colisao original (AABB a mao) .............. src/spaceinvaders/game/World.h:26-29
//   Tiro x invasor ............................. src/spaceinvaders/game/World.cpp:227
//   Bomba x canhao ............................. src/spaceinvaders/game/World.cpp:259
//
// Medidas transcritas de World.cpp:9-47 — canhao 13x8 em y=232, tiro 3x7,
// bomba 3x7, invasor 8..12 de largura por 8 de altura (Squid 8, Crab 11,
// Octopus 12), celula da horda 16x12.
//
// Se o modulo nao consegue expressar estas cenas, ele nao esta pronto para subir.
// -----------------------------------------------------------------------------

TEST(Collision2dTest, SpaceInvadersPlayerShotHitsInvader)
{
    // Um Crab (11x8), como o invaderRect() do jogo o monta.
    const Aabb invader{ 100.0f, 60.0f, 11.0f, 8.0f };

    // Tiro do jogador (3x7) subindo, entrando pela base do invasor.
    EXPECT_TRUE(intersects(Aabb{ 104.0f, 64.0f, 3.0f, 7.0f }, invader));

    // Mesma coluna, ainda abaixo dele: nao acertou (ainda).
    EXPECT_FALSE(intersects(Aabb{ 104.0f, 80.0f, 3.0f, 7.0f }, invader));

    // Na altura certa, mas na coluna do vizinho (celula da horda = 16): passa.
    EXPECT_FALSE(intersects(Aabb{ 116.0f, 64.0f, 3.0f, 7.0f }, invader));
}

TEST(Collision2dTest, SpaceInvadersBombHitsPlayerCannon)
{
    // Canhao 13x8 na linha do jogador (kPlayerY = 232).
    const Aabb cannon{ 50.0f, 232.0f, 13.0f, 8.0f };

    // Bomba (3x7) chegando na cabeca do canhao.
    EXPECT_TRUE(intersects(Aabb{ 55.0f, 228.0f, 3.0f, 7.0f }, cannon));

    // Ainda caindo, la em cima: nada.
    EXPECT_FALSE(intersects(Aabb{ 55.0f, 180.0f, 3.0f, 7.0f }, cannon));
}

TEST(Collision2dTest, SpaceInvadersEdgeContractIsPreserved)
{
    // O `si::Rect::intersects` original (World.h:26-29) usa comparacoes
    // ESTRITAS (`x < other.x + other.w`): encostar nao colide. O `intersects`
    // desta engine mantem o mesmo contrato — o jogo congelado nao mudaria de
    // comportamento se fosse migrado (e e essa a promessa que a promocao faz).
    const Aabb invader{ 100.0f, 60.0f, 11.0f, 8.0f };
    const Aabb shotTouchingTheLeftEdge{ 97.0f, 64.0f, 3.0f, 7.0f }; // 97+3 == 100

    EXPECT_FALSE(intersects(shotTouchingTheLeftEdge, invader));
}

// -----------------------------------------------------------------------------
// Asteroids — a evidencia circulo
//
//   Fonte: github.com/mrmarmitt/asteroids @ main (consumidor VIVO)
//   Arena: 800x600 que DA A VOLTA ............. src/asteroids/game/World.h (kArenaW/kArenaH)
//   Raios: nave 9, tiro 2, rocha 42/22/11 ..... src/asteroids/game/World.h (kShipRadius,
//                                               kShotRadius, asteroidRadius)
//   Correcao do toro (fica NO JOGO) ........... src/asteroids/game/World.cpp (toroidalDelta,
//                                               chamado por World::circlesOverlap)
// -----------------------------------------------------------------------------

TEST(Collision2dTest, AsteroidsShotHitsRock)
{
    const Circle rock{ { 400.0f, 180.0f }, 42.0f }; // rocha grande
    const Circle shot{ { 400.0f, 220.0f }, 2.0f };  // tiro subindo, a 40 do centro

    EXPECT_TRUE(intersects(shot, rock)) << "40 < 42 + 2: o tiro entrou na rocha";

    EXPECT_FALSE(intersects(Circle{ { 400.0f, 300.0f }, 2.0f }, rock)) << "ainda a caminho";
}

TEST(Collision2dTest, AsteroidsShipHitsRock)
{
    const Circle ship{ { 400.0f, 300.0f }, 9.0f };
    const Circle mediumRock{ { 430.0f, 300.0f }, 22.0f };

    EXPECT_TRUE(intersects(ship, mediumRock)) << "30 < 9 + 22";
    EXPECT_FALSE(intersects(ship, Circle{ { 440.0f, 300.0f }, 22.0f })) << "40 > 9 + 22";
}

TEST(Collision2dTest, AsteroidsWrapIsTheGamesJobNotTheEngines)
{
    // Nave colada na borda esquerda, rocha colada na direita: na arena-toro do
    // Asteroids elas se tocam — a distancia PELA BORDA e 10, nao 790.
    const Circle ship{ { 5.0f, 300.0f }, 9.0f };
    const Circle rockFarSide{ { 795.0f, 300.0f }, 22.0f };

    // A engine, que nao sabe do toro, responde o que a geometria plana diz: 790
    // de distancia, nao se tocam. E esta CERTA — ela nao tem (nem deve ter)
    // opiniao sobre o formato do mundo.
    EXPECT_FALSE(intersects(ship, rockFarSide));

    // O JOGO corrige a posicao pelo menor delta no toro e pergunta de novo — e
    // exatamente isto que o ast::World::circlesOverlap faz antes de chamar aqui
    // (795 - 800 = -5, a 10 da nave).
    constexpr float kArenaW = 800.0f;
    const Circle    rockWrapped{ { rockFarSide.center.x - kArenaW, rockFarSide.center.y }, rockFarSide.radius };

    EXPECT_TRUE(intersects(ship, rockWrapped)) << "com a posicao corrigida pelo jogo, a engine acerta";
}
