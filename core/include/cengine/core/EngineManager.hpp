#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include <cengine/core/IGameManager.hpp>
#include <cengine/core/IWindowManager.hpp>
#include <cengine/core/Time.hpp>

namespace cengine::core {

/**
 * @brief O coração da engine: o game loop, com *fixed timestep*.
 *
 * É a única classe concreta do `core`. Nasce por uma de duas factories
 * nomeadas — o modo é decidido na construção e expresso no call site
 * (ver .ai/task/21):
 *
 * - **`owned(window, game)` — modo próprio**: a engine é dona da janela e do
 *   loop; `start()` bloqueia e dirige até o jogo pedir para sair (terminal,
 *   FTXUI, The-Forge como biblioteca).
 * - **`hosted(game)` — modo hospedado**: um host com inversão de controle
 *   (The-Forge `IApp`, editor, browser) é dono do loop e chama `frame(dt)`
 *   uma vez por quadro. Não há janela da engine: janela, pump de mensagens e
 *   pacing são do host — `start()` lança `std::logic_error` nesse modo.
 *   Ver .ai/task/15.
 *
 * Cada quadro executa: `game.onEnter()` → `game.input()` → `game.update(dt)`
 * (0..N vezes) → `game.render()` → `game.onExit()`. No modo próprio o quadro
 * da janela envolve as fases — `window.update()` antes (eventos de SO,
 * preparo do quadro) e `window.present()` depois (fechar/apresentar o que foi
 * desenhado, inclusive no último quadro) — e `shouldExit()` encerra o loop
 * com `cleanup()`; no modo hospedado, `frame()` devolve false quando o jogo
 * pediu saída e o host decide o shutdown (e chama `cleanup()` no teardown
 * dele).
 *
 * ## Tempo (padrão "fix your timestep")
 * O tempo do quadro (medido pelo relógio no modo próprio; informado pelo host
 * no modo hospedado) é acumulado; a simulação avança em passos FIXOS de
 * `fixedDt` (default 1/60 s): num quadro curto, `update` pode não rodar — mas
 * o `render` acontece mesmo assim; num quadro longo, roda várias vezes —
 * sempre com o mesmo dt, o que mantém a simulação determinística e estável
 * (pré-requisito de física). O tempo de quadro é limitado a `maxFrameTime`
 * (default 250 ms) para evitar a espiral da morte (quadro lento → mais passos
 * → quadro mais lento); se o host aplicar um clamp próprio antes, os dois se
 * compõem sem conflito. Render não interpola entre passos (fora do escopo —
 * ver task 14).
 *
 * A fonte de tempo é injetável (`clockNow`) para permitir testes
 * determinísticos do loop, sem tempo real; o default é o relógio monotônico
 * `std::chrono::steady_clock`. No modo hospedado ela não é consultada — o
 * tempo vem por parâmetro.
 *
 * @code
 * // modo próprio: a engine dirige
 * auto engine = cengine::core::EngineManager::owned(
 *     std::make_unique<MyWindowManager>(),
 *     std::make_unique<MyGameManager>());
 * engine.start(); // bloqueia até o jogo pedir para sair
 *
 * // modo hospedado: o host dirige (ex.: dentro do Draw() de um IApp)
 * auto hosted = cengine::core::EngineManager::hosted(
 *     std::make_unique<MyGameManager>());
 * // ... por quadro do host:
 * if (!hosted.frame(dtDoHost)) {
 *     // o host encerra o loop dele e chama hosted.cleanup() no teardown
 * }
 * @endcode
 */
class EngineManager {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using ClockNowFn = std::function<TimePoint()>;

    static constexpr Seconds kDefaultFixedDt{1.0 / 60.0};
    static constexpr Seconds kDefaultMaxFrameTime{0.25};

