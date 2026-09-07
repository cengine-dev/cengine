// EXTRAIDOS junto com a classe, do `delve/test/delve/app/LayerStackTest.cpp`
// (delve@f9cd31b). O ADR 0002 exige que o mecanismo promovido seja testavel
// DENTRO da cengine, sem jogo e sem GPU — e era, desde o prototipo: a camada
// de mentira abaixo so anota num diario o que recebeu, e a ORDEM das
// anotacoes e o que se confere.

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <cengine/routing/SceneStack.hpp>

namespace cengine::routing {
namespace {

// Camada de mentira que so anota, num diario compartilhado, o que recebeu.
// A ordem das anotacoes E o que os testes conferem — a fronteira do degrau
// e exatamente "quem recebe o que, em que ordem".
class RecordingLayer final: public core::IScene
{
public:
    RecordingLayer(std::string name, std::string& log): m_name(std::move(name)), m_log(log) {}

    void onEnter() override { m_log += m_name + ":enter "; }
    void update(core::Seconds) override { m_log += m_name + ":update "; }
    void draw() override { m_log += m_name + ":draw "; }
    void input() override { m_log += m_name + ":input "; }
    void onExit() override { m_log += m_name + ":exit "; }

private:
    std::string  m_name;
    std::string& m_log;
};

TEST(SceneStackTest, StartsEmpty)
{
    const SceneStack stack;
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.size(), 0u);
    EXPECT_FALSE(stack.isTop(nullptr));
}

TEST(SceneStackTest, AnEmptyStackTakesTheWholeCycleWithoutBreaking)
{
    SceneStack stack;
    stack.update(core::Seconds{0.016f});
    stack.draw();
    stack.input();
    stack.pop(); // desempilhar de pilha vazia tambem e no-op
    EXPECT_TRUE(stack.empty());
}

TEST(SceneStackTest, PushCallsOnEnterAndPopCallsOnExit)
{
    std::string log;
    SceneStack  stack;

    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    EXPECT_EQ(log, "mundo:enter ");
    EXPECT_EQ(stack.size(), 1u);

    stack.pop();
    EXPECT_EQ(log, "mundo:enter mundo:exit ");
    EXPECT_TRUE(stack.empty());
}

TEST(SceneStackTest, UpdateAndDrawVisitEveryLayerFromBottomToTop)
{
    // O criterio 1 do gate 18: a cena de baixo continua RODANDO atras do
    // overlay — nao e congelada por ele.
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    stack.push(std::make_shared<RecordingLayer>("pausa", log));
    log.clear();

    stack.update(core::Seconds{0.016f});
    EXPECT_EQ(log, "mundo:update pausa:update ");

    log.clear();
    stack.draw();
    EXPECT_EQ(log, "mundo:draw pausa:draw "); // ordem de desenho: topo por cima
}

TEST(SceneStackTest, InputGoesOnlyToTheTopLayer)
{
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    stack.push(std::make_shared<RecordingLayer>("pausa", log));
    log.clear();

    stack.input();
    EXPECT_EQ(log, "pausa:input ");
}

TEST(SceneStackTest, ThreeLayersStackedKeepTheWholeOrder)
{
    // Duas camadas por cima do mundo — o criterio 2 do gate 18.
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    stack.push(std::make_shared<RecordingLayer>("pausa", log));
    stack.push(std::make_shared<RecordingLayer>("mochila", log));
    ASSERT_EQ(stack.size(), 3u);
    log.clear();

    stack.update(core::Seconds{0.016f});
    stack.input();
    EXPECT_EQ(log, "mundo:update pausa:update mochila:update mochila:input ");
}

TEST(SceneStackTest, IsTopAnswersForTheTopLayerOnly)
{
    std::string log;
    const auto  world = std::make_shared<RecordingLayer>("mundo", log);
    const auto  pause = std::make_shared<RecordingLayer>("pausa", log);

    SceneStack stack;
    stack.push(world);
    EXPECT_TRUE(stack.isTop(world.get()));

    stack.push(pause);
    EXPECT_FALSE(stack.isTop(world.get()));
    EXPECT_TRUE(stack.isTop(pause.get()));

    stack.pop();
    EXPECT_TRUE(stack.isTop(world.get()));
}

TEST(SceneStackTest, PoppingUncoversTheLayerBelowForInput)
{
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    stack.push(std::make_shared<RecordingLayer>("pausa", log));

    stack.pop();
    log.clear();

    stack.input();
    EXPECT_EQ(log, "mundo:input ");
}

TEST(SceneStackTest, APassiveLayerDoesNotSwallowTheInput)
{
    // O bug do degrau 08, achado JOGANDO: com o HUD empilhado por cima do
    // mundo, a regra "so o topo recebe input" deixava o jogo sem resposta —
    // o HUD ficava com todo o input e o input dele e vazio.
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    stack.push(std::make_shared<RecordingLayer>("hud", log), false); // passiva
    log.clear();

    stack.input();
    EXPECT_EQ(log, "mundo:input "); // pulou o HUD e chegou em quem joga

    // Passiva nao quer dizer invisivel: ela continua desenhando e recebendo
    // update, por cima.
    log.clear();
    stack.update(core::Seconds{0.016f});
    stack.draw();
    EXPECT_EQ(log, "mundo:update hud:update mundo:draw hud:draw ");
}

TEST(SceneStackTest, AnActiveLayerAboveAPassiveOneTakesTheInput)
{
    // Pausa aberta por cima do HUD: quem recebe input e a pausa, nao o
    // mundo, mesmo com uma camada passiva no meio do caminho.
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    stack.push(std::make_shared<RecordingLayer>("hud", log), false);
    stack.push(std::make_shared<RecordingLayer>("pausa", log));
    log.clear();

    stack.input();
    EXPECT_EQ(log, "pausa:input ");
}

TEST(SceneStackTest, AStackOfOnlyPassiveLayersConsumesNothing)
{
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("hud", log), false);
    log.clear();

