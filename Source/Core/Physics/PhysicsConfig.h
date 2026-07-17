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
};

}  // namespace core
