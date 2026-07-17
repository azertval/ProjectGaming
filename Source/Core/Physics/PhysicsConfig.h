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
    float moveSpeed = 6.0f;
    /// Accélération de la gravité, en unités/seconde². ⚠️ à affiner au ressenti.
    float gravity = 50.0f;
    /// Vitesse de chute maximale (terminale), en unités/seconde : borne de confort et de sûreté.
    float maxFallSpeed = 25.0f;
};

}  // namespace core
