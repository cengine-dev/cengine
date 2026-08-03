#include <cengine/input/Mouse.hpp>

namespace cengine::input {

void Mouse::pushPosition(const float x, const float y)
{
    m_x = x;
    m_y = y;

    // O gesto em andamento acompanha o ponteiro: e o que permite a cena desenhar
    // o que esta na mao seguindo o cursor.
    if (m_drag.active)
    {
        m_drag.x = x;
        m_drag.y = y;
    }
}

void Mouse::pushClick(const MouseClick click)
{
    if (click.button == MouseButton::None)
    {
        return; // "nenhum botao" nao e um clique
    }
    if (m_clicks.size() >= kQueueMax)
    {
        return; // cheia: descarta o NOVO, para nao embaralhar a ordem
    }
    m_clicks.push_back(click);
}

void Mouse::pushDown(const float x, const float y)
{
    m_drag.active = true;
    m_drag.startX = x;
    m_drag.startY = y;
    m_drag.x = x;
    m_drag.y = y;
}

void Mouse::pushUp(const float x, const float y)
{
    if (!m_drag.active)
    {
        return; // soltou sem ter apertado aqui (o aperto foi noutra janela)
    }

    if (m_drops.size() < kQueueMax)
    {
        m_drops.push_back(Drop{ true, m_drag.startX, m_drag.startY, x, y });
    }

    m_drag = DragState{};
}

void Mouse::cancelDrag()
{
    m_drag = DragState{};
}

MouseClick Mouse::readClick()
{
    if (m_clicks.empty())
    {
        return MouseClick{};
    }

    const MouseClick front = m_clicks.front();
    m_clicks.erase(m_clicks.begin());
    return front;
}

Drop Mouse::readDrop()
{
    if (m_drops.empty())
    {
        return Drop{};
    }

    const Drop front = m_drops.front();
    m_drops.erase(m_drops.begin());
    return front;
}

} // namespace cengine::input
