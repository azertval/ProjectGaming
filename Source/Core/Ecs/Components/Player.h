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
 * (`CharacterPhysicsSystem`) que celle-ci est pilotée par l'intention d'entrée
 * (::core::PlayerInput) et soumise à la gravité. L'état « au sol » est **calculé par la physique**
 * (mis à `true` quand un contact bloquant se produit sous le personnage) et sert la caméra et le
 * **saut** (`EX-GP-013`). Les minuteries `coyoteTimer`/`jumpBufferTimer` portent l'état de *game
 * feel* du saut (coyote time, jump buffering), décompté par la physique au pas fixe.
 */
struct Player {
    /// Vrai si le personnage repose sur une surface solide ; calculé par la physique.
    bool grounded = false;
    /// Temps restant (secondes) pendant lequel un saut est encore permis après avoir quitté le
    /// sol (*coyote time*) ; rechargé au contact du sol, décompté par la physique au pas fixe.
    float coyoteTimer = 0.0f;
    /// Temps restant (secondes) de validité d'un saut pré-appuyé avant l'atterrissage
    /// (*jump buffering*) ; rechargé à l'appui, décompté par la physique au pas fixe.
    float jumpBufferTimer = 0.0f;
};

}  // namespace core