    stack.input();
    EXPECT_EQ(log, "");
}

TEST(SceneStackTest, ReplaceBottomSwapsTheWorldAndKeepsTheOverlaysAbove)
{
    // O criterio 3 do gate 18: o overlay SOBREVIVE a troca da cena de baixo.
    // Ficou descoberto no degrau 05 por falta de uma segunda cena de mundo;
    // a descida entre andares (degrau 08) deu a segunda.
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("andar1", log));
    stack.push(std::make_shared<RecordingLayer>("hud", log));
    log.clear();

    stack.replaceBottom(std::make_shared<RecordingLayer>("andar2", log));

    // A que saiu se despede, a que entrou e apresentada, e o HUD nem fica
    // sabendo — ele nao recebe onExit nem onEnter.
    EXPECT_EQ(log, "andar1:exit andar2:enter ");
    EXPECT_EQ(stack.size(), 2u);

    log.clear();
    stack.update(core::Seconds{0.016f});
    stack.draw();
    stack.input();

    // O andar novo esta embaixo, o HUD continua em cima e ainda e quem
    // recebe input.
    EXPECT_EQ(log, "andar2:update hud:update andar2:draw hud:draw hud:input ");
}

TEST(SceneStackTest, ReplaceBottomOnAnEmptyStackIsANoOp)
{
    std::string log;
    SceneStack  stack;

    stack.replaceBottom(std::make_shared<RecordingLayer>("mundo", log));
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(log, ""); // nem onEnter: nao ha fundo para trocar
}

TEST(SceneStackTest, ReplaceBottomWithASingleLayerSwapsIt)
{
    std::string log;
    SceneStack  stack;
    stack.push(std::make_shared<RecordingLayer>("andar1", log));
    log.clear();

    stack.replaceBottom(std::make_shared<RecordingLayer>("andar2", log));
    ASSERT_EQ(stack.size(), 1u);

    log.clear();
    stack.input();
    EXPECT_EQ(log, "andar2:input "); // o fundo tambem e o topo agora
}

