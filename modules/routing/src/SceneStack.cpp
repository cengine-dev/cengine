#include <cengine/routing/SceneStack.hpp>

#include <utility>

namespace cengine::routing {

void SceneStack::push(Layer layer, const bool consumesInput)
{
    if (!layer)
    {
        return;
    }
    m_layers.push_back(Entry{ std::move(layer), consumesInput });
    m_layers.back().layer->onEnter();
}

void SceneStack::pop()
{
    if (m_layers.empty())
    {
        return;
    }

    // Segura a camada viva ate o fim do onExit(): tirar do vetor primeiro
    // destruiria o objeto no meio da propria chamada.
    const Layer leaving = m_layers.back().layer;
    m_layers.pop_back();
    leaving->onExit();
}

void SceneStack::replaceBottom(Layer layer)
{
    if (!layer || m_layers.empty())
    {
        return;
    }

    // Segura a que sai viva ate o fim do onExit(), como no pop(). A flag de
    // input e do LUGAR na pilha, nao do objeto: quem entra no fundo entra
    // com o mesmo papel de quem saiu.
    const Layer leaving = m_layers.front().layer;
    m_layers.front().layer = std::move(layer);
    leaving->onExit();
    m_layers.front().layer->onEnter();
}

bool SceneStack::isTop(const core::IScene* layer) const
{
    return !m_layers.empty() && m_layers.back().layer.get() == layer;
}

bool SceneStack::naPilha(const core::IScene* layer) const
{
    for (const Entry& entry: m_layers)
    {
        if (entry.layer.get() == layer)
        {
            return true;
        }
    }
    return false;
}

void SceneStack::update(const core::Seconds dt)
{
    // De baixo para cima, TODAS: e o criterio 1 do gate 18 — a cena de baixo
    // continua rodando atras do overlay.
    //
    // A copia existe porque uma camada pode empilhar ou desempilhar durante a
    // propria chamada, e iterar o vetor vivo invalidaria o iterador. Mas a copia
    // sozinha CRIAVA UM DEFEITO: uma camada removida no meio do laco -- que ja
    // recebeu `onExit()` -- ainda recebia `update()` depois, porque continuava
    // na copia. Cena morta continuando a simular e a mesma familia de "corpo que
    // some sem avisar", so que ao contrario.
    //
    // Por isso a confirmacao antes de cada chamada. A busca e linear e as pilhas
    // deste ecossistema tem duas ou tres camadas.
    const std::vector<Entry> snapshot = m_layers;
    for (const Entry& entry: snapshot)
    {
        if (!naPilha(entry.layer.get()))
        {
            continue; // saiu da pilha no meio deste mesmo laco
        }
        entry.layer->update(dt);
    }
}

void SceneStack::draw()
{
    // Mesma regra do `update`: quem saiu no meio do quadro nao desenha depois de
    // ter recebido `onExit()`.
    const std::vector<Entry> snapshot = m_layers;
    for (const Entry& entry: snapshot)
    {
        if (!naPilha(entry.layer.get()))
        {
            continue;
        }
        entry.layer->draw(); // ordem de desenho = ordem da pilha: o topo fica por cima
    }
}

void SceneStack::input()
{
    // Do topo para baixo ate a primeira camada ATIVA, e so ela recebe. Sem o
    // pulo das passivas, um HUD por cima do mundo deixaria o jogo inteiro sem
    // resposta — foi exatamente o que aconteceu no degrau 08.
    for (auto entry = m_layers.rbegin(); entry != m_layers.rend(); ++entry)
    {
        if (!entry->consumesInput)
        {
            continue;
        }

        // A copia importa: a camada pode empilhar/desempilhar (e ate se
        // remover) dentro deste input().
        const Layer active = entry->layer;
        active->input();
        return;
    }
}

} // namespace cengine::routing
