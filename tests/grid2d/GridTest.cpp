#include <gtest/gtest.h>

#include <cengine/grid2d/Grid.hpp>

namespace cengine::grid2d {
namespace {

/// A grade do TACTICS, transcrita do consumidor real.
///
/// tactics @ 5c59205, `ForgeBoardLayer.cpp`: tabuleiro 12x10, celula de 44
/// pixels, centralizado numa janela de 1280x720 com 20 pixels de desvio
/// vertical (o titulo). Os numeros abaixo sao a conta daquele arquivo, feita
/// aqui a mao para o teste medir a MESMA grade que o jogo desenha.
///
///   originX = (1280 - 12*44) / 2       = 376
///   originY = (720  - 10*44) / 2 + 20  = 160
Grid tacticsBoard()
{
    return Grid{ 376.0f, 160.0f, 44.0f, 12, 10 };
}

// --- ida ---

TEST(GridTest, ACellLandsWhereTheGameDrawsIt)
{
    const Grid grid = tacticsBoard();

    const Rect first = cellRect(grid, 0, 0);
    EXPECT_FLOAT_EQ(first.x, 376.0f);
    EXPECT_FLOAT_EQ(first.y, 160.0f);
    EXPECT_FLOAT_EQ(first.width, 44.0f);
    EXPECT_FLOAT_EQ(first.height, 44.0f);

    // A ultima celula do tabuleiro do tactics: (11, 9).
    const Rect last = cellRect(grid, 11, 9);
    EXPECT_FLOAT_EQ(last.x, 376.0f + 11 * 44.0f);
    EXPECT_FLOAT_EQ(last.y, 160.0f + 9 * 44.0f);
}

// --- volta: a metade que trouxe o gate ---

TEST(GridTest, APointInsideACellFindsThatCell)
{
    const Grid grid = tacticsBoard();

    int col = -1;
    int row = -1;

    // Canto exato da celula (0,0).
    ASSERT_TRUE(cellAt(grid, 376.0f, 160.0f, col, row));
    EXPECT_EQ(col, 0);
    EXPECT_EQ(row, 0);

    // Meio da celula (3,2).
    ASSERT_TRUE(cellAt(grid, 376.0f + 3 * 44.0f + 22.0f, 160.0f + 2 * 44.0f + 22.0f, col, row));
    EXPECT_EQ(col, 3);
    EXPECT_EQ(row, 2);
}

TEST(GridTest, APointBEFORETheGridIsRefusedAndNotWrappedAround)
{
    // ESTE e o teste que justifica o modulo.
    //
    // Escrito na ordem "natural" — converter para inteiro e depois conferir so
    // o limite de cima — um ponto A ESQUERDA da grade vira uma coluna enorme e
    // e recusado por acidente com `int`, e e COMPORTAMENTO INDEFINIDO com
    // inteiro sem sinal (que foi o tipo do primeiro consumidor: bulwark @
    // 4554c8d usa `uint32_t`). Aqui o negativo e testado ANTES.
    const Grid grid = tacticsBoard();

    int col = 0;
    int row = 0;

    EXPECT_FALSE(cellAt(grid, 375.0f, 200.0f, col, row));  // um pixel a esquerda
    EXPECT_FALSE(cellAt(grid, 400.0f, 159.0f, col, row));  // um pixel acima
    EXPECT_FALSE(cellAt(grid, 0.0f, 0.0f, col, row));      // canto da janela
    EXPECT_FALSE(cellAt(grid, -1000.0f, -1000.0f, col, row));
}

TEST(GridTest, APointAFTERTheGridIsRefused)
{
    const Grid grid = tacticsBoard();

    int col = 0;
    int row = 0;

    // Um pixel depois da ultima coluna / ultima linha.
    EXPECT_FALSE(cellAt(grid, 376.0f + 12 * 44.0f, 200.0f, col, row));
    EXPECT_FALSE(cellAt(grid, 400.0f, 160.0f + 10 * 44.0f, col, row));
}

TEST(GridTest, TheRefusalDoesNotTouchTheOutputs)
{
    const Grid grid = tacticsBoard();

    int col = 42;
    int row = 43;

    ASSERT_FALSE(cellAt(grid, -5.0f, -5.0f, col, row));

    // Quem recusou nao escreveu: o chamador pode confiar no que tinha.
    EXPECT_EQ(col, 42);
    EXPECT_EQ(row, 43);
}

TEST(GridTest, TheTwoDirectionsAgreeWithEachOther)
{
    // A volta desfaz a ida para TODA celula da grade — a propriedade que
    // impede os dois sentidos de divergirem, que e a razao de a ida ter subido
    // de carona.
    const Grid grid = tacticsBoard();

    for (int row = 0; row < grid.rows; ++row)
    {
        for (int col = 0; col < grid.cols; ++col)
        {
            const Rect rect = cellRect(grid, col, row);

            int backCol = -1;
            int backRow = -1;
            ASSERT_TRUE(cellAt(grid, rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f, backCol, backRow))
                << "centro da celula (" << col << "," << row << ") caiu fora da grade";

            EXPECT_EQ(backCol, col);
            EXPECT_EQ(backRow, row);
        }
    }
}

// --- grades degeneradas ---

TEST(GridTest, AGridWithNoCellsFindsNothing)
{
    const Grid empty{ 0.0f, 0.0f, 44.0f, 0, 0 };

    int col = 0;
    int row = 0;
    EXPECT_FALSE(cellAt(empty, 10.0f, 10.0f, col, row));
    EXPECT_FALSE(inside(empty, 0, 0));
}

TEST(GridTest, AZeroSizedCellIsRefusedInsteadOfDividingByZero)
{
    const Grid degenerate{ 0.0f, 0.0f, 0.0f, 4, 4 };

    int col = 0;
    int row = 0;
    EXPECT_FALSE(cellAt(degenerate, 10.0f, 10.0f, col, row));
}

TEST(GridTest, InsideAnswersTheBoundsWithoutPixels)
{
    const Grid grid = tacticsBoard();

    EXPECT_TRUE(inside(grid, 0, 0));
    EXPECT_TRUE(inside(grid, 11, 9));
    EXPECT_FALSE(inside(grid, 12, 0));
    EXPECT_FALSE(inside(grid, 0, 10));
    EXPECT_FALSE(inside(grid, -1, 0));
}

} // namespace
} // namespace cengine::grid2d