    /**
     * @brief Modo próprio: a engine é dona da janela e do loop (`start()`).
     *
     * @param windowManager plataforma gráfica (posse transferida à engine).
     * @param gameManager   regras/telas do jogo (posse transferida à engine).
     * @param fixedDt       passo fixo da simulação (> 0; default 1/60 s).
     * @param maxFrameTime  teto do tempo de quadro acumulável (> 0; default
     *                      250 ms) — proteção contra a espiral da morte.
     * @param clockNow      fonte de tempo (default: relógio monotônico).
     * @throws std::invalid_argument se @p windowManager ou @p gameManager for
     *         nulo, ou se @p fixedDt ou @p maxFrameTime não for positivo.
     */
    static EngineManager owned(
        std::unique_ptr<IWindowManager> windowManager,
        std::unique_ptr<IGameManager> gameManager,
        Seconds fixedDt = kDefaultFixedDt,
        Seconds maxFrameTime = kDefaultMaxFrameTime,
        ClockNowFn clockNow = Clock::now);

    /**
     * @brief Modo hospedado: o host é dono da janela e do loop (`frame(dt)`).
     *
     * Não há janela do lado da engine e a fonte de tempo não é consultada —
     * o tempo de quadro vem por parâmetro em `frame()`. `start()` lança
     * `std::logic_error` em uma engine construída por esta factory.
     *
     * @param gameManager   regras/telas do jogo (posse transferida à engine).
     * @param fixedDt       passo fixo da simulação (> 0; default 1/60 s).
     * @param maxFrameTime  teto do tempo de quadro acumulável (> 0; default
     *                      250 ms).
     * @throws std::invalid_argument se @p gameManager for nulo, ou se
     *         @p fixedDt ou @p maxFrameTime não for positivo.
     */
    static EngineManager hosted(
        std::unique_ptr<IGameManager> gameManager,
        Seconds fixedDt = kDefaultFixedDt,
        Seconds maxFrameTime = kDefaultMaxFrameTime);

    EngineManager(EngineManager&&) noexcept = default;
    EngineManager& operator=(EngineManager&&) noexcept = default;

    ~EngineManager() = default;

    /// Inicializa a janela e entra no game loop. Bloqueia até `shouldExit()`.
    /// Modo próprio — em uma engine `hosted()` lança `std::logic_error`.
    void start();

    /**
     * Modo hospedado: executa UM quadro completo — `onEnter()` → `input()` →
     * `update(fixedDt)` 0..N vezes (consumindo @p frameTime no acumulador
     * interno, com o clamp de `maxFrameTime`) → `render()` → `onExit()`.
     *
     * O host (dono do loop) chama uma vez por quadro com o tempo decorrido
     * que ele mesmo mediu; a janela NÃO é tocada (é do host). `start()` e
     * `frame()` compartilham a mesma lógica de quadro — as garantias de
     * fixed timestep valem nos dois modos.
     *
     * @param frameTime tempo decorrido desde o quadro anterior, medido pelo
     *        host (será limitado a `maxFrameTime`).
     * @return false quando o jogo pediu para sair (`shouldExit()`): o host
     *         deve parar de chamar, encerrar o loop dele e invocar
     *         `cleanup()` no teardown.
     */
    [[nodiscard]] bool frame(Seconds frameTime);

    /// Libera recursos do jogo e da janela. Invocado ao final de `start()`;
    /// no modo hospedado é responsabilidade do host, no teardown dele.
    void cleanup() const;

private:
    // Só as factories constroem: `owned()` garante janela não nula; `hosted()`
    // é o ÚNICO caminho sem janela (m_windowManager nulo é detalhe interno,
    // nunca API pública — ver .ai/task/21).
    EngineManager(
        std::unique_ptr<IWindowManager> windowManager,
        std::unique_ptr<IGameManager> gameManager,
        Seconds fixedDt,
        Seconds maxFrameTime,
        ClockNowFn clockNow);

    void run();

    std::unique_ptr<IWindowManager> m_windowManager;
    std::unique_ptr<IGameManager> m_gameManager;
    Seconds m_fixedDt;
    Seconds m_maxFrameTime;
    ClockNowFn m_clockNow;
    Seconds m_accumulator{0}; // resto de tempo entre quadros (fixed timestep)
    bool m_isRunning;
};

} // namespace cengine::core