TEST(SceneStackTest, PushingNullptrIsIgnored)
{
    SceneStack stack;
    stack.push(nullptr);
    EXPECT_TRUE(stack.empty());
}

// =============================================================================
// QUEM SAIU no meio do quadro NAO recebe o resto do quadro
// =============================================================================
//
// O `update`/`draw` copiam a pilha antes de iterar, para uma camada poder
// empilhar ou desempilhar durante a propria chamada sem invalidar o iterador.
// A copia sozinha criava um defeito: a camada removida no meio do laco -- que
// ja recebeu `onExit()` -- continuava na copia e ainda recebia `update()`
// depois. Cena morta continuando a simular.

// Camada que, no proprio `update`, tira o topo da pilha.
class PoppingLayer final: public core::IScene
{
public:
    PoppingLayer(std::string name, std::string& log, SceneStack& stack):
        m_name(std::move(name)), m_log(log), m_stack(stack)
    {
    }

    void onEnter() override { m_log += m_name + ":enter "; }
    /// `noDraw = true` move o desempilhamento para o `draw`, para o laco de
    /// desenho ser exercitado pelo mesmo caminho que o de update.
    void desempilharNoDraw() { m_noDraw = true; }

    void update(core::Seconds) override
    {
        m_log += m_name + ":update ";
        if (!m_noDraw)
        {
            m_stack.pop(); // tira o de cima, no meio do laco
        }
    }
    void draw() override
    {
        m_log += m_name + ":draw ";
        if (m_noDraw)
        {
            m_stack.pop();
        }
    }
    void input() override { m_log += m_name + ":input "; }
    void onExit() override { m_log += m_name + ":exit "; }

private:
    std::string  m_name;
    std::string& m_log;
    SceneStack&  m_stack;
    bool         m_noDraw = false;
};

TEST(SceneStackTest, ALayerPoppedMidUpdateDoesNotStillGetUpdated)
{
    std::string log;
    SceneStack  stack;

    stack.push(std::make_shared<PoppingLayer>("mundo", log, stack));
    stack.push(std::make_shared<RecordingLayer>("pausa", log));
    log.clear();

    stack.update(core::Seconds{ 0.016 });

    // O "mundo" atualiza e desempilha a "pausa"; a "pausa" recebe `exit` e NAO
    // pode receber `update` depois disso.
    EXPECT_EQ(log, "mundo:update pausa:exit ");
    EXPECT_EQ(stack.size(), 1u);
}

TEST(SceneStackTest, ALayerPoppedMidDrawDoesNotStillGetDrawn)
{
    std::string log;
    SceneStack  stack;

    const auto mundo = std::make_shared<PoppingLayer>("mundo", log, stack);
    mundo->desempilharNoDraw();
    stack.push(mundo);
    stack.push(std::make_shared<RecordingLayer>("pausa", log));
    log.clear();

    stack.draw();

    // O "mundo" desenha e desempilha a "pausa" no meio do laco de DESENHO: ela
    // recebe `exit` e nao pode desenhar depois.
    EXPECT_EQ(log, "mundo:draw pausa:exit ");
    EXPECT_EQ(stack.size(), 1u);
}

TEST(SceneStackTest, TheLayerBELOWStillGetsTheFrameWhenTheOneAboveLeaves)
{
    // O contrario do teste acima, e ele importa tanto quanto: a guarda nova nao
    // pode ter comido o criterio 1 do gate 18 (a cena de baixo continua rodando).
    std::string log;
    SceneStack  stack;

    stack.push(std::make_shared<RecordingLayer>("mundo", log));
    stack.push(std::make_shared<PoppingLayer>("hud", log, stack));
    log.clear();

    stack.update(core::Seconds{ 0.016 });

    // O "hud" e o topo e desempilha a si mesmo; o "mundo" ja tinha atualizado.
    EXPECT_NE(log.find("mundo:update "), std::string::npos);
    EXPECT_NE(log.find("hud:exit "), std::string::npos);
    EXPECT_EQ(stack.size(), 1u);
}

} // namespace
} // namespace cengine::routing
