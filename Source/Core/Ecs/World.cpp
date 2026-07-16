#include "Core/Ecs/World.h"

#include <string>
#include <utility>

#include "Core/Ecs/EcsLog.h"

namespace core {

/**
 * @brief Crée une nouvelle entité vivante.
 * @return Un handle valide.
 */
Entity World::createEntity() {
    return _entities.create();
}

/**
 * @brief Détruit une entité et retire ses composants de toutes les pools.
 * @param entity Entité à détruire ; sans effet si elle n'est pas vivante.
 */
void World::destroyEntity(Entity entity) {
    if (!_entities.isAlive(entity)) {
        // Handle déjà détruit ou périmé : rien à faire.
        return;
    }

    // Retire le composant de cette entité dans chaque pool avant de recycler
    // l'index, pour qu'aucune pool ne référence encore l'entité détruite.
    for (auto& entry : _pools) {
        entry.second->removeIfPresent(entity);
    }
    _entities.destroy(entity);
}

/**
 * @brief Indique si une entité est vivante.
 * @param entity Handle à tester.
 * @return `true` si l'entité est vivante.
 */
bool World::isAlive(Entity entity) const {
    return _entities.isAlive(entity);
}

/**
 * @brief Enregistre un système ; l'ordre d'enregistrement fixe l'ordre d'exécution.
 * @param system Système à ajouter (le `World` en prend possession).
 */
void World::addSystem(std::unique_ptr<ISystem> system) {
    _systems.push_back(std::move(system));
    ECS_LOG_TRACE("Systeme enregistre (total : " + std::to_string(_systems.size()) + ")");
}

/**
 * @brief Exécute tous les systèmes enregistrés, dans l'ordre, pour un pas fixe.
 * @param fixedDelta Durée du pas de simulation, en secondes.
 */
void World::update(float fixedDelta) {
    // Ordre d'exécution déterministe : l'ordre d'enregistrement des systèmes.
    for (const std::unique_ptr<ISystem>& system : _systems) {
        system->update(*this, fixedDelta);
    }
}

/**
 * @brief Nombre de systèmes enregistrés.
 * @return Le décompte des systèmes.
 */
std::size_t World::systemCount() const {
    return _systems.size();
}

}  // namespace core
