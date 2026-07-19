#pragma once

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/TileVisuals.h
 * @brief Correspondance type de tuile → région d'atlas, partagée entre le jeu et l'éditeur.
 */

namespace hmi {

class TextureAtlas;

/**
 * @brief Région d'atlas (couleur procédurale) associée à un type de tuile.
 *
 * Utilisée à la fois par `GameScreen` (rendu d'un niveau joué) et `EditorScreen` (rendu du
 * niveau en cours d'édition, LOT-14) : une **unique** correspondance type → couleur garantit que
 * l'aperçu dans l'éditeur correspond exactement à ce que le joueur verra en jeu.
 * @param type  Type de tuile (`Empty` renvoie une région arbitraire, jamais dessinée en pratique
 *              — les cases vides ne sont pas rendues par l'appelant).
 * @param atlas Atlas fournissant les régions.
 * @return La région d'atlas à échantillonner pour ce type.
 */
[[nodiscard]] core::AtlasRegion regionForTile(core::TileType type, const TextureAtlas& atlas);

}  // namespace hmi
