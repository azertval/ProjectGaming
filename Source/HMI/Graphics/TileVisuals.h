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
 * Utilisée par `GameScreen` (rendu d'un niveau joué, fidèle à la hitbox réelle) et par la palette
 * de l'éditeur (`hmi::TilePalette`, sélection d'un outil — chaque entrée porte aussi un libellé
 * textuel, donc pas d'ambiguïté à garder des couleurs/formes distinctes). Pour le **canevas**
 * de l'éditeur (la grille du niveau en cours d'édition), voir `editorCanvasRegionForTile`, qui
 * simplifie délibérément certains types pour rester lisible.
 * @param type  Type de tuile (`Empty` renvoie une région arbitraire, jamais dessinée en pratique
 *              — les cases vides ne sont pas rendues par l'appelant).
 * @param atlas Atlas fournissant les régions.
 * @return La région d'atlas à échantillonner pour ce type.
 */
[[nodiscard]] core::AtlasRegion regionForTile(core::TileType type, const TextureAtlas& atlas);

/**
 * @brief Région d'atlas pour le **canevas** de l'éditeur (grille du niveau en cours d'édition).
 *
 * Identique à `regionForTile`, sauf pour les tuiles à forme fine ou à taille réduite (pentes,
 * arrondis, `BlockHalf`/`BlockQuarter`) : ce canevas les affiche comme un `Solid` standard (gris,
 * case pleine) plutôt qu'avec leur forme précise (triangle/courbe) ou leur taille réelle
 * (`×0.5`/`×0.25`) — une grille dense mélangeant des formes fines masque plus qu'elle n'aide à
 * lire la disposition d'ensemble d'un niveau pendant l'édition. La forme/taille exacte reste
 * visible en jouant le niveau (`P`, `GameScreen`) et dans la palette (icône dédiée + libellé).
 * @param type  Type de tuile.
 * @param atlas Atlas fournissant les régions.
 * @return La région d'atlas à échantillonner pour ce type, dans le canevas de l'éditeur.
 */
[[nodiscard]] core::AtlasRegion editorCanvasRegionForTile(core::TileType type,
                                                          const TextureAtlas& atlas);

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
