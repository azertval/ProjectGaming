// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <functional>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion
#include "Core/Ecs/Entity.h"
#include "Core/Levels/TileType.h"

/**
 * @file Core/Levels/LevelScene.h
 * @brief Projection d'un niveau en entités ECS (une tuile non vide = un sprite).
 */

namespace core {

class World;
class Level;

/**
 * @brief Peuple un `World` d'une **entité par tuile non vide** du niveau.
 *
 * Chaque tuile non `Empty` devient une entité portant un `Transform` (position = colonne, ligne
 * en unités monde) et un `Sprite` (couche 0, région fournie par @p regionForTile). Logique pure,
 * indépendante du rendu (`EX-ARCH-011`) : la correspondance **type de tuile → région d'atlas**
 * est **injectée**, ce qui rend la projection testable sans GPU (une fausse correspondance suffit).
 *
 * @param world         Monde à peupler.
 * @param level         Niveau source.
 * @param regionForTile Correspondance type de tuile → région d'atlas (dépendance de rendu
 * injectée).
 * @param onTileEntity  Rappel invoqué après la création de chaque entité tuile, avec son type et
 *                      sa case. Permet à la couche de présentation d'y attacher ses **propres**
 *                      composants (`hmi::TileSkinTag`, `LOT-42`) sans que `Core` connaisse la
 *                      notion d'habillage (`EX-NFR-011`) — même principe d'injection que
 *                      @p regionForTile. Vide par défaut.
 *
 * Ne peuple **que** les tuiles. Les plans picturaux (`EX-DEC-040`, LOT-69) qui ont remplacé les
 * décors-sprites ne sont **pas** des entités : ce sont des données d'habillage du niveau, composées
 * directement par `HMI` sans passer par l'ECS.
 */
void buildLevelScene(
    World& world, const Level& level, const std::function<AtlasRegion(TileType)>& regionForTile,
    const std::function<void(Entity, TileType, int, int)>& onTileEntity = {});

}  // namespace core
