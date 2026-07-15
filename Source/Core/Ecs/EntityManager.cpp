#include "Core/Ecs/EntityManager.h"

namespace core {

/**
 * @brief Crée une nouvelle entité vivante.
 * @return Un handle valide, distinct de toute entité vivante existante.
 */
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

/**
 * @brief Détruit une entité et libère son index pour recyclage.
 *
 * Sans effet si le handle n'est pas vivant (déjà détruit ou périmé). Après
 * l'appel, `isAlive(entity)` est faux pour ce handle.
 *
 * @param entity Handle de l'entité à détruire.
 */
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

/**
 * @brief Indique si un handle désigne une entité actuellement vivante.
 * @param entity Handle à tester.
 * @return `true` si l'index est vivant **et** la génération correspond.
 */
bool EntityManager::isAlive(Entity entity) const {
    if (entity.index >= _generations.size()) {
        return false;
    }
    return _alive[entity.index] && _generations[entity.index] == entity.generation;
}

/**
 * @brief Nombre d'entités actuellement vivantes.
 * @return Le décompte des entités vivantes.
 */
std::size_t EntityManager::aliveCount() const {
    return _aliveCount;
}

}  // namespace core
