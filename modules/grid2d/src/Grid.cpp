#include <cengine/grid2d/Grid.hpp>

namespace cengine::grid2d {

Rect cellRect(const Grid& grid, const int col, const int row)
{
    return Rect{ grid.originX + static_cast<float>(col) * grid.cellSize,
                 grid.originY + static_cast<float>(row) * grid.cellSize, grid.cellSize, grid.cellSize };
}

bool inside(const Grid& grid, const int col, const int row)
{
    return col >= 0 && row >= 0 && col < grid.cols && row < grid.rows;
}

bool cellAt(const Grid& grid, const float x, const float y, int& col, int& row)
{
    if (grid.cellSize <= 0.0f)
    {
        return false; // grade degenerada: dividir por zero nao e resposta
    }

    const float localX = x - grid.originX;
    const float localY = y - grid.originY;

    // NEGATIVO ANTES da divisao e da conversao. Ver o bloco no cabecalho: e
    // aqui que a versao "natural" erra em silencio.
    if (localX < 0.0f || localY < 0.0f)
    {
        return false;
    }

    const int candidateCol = static_cast<int>(localX / grid.cellSize);
    const int candidateRow = static_cast<int>(localY / grid.cellSize);

    if (!inside(grid, candidateCol, candidateRow))
    {
        return false;
    }

    col = candidateCol;
    row = candidateRow;
    return true;
}

} // namespace cengine::grid2d
