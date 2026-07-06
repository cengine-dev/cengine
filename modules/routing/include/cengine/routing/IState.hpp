#pragma once

#include <memory>
#include <string>

namespace cengine::routing {

/**
 * @brief Um estado do jogo, identificado por um código, ao qual corresponde uma
 *        cena (`IScene`).
 *
 * O `getCode()` é a chave usada pelo roteador/repositório para localizar a cena
 * do estado (ex.: `"main_menu"`, `"gameplay"`, `cengine::routing::kExitStateCode`).
 * O jogo define seus próprios estados implementando esta interface.
 *
 * `clone()` segue o padrão **Prototype**: o repositório guarda cópias
 * independentes do estado atual e do próximo, sem acoplar-se ao tipo concreto —
 * por isso o estado deve saber se duplicar.
 */
class IState {
public:
    virtual ~IState() = default;

    /// Código único do estado; chave da cena correspondente. Ex.: "gameplay".
    [[nodiscard]] virtual std::string getCode() const = 0;

    /// Nome legível do estado (para logs/depuração).
    [[nodiscard]] virtual std::string getName() const = 0;

    /// Cria uma cópia independente deste estado (padrão Prototype).
    [[nodiscard]] virtual std::unique_ptr<IState> clone() const = 0;
};

} // namespace cengine::routing
