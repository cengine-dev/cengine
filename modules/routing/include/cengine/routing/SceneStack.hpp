#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

namespace cengine::routing {

/**
 * @brief Cenas EMPILHADAS: overlays que compoem em vez de substituir.
 *
 * O `IRouter` modela uma cena ativa por vez — bom para fluxos lineares
 * (splash -> menu -> jogo -> saida) e insuficiente para pausa sobre o jogo,
 * inventario sobre a pausa, ou um HUD que sobrevive a troca do que esta
 * embaixo. Esta pilha e a resposta para esses casos, e vive ao lado do
 * router: quem quer compor empilha, quem quer trocar continua trocando.
 *
 * ## A regra
 *
 * - `update(dt)` e `draw()` chegam em TODAS as camadas, de baixo para cima;
 * - `input()` chega SO na primeira camada ATIVA a partir do topo.
 *
 * A ordem de desenho e a da pilha: quem esta em cima desenha por ultimo.
 *
 * ## Proveniencia (task 18, ADR 0002)
 *
 * Isto NAO foi desenhado aqui: foi EXTRAIDO do `delve::LayerStack`
 * (delve@f9cd31b, `src/delve/app/LayerStack.{h,cpp}`), que nasceu local no
 * jogo, como a task 18 mandava, e cumpriu as tres condicoes do gate — cena
 * de baixo continuando a rodar, duas camadas simultaneas, e overlay que
 * sobrevive a troca da cena de baixo (`replaceBottom`, usado na descida
 * entre andares).
 *
 * O desenho que a task 18 imaginava ANTES de existir consumidor tinha tres
 * politicas por camada (`blocksInputBelow`, `updatesBelow`, `drawsBelow`).
 * Duas delas nao tiveram nenhuma evidencia em cinco tipos de camada de uso
 * real e nao subiram; a terceira apareceu com outra forma — nao "esta camada
 * bloqueia as de baixo?" e sim "esta camada PARTICIPA do input?", que e o
 * `consumesInput` do `push`.
 *
 * O segundo consumidor e o Bulwark (tower defense, degrau 06), que exercita
 * a extracao adaptando em vez de reescrever.
 */
class SceneStack
{
public:
    using Layer = std::shared_ptr<core::IScene>;

    // Empilha e chama onEnter() na camada nova.
    //
    // `consumesInput = false` marca uma camada PASSIVA: ela desenha e recebe
    // update, mas nao entra na fila do input — quem esta embaixo continua
    // jogando. E o caso do HUD.
    //
    // Isto NAO estava previsto. A regra original ("so o topo recebe input")
    // parecia completa ate o HUD virar camada por cima do mundo no degrau
    // 08: o jogo abria, animava, e nao respondia a tecla nenhuma, porque o
    // HUD engolia tudo e o input dele e vazio. Achado JOGANDO, e vale como
    // evidencia para o gate 18 — uma pilha de cenas de verdade precisa
    // separar camada que age de camada que so mostra.
    void push(Layer layer, bool consumesInput = true);

    // Desempilha o topo, chamando onExit(). No-op se estiver vazia.
    void pop();

    // Troca a camada de BAIXO mantendo intacto tudo que esta por cima: a que
    // sai recebe onExit(), a que entra recebe onEnter(). No-op se a pilha
    // estiver vazia.
    //
    // E o terceiro criterio do gate 18 ("overlay que sobrevive a troca de
    // cena de baixo"), que ficou descoberto no degrau 05 por falta de uma
    // segunda cena de mundo para trocar. Com a descida entre andares (degrau
    // 08) ele deixa de ser hipotetico.
    //
    // `replaceBottom` e nao `replaceAt(indice)`: so a de baixo e trocada, e a
    // forma geral seria desenhar API sem evidencia — mesma disciplina que
    // manteve esta pilha local no degrau 05.
    void replaceBottom(Layer layer);

    [[nodiscard]] bool   empty() const { return m_layers.empty(); }
    [[nodiscard]] size_t size() const { return m_layers.size(); }

    // Quem esta recebendo input. Uma camada usa isto para se desenhar de
    // outro jeito quando NAO e a ativa.
    [[nodiscard]] bool isTop(const core::IScene* layer) const;

    void update(core::Seconds dt);
    void draw();

    // Do topo para baixo, ate achar a primeira camada que consome input —
    // e SO ela recebe. Camadas passivas (o HUD) sao puladas.
    void input();

private:
    struct Entry
    {
        Layer layer;
        bool  consumesInput;
    };

    std::vector<Entry> m_layers;
};

} // namespace cengine::routing
