// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/Level.h"
#include "Core/Math/Vector2.h"

/**
 * @file Core/Gameplay/PlatformPath.h
 * @brief Route d'une plateforme mobile : polyligne et position déterministe le long de celle-ci
 *        (`EX-GP-026`, `EX-GP-054`).
 */

namespace core {

/**
 * @brief Sommets de la route de @p config en unités monde (coin **haut-gauche** de chaque case).
 *
 * Le premier point est `startPosition`, suivi de `waypoints` dans l'ordre. En mode
 * `PlatformPathMode::Loop`, le point de départ est **répété en fin de liste** pour matérialiser le
 * segment de fermeture : la liste décrit donc toujours le parcours complet d'un cycle « aller »,
 * qu'il soit ouvert (aller-retour) ou fermé (circuit).
 *
 * Fonction pure, partagée par `core::PlatformController` (mouvement) et par l'overlay d'édition
 * (`hmi::DraftRenderer`) : la trajectoire dessinée dans l'éditeur est **littéralement** celle que
 * le gameplay parcourt, jamais une réimplémentation parallèle.
 */
[[nodiscard]] std::vector<Vector2> platformPathPoints(const MovingPlatformConfig& config);

/**
 * @brief Route **précalculée** d'une plateforme : sommets et longueurs cumulées.
 *
 * Construite une fois (au chargement) puis interrogée à chaque pas : `PlatformController::boxAt`
 * et `previousBoxAt` sont appelées plusieurs fois par pas et par consommateur
 * (`CharacterPhysicsSystem`, `BlockController`, `hmi::GameSession`), on ne recalcule donc jamais
 * les longueurs à l'exécution.
 */
struct PlatformPath {
    /// Sommets du parcours (`platformPathPoints`), au moins un.
    std::vector<Vector2> points;
    /// Longueurs cumulées depuis `points[0]` ; même taille que `points`, `cumulative[0] == 0`.
    std::vector<double> cumulative;
    /// Longueur totale du parcours décrit par `points` (circuit fermé compris en mode `Loop`).
    double totalLength = 0.0;
    /// Longueur d'un **cycle complet** : `2 × totalLength` en aller-retour (l'aller puis le
    /// retour), `totalLength` en circuit fermé. Nulle si la plateforme est immobile.
    double cycleLength = 0.0;
};

/// Précalcule la route de @p config (voir `PlatformPath`).
[[nodiscard]] PlatformPath buildPlatformPath(const MovingPlatformConfig& config);

/**
 * @brief Position (coin haut-gauche, unités monde) atteinte après avoir parcouru @p travelled
 *        unités le long de @p path depuis son point de départ.
 *
 * @p travelled est replié sur le cycle (modulo positif, valeurs négatives admises) puis, en
 * aller-retour, mis en miroir au-delà de la moitié du cycle. Une route de longueur nulle renvoie
 * toujours le point de départ. Fonction pure : aucune accumulation d'un appel à l'autre
 * (`EX-NFR-002`).
 */
[[nodiscard]] Vector2 platformPositionAt(const PlatformPath& path, double travelled) noexcept;

}  // namespace core
