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

} // namespace
} // namespace cengine::input
