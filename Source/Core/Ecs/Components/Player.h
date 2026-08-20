// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

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
    /// Charges de **dash** restantes avant de retoucher le sol (`EX-GP-017`, `EX-GP-055`) ;
    /// consommées une par dash, toutes rechargées au contact du sol. Un compteur, et non plus un
    /// booléen : un tableau peut accorder plusieurs dashs par saut (`PhysicsConfig::dashCharges`).
    int dashChargesRemaining = 0;
    /// Durée restante (secondes) du dash en cours ; > 0 pendant la ruée (gravité suspendue).
    float dashTimer = 0.0f;
    /// Sauts **restants** dans le tableau (budget, `EX-GP-024`) ; **-1 = illimité**. Décompté par
    /// la physique ; initialisé au spawn depuis le niveau.
    int jumpsRemaining = -1;
    /// Dashs **restants** dans le tableau (budget, `EX-GP-024`) ; **-1 = illimité**.
    int dashesRemaining = -1;
    /// Masse du personnage, en unité de jeu arbitraire (`EX-GP-019`). Détermine, avec la traînée
    /// de `PhysicsConfig`, la vitesse terminale de chute (poids = masse × gravité effective) ; une
    /// masse plus grande tombe plus vite. Sert aussi de seuil pour les mécanismes sensibles au
    /// poids (plaque de pression, `EX-GP-025`).
    float mass = 1.0f;
    /// Étendue horizontale (bords gauche/droit, unité monde) couverte par la boîte depuis le
    /// **début de la montée courante** (dernier saut), pas seulement le pas précédent — remise à
    /// l'étendue courante à chaque pas où le personnage est au sol ou ne monte pas
    /// (`velocity.y >= 0`), puis étendue à chaque pas de montée. Sans cette mémoire, marcher tout
    /// en sautant sous un arrondi de plafond peut faire « disparaître » une colonne pourtant
    /// franchie plus tôt dans la même montée, avant que le seuil vertical de blocage n'y soit
    /// atteint (`core::resolveCeilingSlopeFollow`, `EX-GP-007`).
    float ascentSweepMinX = 0.0f;
    float ascentSweepMaxX = 0.0f;
    /// Vrai pendant le pas où un saut (sol, coyote, mur ou aérien) vient de se déclencher ; remis
    /// à faux au début du pas suivant. Contrairement aux autres champs, ne porte aucun état de
    /// simulation persistant — c'est un **front** à usage externe (`hmi::GameEvents`, `LOT-60`) :
    /// aucune des minuteries existantes (`jumpBufferTimer`...) ne permet de distinguer un saut
    /// déclenché d'un buffer simplement expiré sans saut.
    bool justJumped = false;
    /// Vrai pendant le pas où le personnage a été **écrasé** par une plateforme mobile contre un
    /// plafond (`EX-GP-026`, cas d'écrasement — décision de cadrage : mortel, comme un danger).
    /// Même nature que `justJumped` : un front à usage externe, remis à faux au début du pas
    /// suivant, sans effet sur la simulation elle-même (l'appelant traduit ce champ en issue
    /// `LevelOutcome::Lost`, `core::evaluateOutcome`).
    bool squished = false;
};

}  // namespace core
