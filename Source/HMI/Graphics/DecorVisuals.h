// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "Core/Levels/Decor.h"
#include "HMI/Graphics/RenderLayer.h"

/**
 * @file HMI/Graphics/DecorVisuals.h
 * @brief Projection couche → calque et résolution d'apparence des décors (LOT-49 TACHE-02).
 */

namespace hmi {

struct SceneTextures;

/// Sous-dossier des décors, relatif au dossier d'assets (`LOT-49`).
inline const std::string DECORS_SUBDIRECTORY = "Decors/";

/**
 * @brief Projette une couche de décor (`Core`) vers son calque de rendu (`HMI`), `EX-DEC-002`.
 *
 * Fonction **pure**, triviale mais explicite — c'est le point où la frontière `Core`/`HMI` est
 * tenue (`EX-NFR-011`) : `Core` ignore tout de `hmi::RenderLayer`.
 *
 * `core::DecorLayer::Background` et `core::DecorLayer::Decor` projettent tous deux sur le **même**
 * `RenderLayer::Decor` : un seul calque « arrière-plan » a été réservé côté rendu (`LOT-40`), les
 * trois calques du vocabulaire de conception (arrière-plan/décor/premier plan, `decors.md`) ne
 * correspondant qu'à **deux** intentions de superposition avec les tuiles et le personnage —
 * derrière, ou devant. Le rang dans `core::Level::decors()` (`core::Sprite::layer`) reste le tri
 * fin qui distingue deux décors du même calque, y compris entre `Background` et `Decor`.
 * `core::DecorLayer::Foreground` projette sur `RenderLayer::Foreground`, au-dessus du personnage.
 * @param layer Couche de décor, côté `Core`.
 * @return Le calque de rendu correspondant.
 */
[[nodiscard]] RenderLayer decorRenderLayer(core::DecorLayer layer) noexcept;

/**
 * @brief Marque de présentation portée par une entité décor, pour résoudre son apparence et sa
 *        parallaxe.
 *
 * Vit côté `HMI` (comme `hmi::TileSkinTag`) : `Core` ne connaît pas les noms de fichiers d'assets
 * (`EX-NFR-011`). Attachée par le rappel `onDecorEntity` de `core::buildLevelScene`.
 */
struct DecorVisualTag {
    /// Nom de l'asset désigné par le décor (`core::Decor::assetName`).
    std::string assetName;
    /// Couche d'origine du décor (`core::Decor::layer`) : conservée telle quelle (distincte du
    /// `RenderLayer` projeté, `hmi::decorRenderLayer`) car `Background` et `Decor` projettent sur
    /// le même calque de rendu mais portent des facteurs de parallaxe différents
    /// (`hmi::parallaxFactor`, `LOT-49` TACHE-03).
    core::DecorLayer layer = core::DecorLayer::Decor;
};

/// Apparence résolue d'un décor : quelle texture lier, et sa taille en pixels.
struct DecorAppearance {
    /// Texture à lier : l'asset réel si chargé, le damier de repli sinon.
    TextureHandle texture = nullptr;
    /// Largeur de la texture liée, en pixels.
    int pixelWidth = 0;
    /// Hauteur de la texture liée, en pixels.
    int pixelHeight = 0;
};

/**
 * @brief Résout l'apparence d'un décor : asset désigné si chargé, damier de repli sinon.
 *
 * Fonction **pure** (aucun accès disque/GPU), testable sans GPU (`EX-NFR-004`) : la résolution
 * disque/GPU a déjà eu lieu en amont (`hmi::sceneTextures`, qui journalise l'avertissement en cas
 * d'asset introuvable, `EX-NFR-040`) — cette fonction ne fait que choisir dans ce qui a déjà été
 * chargé. Contrairement au fond de niveau (`LOT-44`, absence = état normal), un décor **désigné**
 * est toujours censé exister : son absence retombe donc directement sur le damier, à sa taille
 * normale (jamais sur une autre apparence).
 * @param tag      Marque de présentation du décor (nom d'asset).
 * @param textures Textures déjà chargées par la composition.
 * @return La texture à lier et ses dimensions en pixels.
 */
[[nodiscard]] DecorAppearance resolveDecorAppearance(const DecorVisualTag& tag,
                                                     const SceneTextures& textures) noexcept;

}  // namespace hmi
