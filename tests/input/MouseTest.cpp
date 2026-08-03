#include <gtest/gtest.h>

#include <cengine/input/Mouse.hpp>

namespace cengine::input {
namespace {

TEST(MouseTest, PositionIsStateAndSurvivesBeingRead)
{
    Mouse mouse;

    // Nasce na origem: uma cena que desenhe realce antes do primeiro
    // WM_MOUSEMOVE nao pode ler lixo.
    EXPECT_FLOAT_EQ(mouse.x(), 0.0f);
    EXPECT_FLOAT_EQ(mouse.y(), 0.0f);

    mouse.pushPosition(120.0f, 40.0f);

    // Ler NAO consome: e estado, e o realce sob o cursor precisa dele todo
    // quadro.
    EXPECT_FLOAT_EQ(mouse.x(), 120.0f);
    EXPECT_FLOAT_EQ(mouse.y(), 40.0f);
    EXPECT_FLOAT_EQ(mouse.x(), 120.0f);
}

TEST(MouseTest, AClickIsAnEdgeAndIsConsumedOnce)
{
    Mouse mouse;

    mouse.pushClick(MouseClick{ MouseButton::Left, 10.0f, 20.0f });

    const MouseClick first = mouse.readClick();
    EXPECT_EQ(first.button, MouseButton::Left);
    EXPECT_FLOAT_EQ(first.x, 10.0f);
    EXPECT_FLOAT_EQ(first.y, 20.0f);

    // Consumido: a fila esta vazia.
    EXPECT_EQ(mouse.readClick().button, MouseButton::None);
}

TEST(MouseTest, TheClickCarriesWhereItHappenedAndNotWhereThePointerIsNow)
{
    // A decisao mais importante deste tipo. O ponteiro anda entre o aperto e o
    // quadro em que a cena le a fila; sem a posicao no evento, um clique
    // rapido seguido de um movimento acertaria a celula errada.
    Mouse mouse;

    mouse.pushClick(MouseClick{ MouseButton::Left, 10.0f, 20.0f });
    mouse.pushPosition(900.0f, 700.0f); // o ponteiro fugiu depois do clique

    const MouseClick click = mouse.readClick();
    EXPECT_FLOAT_EQ(click.x, 10.0f);
    EXPECT_FLOAT_EQ(click.y, 20.0f);

    // E o estado seguiu o ponteiro, sem contaminar o clique.
    EXPECT_FLOAT_EQ(mouse.x(), 900.0f);
}

TEST(MouseTest, ClicksComeOutInTheOrderTheyWentIn)
{
    Mouse mouse;

    mouse.pushClick(MouseClick{ MouseButton::Left, 1.0f, 1.0f });
    mouse.pushClick(MouseClick{ MouseButton::Right, 2.0f, 2.0f });

    EXPECT_EQ(mouse.readClick().button, MouseButton::Left);
    EXPECT_EQ(mouse.readClick().button, MouseButton::Right);
    EXPECT_EQ(mouse.readClick().button, MouseButton::None);
}

TEST(MouseTest, ANonClickIsNotQueued)
{
    Mouse mouse;

    mouse.pushClick(MouseClick{ MouseButton::None, 5.0f, 5.0f });

    EXPECT_EQ(mouse.readClick().button, MouseButton::None);
}

TEST(MouseTest, AFullQueueDropsTheNEWClickAndKeepsTheOrder)
{
    // Mesma politica da fila de teclas, e pela mesma razao: quem esta na fila
    // chegou primeiro e sera consumido primeiro — descartar o velho
    // embaralharia a ordem em que o jogador clicou.
    Mouse mouse;

    for (size_t i = 0; i < Mouse::kQueueMax; ++i)
    {
        mouse.pushClick(MouseClick{ MouseButton::Left, static_cast<float>(i), 0.0f });
    }
    mouse.pushClick(MouseClick{ MouseButton::Right, 999.0f, 0.0f }); // nao cabe

    // O primeiro continua sendo o primeiro...
    EXPECT_FLOAT_EQ(mouse.readClick().x, 0.0f);

    // ...e o que nao coube nao esta em lugar nenhum da fila.
    for (size_t i = 1; i < Mouse::kQueueMax; ++i)
    {
        const MouseClick click = mouse.readClick();
        EXPECT_EQ(click.button, MouseButton::Left);
        EXPECT_FLOAT_EQ(click.x, static_cast<float>(i));
    }
    EXPECT_EQ(mouse.readClick().button, MouseButton::None);
}

// --- ARRASTAR (task 28, 0.15.0) ---
//
// As duas evidencias que trouxeram o gesto para ca estao encarnadas abaixo, com
// a origem citada. A regra da casa: um teste de consumidor real cita o
// repositorio, o commit e a linha, e transcreve os valores do jogo.

TEST(MouseTest, ThereIsNoGestureBeforeTheButtonGoesDown)
{
    Mouse mouse;

    EXPECT_FALSE(mouse.drag().active);
    EXPECT_FALSE(mouse.readDrop().happened);
}

TEST(MouseTest, TheGestureFollowsThePointerWithoutForgettingWhereItStarted)
{
    // A razao de o estado existir separado do edge: a cena desenha o que esta na
    // mao TODO QUADRO, e para isso precisa das duas pontas ao mesmo tempo.
    Mouse mouse;

    mouse.pushDown(100.0f, 50.0f);
    mouse.pushPosition(140.0f, 90.0f);

    const DragState state = mouse.drag();
    EXPECT_TRUE(state.active);
    EXPECT_FLOAT_EQ(state.startX, 100.0f);
    EXPECT_FLOAT_EQ(state.startY, 50.0f);
    EXPECT_FLOAT_EQ(state.x, 140.0f);
    EXPECT_FLOAT_EQ(state.y, 90.0f);

    // Ler NAO consome: e estado.
    EXPECT_TRUE(mouse.drag().active);
}

TEST(MouseTest, ThePointerMovesTheGestureOnlyWhileItIsActive)
{
    Mouse mouse;

    mouse.pushPosition(10.0f, 10.0f);
    EXPECT_FALSE(mouse.drag().active);
    EXPECT_FLOAT_EQ(mouse.drag().x, 0.0f); // nao existe gesto para acompanhar

    mouse.pushDown(20.0f, 20.0f);
    mouse.pushUp(30.0f, 30.0f);
    mouse.pushPosition(999.0f, 999.0f);

    EXPECT_FALSE(mouse.drag().active);
    EXPECT_FLOAT_EQ(mouse.x(), 999.0f); // mas a POSICAO segue o ponteiro
}

TEST(MouseTest, LettingGoCarriesBOTHEndsTogether)
{
    Mouse mouse;

    mouse.pushDown(100.0f, 50.0f);
    mouse.pushPosition(140.0f, 90.0f);
    mouse.pushUp(160.0f, 110.0f);

    const Drop drop = mouse.readDrop();
    EXPECT_TRUE(drop.happened);
    EXPECT_FLOAT_EQ(drop.startX, 100.0f);
    EXPECT_FLOAT_EQ(drop.startY, 50.0f);
    EXPECT_FLOAT_EQ(drop.x, 160.0f);
    EXPECT_FLOAT_EQ(drop.y, 110.0f);

    // Um edge: consumido uma vez, e o gesto acabou.
    EXPECT_FALSE(mouse.readDrop().happened);
    EXPECT_FALSE(mouse.drag().active);
}

TEST(MouseTest, LettingGoWithoutHavingPressedHereProducesNothing)
{
    // O aperto aconteceu noutra janela e o ponteiro entrou nesta ja segurado.
    Mouse mouse;

    mouse.pushUp(50.0f, 50.0f);

    EXPECT_FALSE(mouse.readDrop().happened);
    EXPECT_FALSE(mouse.drag().active);
}

TEST(MouseTest, CancellingEndsTheGestureWithoutADrop)
{
    // A janela perdeu o foco, ou alguem tomou a captura do ponteiro: o "soltou"
    // pode nunca chegar. Cancelar e melhor que soltar num destino que o jogador
    // nao escolheu.
    Mouse mouse;

    mouse.pushDown(100.0f, 50.0f);
    mouse.pushPosition(140.0f, 90.0f);
    mouse.cancelDrag();

    EXPECT_FALSE(mouse.drag().active);
    EXPECT_FALSE(mouse.readDrop().happened);
}

TEST(MouseTest, DropsComeOutInTheOrderTheyWentIn)
{
    Mouse mouse;

    mouse.pushDown(1.0f, 1.0f);
    mouse.pushUp(2.0f, 2.0f);
    mouse.pushDown(3.0f, 3.0f);
    mouse.pushUp(4.0f, 4.0f);

    EXPECT_FLOAT_EQ(mouse.readDrop().startX, 1.0f);
    EXPECT_FLOAT_EQ(mouse.readDrop().startX, 3.0f);
    EXPECT_FALSE(mouse.readDrop().happened);
}

TEST(MouseTest, AFullQueueDropsTheNEWGestureAndKeepsTheOrder)
{
    // Mesma politica da fila de cliques e da de teclas, e pela mesma razao.
    Mouse mouse;

    for (size_t i = 0; i < Mouse::kQueueMax; ++i)
    {
        mouse.pushDown(static_cast<float>(i), 0.0f);
        mouse.pushUp(static_cast<float>(i), 1.0f);
    }
    mouse.pushDown(999.0f, 0.0f);
    mouse.pushUp(999.0f, 1.0f); // nao cabe

    for (size_t i = 0; i < Mouse::kQueueMax; ++i)
    {
        const Drop drop = mouse.readDrop();
        ASSERT_TRUE(drop.happened);
        EXPECT_FLOAT_EQ(drop.startX, static_cast<float>(i));
    }
    EXPECT_FALSE(mouse.readDrop().happened);
}

TEST(MouseTest, TheKlondikeGesture)
{
    // PRIMEIRA EVIDENCIA — klondike@83ca732,
    // src/platform/theforge/src/KlondikeForge/scene/ForgeGameScene.cpp:34,402.
    //
    // O jogo compara a distancia do gesto com `kTapSlack = 8.0f`: soltou perto
    // de onde apertou vale como TOQUE (a logica de dois cliques); soltou longe e
    // ARRASTO. A porta entrega as duas pontas e NAO opina sobre qual foi.
    constexpr float kTapSlack = 8.0f;

    Mouse mouse;
    mouse.pushDown(300.0f, 400.0f);
    mouse.pushPosition(304.0f, 402.0f); // tremeu 4,5 pixels
    mouse.pushUp(304.0f, 402.0f);

    const Drop drop = mouse.readDrop();
    ASSERT_TRUE(drop.happened);

    const float dx = drop.x - drop.startX;
    const float dy = drop.y - drop.startY;
    EXPECT_LT(dx * dx + dy * dy, kTapSlack * kTapSlack); // o JOGO chama isto de toque
}

TEST(MouseTest, TheCueGesture)
{
    // SEGUNDA EVIDENCIA — cue@3e9c785,
    // src/platform/theforge/src/CueForge/scene/ForgeGameScene.cpp:67,72,293,322.
    //
    // A tacada e um ESTILINGUE: a direcao do tiro e o CONTRARIO do puxao, e a
    // forca e o tamanho dele. O jogo usa `kShotSlack = 12.0f` — numero
    // DIFERENTE do Klondike, e e esse o ponto: a folga e politica do jogo, e
    // depende do tamanho do alvo.
    //
    // Este teste existe para provar que a porta entrega o que os DOIS precisam
    // sem escolher por nenhum dos dois.
    constexpr float kShotSlack = 12.0f;

    Mouse mouse;
    mouse.pushDown(400.0f, 350.0f);   // apertou na branca
    mouse.pushPosition(340.0f, 350.0f); // puxou 60 pixels para a esquerda
    mouse.pushUp(340.0f, 350.0f);

    const Drop drop = mouse.readDrop();
    ASSERT_TRUE(drop.happened);

    const float pullX = drop.startX - drop.x; // o tiro vai ao contrario do puxao
    const float pullY = drop.startY - drop.y;
    EXPECT_FLOAT_EQ(pullX, 60.0f);
    EXPECT_FLOAT_EQ(pullY, 0.0f);
    EXPECT_GT(pullX * pullX + pullY * pullY, kShotSlack * kShotSlack); // e uma tacada
}

} // namespace
} // namespace cengine::input
