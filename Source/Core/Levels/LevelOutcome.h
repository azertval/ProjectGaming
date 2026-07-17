#pragma once

#include "Core/Physics/Aabb.h"

/**
 * @file Core/Levels/LevelOutcome.h
 * @brief Issue d'un niveau (en cours / gagné / perdu) évaluée depuis la position du personnage.
 */

namespace core {

class Level;

/// Issue courante d'un niveau, déduite de la position du personnage.
enum class LevelOutcome {
    Playing,  ///< Partie en cours.
    Won,      ///< Le personnage a atteint la sortie (`EX-GP-030`).
    Lost      ///< Le personnage a touché un danger ou est sorti par le bas (`EX-GP-031`).
};

/**
 * @brief Détermine l'issue du niveau à partir de la **boîte** du personnage.
 *
 * Fonction **pure** d'observation (ne modifie rien) : elle classe l'état courant, sans déclencher
 * de transition (le retour au menu ou le redémarrage relèvent de l'intégration). Priorité
 * **déterministe** en cas de recouvrement simultané : l'**échec l'emporte sur le succès**.
 *
 * @param playerBox Boîte englobante du personnage, en unités monde.
 * @param level     Niveau courant (sortie, tuiles `Danger`, limites de la grille).
 * @return `Lost` si contact avec un `Danger` ou sortie par le bas ; sinon `Won` si la sortie est
 *         recouverte ; sinon `Playing`.
 */
[[nodiscard]] LevelOutcome evaluateOutcome(const Aabb& playerBox, const Level& level);

}  // namespace core
