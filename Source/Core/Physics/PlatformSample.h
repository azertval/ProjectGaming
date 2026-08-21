// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Physics/Aabb.h"

/**
 * @file Core/Physics/PlatformSample.h
 * @brief Position d'un support mobile (plateforme, `EX-GP-026`) au pas précédent et au pas
 *        courant.
 */

namespace core {

/**
 * @brief Échantillon de position d'une plateforme mobile pour un pas donné.
 *
 * Suffit à la fois au **portage** (delta = `currentBox.min - previousBox.min`) et à la collision
 * **continue** (`core::sweepAabbVsAabb` sur `currentBox`) des entités posées dessus — le
 * personnage (`core::CharacterPhysicsSystem`) et les blocs poussables (`core::BlockController`) —
 * sans que ces deux consommateurs (couches `Ecs`/`Gameplay`) n'aient besoin de connaître
 * `core::PlatformController` lui-même (couche `Gameplay`, au-dessus de `Physics`) : ce type vit
 * dans `Physics`, la couche commune la plus basse dont les deux dépendent déjà.
 */
struct PlatformSample {
    /// Boîte de la plateforme au pas **précédent** (avant le déplacement de ce pas).
    Aabb previousBox;
    /// Boîte de la plateforme au pas **courant** (après le déplacement de ce pas).
    Aabb currentBox;
};

}  // namespace core
