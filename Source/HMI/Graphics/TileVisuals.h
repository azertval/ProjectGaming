#pragma once

#include <optional>

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

/// Position (colonne, ligne) d'une tuile dans la grille procédurale de `TextureAtlas`.
struct AtlasGridPosition {
    int column = 0;
    int row = 0;
};

/**
 * @brief Position, dans la grille de tuiles procédurale, réservée à un type de tuile à profil
 *        **suivable** (pente/arrondi, `core::isFollowableSurface`, `EX-GP-003`/`EX-GP-004`).
 *
 * Seule source de vérité pour cette association : utilisée par `regionForTile` (couleur) **et**
 * par `TextureAtlas` (génération d'un masque de forme triangulaire/courbe à cet emplacement,
 * plutôt qu'un carré plein, pour que l'affichage corresponde à la hitbox réelle décrite par
 * `core::slopeSurfaceHeight`) — dupliquer ces coordonnées aux deux endroits risquerait de les
 * laisser diverger silencieusement.
 * @param type Type de tuile.
 * @return La position dans la grille, ou `std::nullopt` si @p type n'a pas de profil suivable.
 */
[[nodiscard]] std::optional<AtlasGridPosition> slopeTileGridPosition(core::TileType type);

}  // namespace hmi
