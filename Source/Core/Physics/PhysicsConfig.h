// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Physics/PhysicsConfig.h
 * @brief Constantes de réglage de la physique du personnage (données pures).
 */

namespace core {

/**
 * @brief Paramètres réglables de la physique du personnage, regroupés pour faciliter le *tuning*.
 *
 * Donnée pure (`EX-ARCH-011`) : le `CharacterPhysicsSystem` la lit, ne la modifie pas. Les valeurs
 * par défaut sont des points de départ **à affiner par tests** (le ressenti « game feel » est
 * marqué ⚠️ dans la spec `gameplay.md`). Unités monde : une tuile = 1 unité, `y` vers le bas.
 */
struct PhysicsConfig {
    /// Vitesse horizontale à pleine intention, en unités/seconde (`EX-GP-010`). ⚠️ à affiner.
    float moveSpeed = 3.0f;
    /// Accélération de la gravité, en unités/seconde². ⚠️ à affiner au ressenti.
    float gravity = 50.0f;
    /// Coefficient de traînée verticale en chute, en 1/s (`EX-GP-019`). La vitesse terminale
    /// **émerge** de l'équilibre poids/traînée (poids = masse × gravité effective) plutôt que
    /// d'être plafonnée : à masse par défaut (1,0), `90 / 3.6 = 25` unités/s, continuité avec
    /// l'ancien plafond fixe. ⚠️ à affiner.
    float fallDragCoefficient = 3.6f;
    /// Vitesse verticale de l'impulsion de saut, en unités/seconde (montée). Avec `gravity = 50`,
    /// ~2,25 tuiles de hauteur / apex ~0,3 s. ⚠️ à affiner (viser ~2,5 tuiles, apex ~0,35 s).
    float jumpSpeed = 15.0f;
    /// Fenêtre de *coyote time*, en secondes : saut permis peu après avoir quitté un bord. ⚠️.
    float coyoteTime = 0.08f;
    /// Fenêtre de *jump buffering*, en secondes : saut pré-appuyé honoré à l'atterrissage. ⚠️.
    float jumpBufferTime = 0.12f;
    /// Fraction de vitesse ascendante **conservée** au relâchement du saut (hauteur variable) :
    /// 0 = coupe nette, 1 = pas de coupe. ⚠️ à affiner (~0,5).
    float jumpCutFactor = 0.5f;
    /// Nombre de sauts **aériens** supplémentaires (double/multi saut, `EX-GP-015`). Défaut 1.
    int airJumps = 1;
    /// Vitesse de descente **plafonnée** le long d'un mur (wall slide), en unités/s. ⚠️.
    float wallSlideSpeed = 4.0f;
    /// Composante horizontale de l'impulsion de **wall jump** (éjection opposée au mur), unités/s.
    float wallJumpSpeedX = 12.0f;
    /// Composante verticale (montée) de l'impulsion de **wall jump**, en unités/s.
    float wallJumpSpeedY = 14.0f;
    /// Durée (secondes) de verrouillage du contrôle horizontal après un wall jump. ⚠️.
    float wallJumpLockTime = 0.18f;
    /// Vitesse de la ruée de **dash**, en unités/s (`EX-GP-017`). ⚠️.
    float dashSpeed = 15.0f;
    /// Durée (secondes) du dash ; distance ≈ dashSpeed × dashDuration. ⚠️.
    float dashDuration = 0.15f;
    /// Nombre de dashs utilisables entre deux contacts avec le sol (`EX-GP-017`, `EX-GP-055`).
    /// Défaut 1 : un seul dash par saut, le comportement historique.
    int dashCharges = 1;
    /// Multiplicateur de gravité en **chute** (> 1) : la chute est plus rapide que la montée
    /// (`EX-GP-018`). ⚠️ à affiner (~1,8).
    float fallGravityMultiplier = 1.8f;
    /// Seuil de vitesse verticale (unités/s) sous lequel on est « à l'apex » (flottement). ⚠️.
    float apexThreshold = 4.0f;
    /// Multiplicateur de gravité **près de l'apex** (< 1) : contrôle flottant au sommet. ⚠️.
    float apexGravityMultiplier = 0.5f;
    /// Multiplicateur de gravité **supplémentaire** en chute quand « bas » est maintenu
    /// (*fast-fall*, > 1). ⚠️ à affiner (~1,6).
    float fastFallMultiplier = 1.6f;
};

}  // namespace core
