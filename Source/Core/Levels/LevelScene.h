#pragma once

#include <cstddef>
#include <functional>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion
#include "Core/Ecs/Entity.h"
#include "Core/Levels/Decor.h"
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
 * @param regionForTile Correspondance type de tuile → région d'atlas (dépendance de rendu injectée).
 * @param onTileEntity  Rappel invoqué après la création de chaque entité tuile, avec son type et
 *                      sa case. Permet à la couche de présentation d'y attacher ses **propres**
 *                      composants (`hmi::TileSkinTag`, `LOT-42`) sans que `Core` connaisse la
 *                      notion d'habillage (`EX-NFR-011`) — même principe d'injection que
 *                      @p regionForTile. Vide par défaut.
 *
 * Peuple également une entité par **décor libre** (`EX-DEC-001`, LOT-49), après les tuiles :
 * `Transform` en unités monde flottantes (position/échelle/rotation du décor, **jamais** calées sur
 * la grille), et `Sprite` dont `layer` porte le rang du décor dans `level.decors()` (tri fin
 * intra-calque, TACHE-02). `Core` ne pose aucune région d'atlas ni apparence : la résolution
 * d'asset est entièrement injectée via @p onDecorEntity, même principe que @p onTileEntity.
 * @param onDecorEntity Rappel invoqué après la création de chaque entité décor, avec le décor
 *                      source et son rang dans `level.decors()`. Permet à `HMI` d'y attacher ses
 *                      propres composants (`hmi::DecorVisualTag`, `hmi::RenderLayerTag`) sans que
 *                      `Core` connaisse la notion d'apparence. Vide par défaut.
 */
void buildLevelScene(
    World& world, const Level& level, const std::function<AtlasRegion(TileType)>& regionForTile,
    const std::function<void(Entity, TileType, int, int)>& onTileEntity = {},
    const std::function<void(Entity, const Decor&, std::size_t)>& onDecorEntity = {});

}  // namespace core
