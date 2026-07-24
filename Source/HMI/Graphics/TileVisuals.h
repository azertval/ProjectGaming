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
 * **Unique** correspondance type → région, utilisée par `GameScreen` (rendu d'un niveau joué),
 * `EditorScreen` (canevas de l'éditeur, LOT-14) et la palette de l'éditeur (`hmi::TilePalette`) :
 * le canevas de l'éditeur est un aperçu **fidèle** (même forme — triangle/courbe pour les pentes/
 * arrondis, `EX-GP-003`/`EX-GP-004`, `core::slopeSurfaceHeight` — et, combinée à
 * `core::tileVisualScale` côté appelant, même taille pour les blocs réduits, `EX-GP-005`) de ce
 * que le joueur verra en jouant le niveau (`P`), sans simplification qui pourrait induire en
 * erreur pendant l'édition.
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
 * @brief Position, dans la grille de tuiles procédurale, réservée à un type de tuile à silhouette
 *        inclinée/courbe : pentes/arrondis convexes ou concaves de **sol**, suivables
 *        (`core::isFollowableSurface`, `EX-GP-003`/`EX-GP-004`/`EX-GP-007`) ou de **plafond**,
 *        solides (`EX-GP-006`/`EX-GP-007`).
 *
 * Seule source de vérité pour cette association : utilisée par `regionForTile` (couleur) **et**
 * par `TextureAtlas` (génération d'un masque de forme triangulaire/courbe à cet emplacement,
 * plutôt qu'un carré plein, pour que l'affichage corresponde à la silhouette réelle — hitbox pour
 * les variantes de sol via `core::slopeSurfaceHeight`, silhouette purement visuelle pour les
 * variantes de plafond, solides par ailleurs) — dupliquer ces coordonnées aux deux endroits
 * risquerait de les laisser diverger silencieusement.
 * @param type Type de tuile.
 * @return La position dans la grille, ou `std::nullopt` si @p type n'a pas de silhouette inclinée/
 *         courbe.
 */
[[nodiscard]] std::optional<AtlasGridPosition> slopeTileGridPosition(core::TileType type);

}  // namespace hmi
