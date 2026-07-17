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
    /// Sauts **aériens** restants (double/multi saut, `EX-GP-015`) ; rechargé au contact du sol.
    int airJumpsRemaining = 0;
    /// Orientation courante : -1 (gauche) / +1 (droite) ; sert de direction de dash par défaut.
    float facing = 1.0f;
    /// Sens du **mur** au contact en l'air : -1 (mur à gauche) / +1 (à droite) / 0 (aucun),
    /// calculé par la physique (`EX-GP-016`).
    float wallDirection = 0.0f;
    /// Temps restant (secondes) de **verrouillage** du contrôle horizontal après un wall jump
    /// (la vitesse d'éjection persiste tant qu'il n'est pas écoulé).
    float wallJumpLockTimer = 0.0f;
    /// Le **dash** est-il disponible ? Consommé à l'usage, rechargé au contact du sol
    /// (`EX-GP-017`).
    bool dashAvailable = false;
    /// Durée restante (secondes) du dash en cours ; > 0 pendant la ruée (gravité suspendue).
    float dashTimer = 0.0f;
    /// Sauts **restants** dans le tableau (budget, `EX-GP-024`) ; **-1 = illimité**. Décompté par
    /// la physique ; initialisé au spawn depuis le niveau.
    int jumpsRemaining = -1;
    /// Dashs **restants** dans le tableau (budget, `EX-GP-024`) ; **-1 = illimité**.
    int dashesRemaining = -1;
};

}  // namespace core
