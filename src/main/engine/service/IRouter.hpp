#pragma once
#include <memory>
#include <string>

#include <engine/IState.hpp>
#include "repository/ISceneRepository.hpp"

class IScene;
class IState;

class IRouter {
public:
    virtual ~IRouter() = default;

    virtual void setNextState(std::unique_ptr<IState> state) const = 0;

    virtual [[nodiscard]] IState& getCurrentStateGame() const = 0;
    virtual [[nodiscard]] std::string getCurrentStateGameName() const = 0;
    virtual [[nodiscard]] std::string getCurrentStateGameCode() const = 0;
    virtual [[nodiscard]] IScene& getCurrentCachedScreen() const = 0;

    virtual [[nodiscard]] IState& getNextStateGame() const = 0;
    virtual [[nodiscard]] std::string getNextStateGameName() const = 0;
    virtual [[nodiscard]] std::string getNextStateGameCode() const = 0;
    virtual [[nodiscard]] IScene& getNextCachedScreen() const = 0;

    virtual [[nodiscard]] bool hasNextScreen() const = 0;

    virtual void goToNextScreen() const = 0;
};
