/**
 * @file Core/Ecs/EntityManager.cpp
 * @brief Implémentation du gestionnaire de cycle de vie des entités.
 */

#include "Core/Ecs/EntityManager.h"

namespace core {

Entity EntityManager::create() {
    Entity::Index index;
    if (!_freeIndices.empty()) {
        // Recycle un index libéré : sa génération a déjà été incrémentée à la
        // destruction, ce qui distingue le nouveau handle de l'ancien.
        index = _freeIndices.back();
        _freeIndices.pop_back();
    } else {
        // Nouvel index : agrandit les tableaux internes.
        index = static_cast<Entity::Index>(_generations.size());
        _generations.push_back(0);
        _alive.push_back(false);
    }

    _alive[index] = true;
    ++_aliveCount;
    return Entity{index, _generations[index]};
}

void EntityManager::destroy(Entity entity) {
    if (!isAlive(entity)) {
        // Handle déjà détruit ou périmé : rien à faire (idempotent).
        return;
    }

    _alive[entity.index] = false;
    // Incrémente la génération pour invalider tout handle pointant cet index.
    ++_generations[entity.index];
    _freeIndices.push_back(entity.index);
    --_aliveCount;
}

bool EntityManager::isAlive(Entity entity) const {
    if (entity.index >= _generations.size()) {
        return false;
    }
    return _alive[entity.index] && _generations[entity.index] == entity.generation;
}

std::size_t EntityManager::aliveCount() const {
    return _aliveCount;
}

}  // namespace core
