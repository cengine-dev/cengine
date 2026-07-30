#include <cengine/input/Mouse.hpp>

namespace cengine::input {

void Mouse::pushPosition(const float x, const float y)
{
    m_x = x;
    m_y = y;
}

void Mouse::pushClick(const MouseClick click)
{
    if (click.button == MouseButton::None)
    {
        return; // "nenhum botao" nao e um clique
    }
    if (m_queue.size() >= kQueueMax)
    {
        return; // cheia: descarta o NOVO, para nao embaralhar a ordem
    }
    m_queue.push_back(click);
}

MouseClick Mouse::readClick()
{
    if (m_queue.empty())
    {
        return MouseClick{};
    }

    const MouseClick front = m_queue.front();
    m_queue.erase(m_queue.begin());
    return front;
}

} // namespace cengine::input
