#pragma once

/**
 * @file Core/Ecs/Components/Player.h
 * @brief Composant marqueur du personnage jouable (données pures).
 */

namespace core {

/**
 * @brief Marque l'entité **contrôlable par le joueur** et porte son état de contact au sol.
 *
 * Donnée pure sans logique (`EX-ARCH-011`). Sa présence sur une entité signale à la physique
 * (`CharacterPhysicsSystem`) que celle-ci est pilotée par l'intention d'entrée (::core::PlayerInput)
 * et soumise à la gravité. L'état « au sol » est **calculé par la physique** (mis à `true` quand un
 * contact bloquant se produit sous le personnage) et sert la caméra ainsi que le futur saut
 * (`EX-GP-013`, hors périmètre de ce lot).
 */
struct Player {
    /// Vrai si le personnage repose sur une surface solide ; calculé par la physique.
    bool grounded = false;
};

}  // namespace core
