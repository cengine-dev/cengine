#include <cengine/routing/SceneRepository.hpp>

namespace cengine::routing {

void SceneRepository::registerFactory(const std::string &name, std::function<std::unique_ptr<core::IScene>()> factory)
{
    // Uma factory vazia so falharia na PRIMEIRA VEZ em que a cena fosse pedida
    // -- possivelmente muitos minutos de jogo depois do registro. O erro
    // pertence a quem registrou.
    if (!factory) {
        throw std::invalid_argument("SceneRepository::registerFactory: factory must not be empty for: " + name);
    }
    m_factories[name] = std::move(factory);
}

core::IScene &SceneRepository::getScene(const std::string &name) {
    // Se já estiver instanciada, retorna
    if (const auto it = m_scenes.find(name); it != m_scenes.end()) {
        return *(it->second);
    }

    // Caso contrário, instancia sob demanda via factory
    if (const auto factoryIt = m_factories.find(name); factoryIt != m_factories.end()) {
        auto criada = factoryIt->second();
        // **A factory pode devolver nulo**, e ate aqui isso virava um
        // `*(it->second)` num ponteiro nulo -- comportamento indefinido no
        // ponto mais quente do laco. Dizer o nome da cena e o que torna o erro
        // acionavel.
        if (!criada) {
            throw std::runtime_error("Scene factory returned null: " + name);
        }
        const auto [it, inserted] = m_scenes.emplace(name, std::move(criada));
        return *(it->second);
    }

    throw std::runtime_error("Scene not found: " + name);
}

void SceneRepository::unloadScene(const std::string &name) {
    m_scenes.erase(name);
}

void SceneRepository::unloadAll() {
    m_scenes.clear();
}

} // namespace cengine::routing
