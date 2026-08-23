// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>

#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

/**
 * @file Core/Physics/SlopeGeometry.h
 * @brief Hauteur de surface des tuiles à profil incliné ou courbe (`EX-GP-003`, `EX-GP-004`,
 *        `EX-GP-007`).
 */

namespace core {

class TileMap;

/**
 * @brief Hauteur de la surface d'une tuile **suivable** (pente, arrondi) à une position
 *        horizontale donnée.
 *
 * Repère **local** à la case : `localX` dans `[0, 1[` (0 = bord gauche, proche de 1 = bord droit),
 * hauteur renvoyée dans `[0, 1]` mesurée depuis le **haut** de la case (0 = haut, 1 = bas) — pour
 * obtenir la position monde de la surface, ajouter le résultat à la ligne (`row`) de la case
 * (`EX-ARCH-020`, origine haut-gauche, une case = une unité monde).
 *
 * Fonction **pure**, sans dépendance rendu ni physique : `core::CharacterPhysicsSystem` (via la
 * passe de suivi de surface, @ref guide-physique) est seul à décider comment l'utiliser
 * (tolérance de calage, cas du saut). Aucune tuile n'est retournée comme solide par cette
 * fonction — voir `core::isSolid`, qui exclut délibérément les tuiles suivables.
 *
 * Couvre aussi la **face du haut** des pentes/arrondis de **plafond** (`isCeilingSlope`,
 * `EX-GP-006`) : toujours `0.0f` (constante, quel que soit `localX`) — leur matière pleine
 * commence **toujours** au sommet de la case, seule sa profondeur vers le bas varie (silhouette
 * inclinée/courbe gérée séparément par `ceilingSlopeHeight`/`resolveCeilingSlopeFollow`, pour un
 * saut qui la franchit par en dessous). Sans ce cas, un personnage tombant sur le dessus d'un
 * plafond incliné (accessible par au-dessus) tomberait au travers plutôt que de s'y poser.
 *
 * @param type   Type de tuile.
 * @param localX Position horizontale dans la case, `[0, 1[`.
 * @return La hauteur de surface, ou `std::nullopt` si @p type n'a pas de surface à suivre.
 */
[[nodiscard]] std::optional<float> slopeSurfaceHeight(TileType type, float localX) noexcept;

/// @return true si @p type a une surface à suivre (pente ou arrondi, convexe ou **concave**, de
///         **sol**) — le personnage s'y cale en marchant, plutôt que d'être simplement bloqué ou de
///         tomber au travers. `false` pour les variantes de **plafond** (`isCeilingSlope`) : on ne
///         « marche » jamais sous un plafond (pas de déplacement latéral calé dessus), seul le fait
///         de ne pas le traverser en sautant compte (`resolveCeilingSlopeFollow`).
[[nodiscard]] constexpr bool isFollowableSurface(TileType type) noexcept {
    return type == TileType::SlopeUpRight || type == TileType::SlopeUpLeft ||
           type == TileType::RoundedUpRight || type == TileType::RoundedUpLeft ||
           type == TileType::ConcaveUpRight || type == TileType::ConcaveUpLeft;
}

/// @return true si @p type est une pente/arrondi de **plafond** (`EX-GP-006`/`EX-GP-007`) — miroir
///         vertical d'une surface de sol suivable, mais dont le personnage ne peut jamais franchir
///         la silhouette en sautant (`resolveCeilingSlopeFollow`), sans pour autant y « marcher »
///         (`isFollowableSurface` reste `false` pour ces types).
[[nodiscard]] constexpr bool isCeilingSlope(TileType type) noexcept {
    return type == TileType::SlopeDownRight || type == TileType::SlopeDownLeft ||
           type == TileType::RoundedDownRight || type == TileType::RoundedDownLeft ||
           type == TileType::ConcaveDownRight || type == TileType::ConcaveDownLeft;
}

/**
 * @brief Hauteur de la silhouette d'une pente/arrondi de **plafond** (`EX-GP-006`), au même repère
 *        que `slopeSurfaceHeight` (mesurée depuis le haut de la case, `[0, 1]`), mais avec la
 *        sémantique **inversée** : la matière pleine remplit `[0, hauteur]` (le **haut** de la
 *        case), pas `[hauteur, 1]` (le bas) comme pour une surface de sol.
 *
 * Calculée comme le **miroir vertical exact** de la surface de sol de même orientation (`1 -
 * slopeSurfaceHeight(type miroir, localX)`), pas une famille de formules dupliquée — un plafond
 * `SlopeDownRight` a exactement le profil horizontal de `SlopeUpRight`, silhouette retournée.
 *
 * @param type   Type de tuile.
 * @param localX Position horizontale dans la case, `[0, 1[`.
 * @return La hauteur de la silhouette, ou `std::nullopt` si @p type n'est pas une pente/arrondi de
 *         plafond (`isCeilingSlope`).
 */
[[nodiscard]] std::optional<float> ceilingSlopeHeight(TileType type, float localX) noexcept;

/// @brief Résultat de `resolveSlopeFollow` : la position verticale sur laquelle se caler, si une
///        surface suivable a été franchie pendant le pas.
struct SlopeFollowResult {
    /// true si une surface suivable a été trouvée et doit s'appliquer ce pas.
    bool grounded = false;
    /// Position Y du bord bas de la boîte, calée sur la surface (valide seulement si `grounded`).
    float bottomY = 0.0f;
};

/**
 * @brief Détecte si le bord bas d'une boîte a **franchi** une surface suivable (pente, arrondi)
 *        pendant un pas, et calcule la position à laquelle s'y caler.
 *
 * Compare le bord bas **avant** le pas (@p previousBox, avant le balayage classique sur grille) et
 * **après** (`newBox.max.y`, une fois la grille résolue) : si ce bord a franchi la hauteur de
 * surface d'une tuile suivable quelque part entre les deux (parcours de toutes les lignes
 * concernées, pas seulement la position finale — pour ne jamais « sauter » une pente à grande
 * vitesse de chute, même si le balayage classique ne la voit pas comme solide), renvoie la position
 * de calage. Ignore toute surface si @p velocityY est **négative** (le personnage monte — vient de
 * sauter) : le suivi de pente ne doit jamais annuler un saut volontaire.
 *
 * Les colonnes testées sont celles réellement couvertes par la **largeur** de la boîte — à la fois
 * @p previousBox (position avant le pas) et @p newBox (après), pas seulement son centre final : un
 * déplacement horizontal (marche) pendant le même pas qu'une ascension/chute peut faire sortir la
 * boîte d'une colonne pertinente avant que le seuil vertical ne soit atteint ; ignorer @p
 * previousBox manquerait alors la case, qui reste pourtant celle qui aurait dû bloquer/porter le
 * personnage.
 *
 * @param previousBox Boîte avant le pas (avant le balayage sur grille) — sert à la fois pour son
 *                     bord bas et pour l'étendue horizontale qu'elle couvrait.
 * @param newBox      Boîte après résolution du balayage classique sur grille (murs/sols).
 * @param velocityY   Vitesse verticale courante (`> 0` = chute, `< 0` = monte).
 * @param tiles       Grille de niveau (pour lire le type de tuile aux positions testées).
 * @return Le résultat du calage, ou `grounded == false` si aucune surface n'a été franchie.
 */
[[nodiscard]] SlopeFollowResult resolveSlopeFollow(const Aabb& previousBox, const Aabb& newBox,
                                                   float velocityY, const TileMap& tiles) noexcept;

/// @brief Résultat de `resolveCeilingSlopeFollow` : la position verticale à laquelle bloquer le
///        bord haut, si une silhouette de plafond a été franchie pendant le pas.
struct CeilingSlopeFollowResult {
    /// true si une silhouette de plafond a été franchie et doit bloquer l'ascension ce pas.
    bool blocked = false;
    /// Position Y du bord haut de la boîte, calée sur la silhouette (valide si `blocked`).
    float topY = 0.0f;
};

/**
 * @brief Détecte si le bord haut d'une boîte a **franchi** une silhouette de pente/arrondi de
 *        plafond (`EX-GP-006`) pendant un pas, et calcule la position à laquelle s'y bloquer.
 *
 * **Miroir exact** de `resolveSlopeFollow`, pour le bord **haut** plutôt que bas, déclenché en
 * **montant** (`velocityY < 0`, saut) plutôt qu'en tombant : compare le bord haut **avant** le pas
 * (@p previousBox) et **après** (`newBox.min.y`) ; si ce bord a franchi la hauteur de silhouette
 * d'une tuile de plafond quelque part entre les deux (parcours de toutes les lignes concernées,
 * pas seulement la position finale — un saut rapide ne doit jamais « traverser » un plafond
 * incliné/courbe), renvoie la position de blocage. Ignore toute silhouette si @p velocityY est
 * **positive ou nulle** (le personnage tombe ou est immobile) : contrairement à une surface de
 * sol, une silhouette de plafond ne fait **jamais** marcher le personnage — elle ne fait que
 * bloquer une ascension, jamais autre chose (`core::isFollowableSurface` reste `false` pour ces
 * types).
 *
 * Les colonnes testées sont celles réellement couvertes par la largeur de @p newBox **et** par
 * l'intervalle [@p sweptMinX, @p sweptMaxX] (même principe que `resolveSlopeFollow`, mais avec un
 * intervalle fourni par l'appelant plutôt qu'une seule boîte précédente) : sauter tout en marchant
 * peut faire sortir la boîte d'une colonne pertinente avant que le seuil vertical de blocage n'y
 * soit atteint — parfois sur PLUSIEURS pas (une montée lente combinée à une marche rapide), pas
 * seulement le pas immédiatement précédent. L'appelant (`CharacterPhysicsSystem`) est donc
 * responsable d'accumuler cet intervalle sur toute la montée courante (depuis le dernier contact au
 * sol ou le dernier pas où `velocityY >= 0`), pas seulement de fournir la boîte du pas précédent.
 *
 * @param previousTopY Bord haut de la boîte avant le pas (avant le balayage sur grille).
 * @param sweptMinX    Bord gauche le plus à gauche couvert par la boîte depuis le début de la
 *                      montée courante (avant ce pas) — unioné avec @p newBox en interne.
 * @param sweptMaxX    Bord droit le plus à droite couvert, symétrique de @p sweptMinX.
 * @param newBox       Boîte après résolution du balayage classique sur grille (murs/sols/plafonds
 *                      plats).
 * @param velocityY    Vitesse verticale courante (`> 0` = chute, `< 0` = monte).
 * @param tiles        Grille de niveau (pour lire le type de tuile aux positions testées).
 * @return Le résultat du blocage, ou `blocked == false` si aucune silhouette n'a été franchie.
 */
[[nodiscard]] CeilingSlopeFollowResult resolveCeilingSlopeFollow(float previousTopY,
                                                                 float sweptMinX, float sweptMaxX,
                                                                 const Aabb& newBox,
                                                                 float velocityY,
                                                                 const TileMap& tiles) noexcept;

}  // namespace core
