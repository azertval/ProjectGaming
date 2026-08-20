// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "Core/Math/Vector2.h"

/**
 * @file Core/Levels/Decor.h
 * @brief Objet de décor libre (hors grille) d'un niveau (`EX-DEC-001`, LOT-49).
 */

namespace core {

/**
 * @brief Couche de superposition d'un décor (`EX-DEC-002`).
 *
 * `Core` ne connaît pas `hmi::RenderLayer` (`EX-NFR-011`) : la projection vers les calques de
 * rendu (`Background`/`Decor`/`Foreground`) est le travail de `HMI` (`hmi::decorRenderLayer`,
 * TACHE-02).
 */
enum class DecorLayer {
    /// Arrière-plan : sous les tuiles.
    Background,
    /// Décor : entre les tuiles et le personnage.
    Decor,
    /// Premier plan : au-dessus du personnage (`EX-DEC-002`).
    Foreground,
};

/**
 * @brief Décor **libre** : nom d'asset, transform en unités monde flottantes, couche et
 *        indicateur statique/manipulable (`EX-DEC-001`, `EX-DEC-005`).
 *
 * Donnée pure (`EX-ARCH-011`), sur le patron `Mechanism`/`DangerLink`/`TileTextureOverride`
 * (`Source/Core/Levels/Level.h`) : vecteur annexe de `Level`/`LevelDraft`, mais keyé par **rang**
 * dans le vecteur plutôt que par `GridPosition` — un décor n'est pas calé sur la grille. L'ordre
 * dans le vecteur est **significatif** : il détermine la superposition à l'intérieur d'une même
 * couche (TACHE-02).
 *
 * Le nom d'asset est une simple chaîne : `Core` ne vérifie pas son existence (`EX-NFR-011`), un
 * décor pointant un fichier absent reste un niveau valide (`EX-LVL-004` ne le rejette pas).
 *
 * `manipulable` est stocké et sérialisé dès ce lot mais **sans effet** : la manipulation en jeu
 * (`EX-DEC-020`/`EX-DEC-021`) reste post-programme.
 */
struct Decor {
    /// Nom de l'asset affiché (résolu côté `HMI`).
    std::string assetName;
    /// Position en unités monde, **flottante** : aucune contrainte de grille (`EX-DEC-001`).
    Vector2 position;
    /// Facteurs d'échelle horizontal et vertical (1 = taille nominale).
    Vector2 scale{1.0f, 1.0f};
    /// Rotation autour de la position, en radians (sort au rendu tranché en TACHE-02).
    float rotation = 0.0f;
    /// Couche de superposition (`EX-DEC-002`).
    DecorLayer layer = DecorLayer::Decor;
    /// `true` si ce décor sera manipulable en jeu (`EX-DEC-005`) ; sans effet avant `EX-DEC-020`.
    bool manipulable = false;
};

}  // namespace core
