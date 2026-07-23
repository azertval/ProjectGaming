#pragma once

#include <optional>

#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

/**
 * @file Core/Physics/SlopeGeometry.h
 * @brief Hauteur de surface des tuiles à profil incliné ou courbe (`EX-GP-003`, `EX-GP-004`).
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
 * @param type   Type de tuile.
 * @param localX Position horizontale dans la case, `[0, 1[`.
 * @return La hauteur de surface, ou `std::nullopt` si @p type n'a pas de surface à suivre.
 */
[[nodiscard]] std::optional<float> slopeSurfaceHeight(TileType type, float localX) noexcept;

/// @return true si @p type a une surface à suivre (pente ou arrondi) — le personnage s'y cale
///         plutôt que d'être simplement bloqué ou de tomber au travers.
[[nodiscard]] constexpr bool isFollowableSurface(TileType type) noexcept {
    return type == TileType::SlopeUpRight || type == TileType::SlopeUpLeft ||
           type == TileType::RoundedUpRight || type == TileType::RoundedUpLeft;
}

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
 * Compare le bord bas **avant** le pas (@p previousBottomY, avant le balayage classique sur
 * grille) et **après** (`newBox.max.y`, une fois la grille résolue) : si ce bord a franchi la
 * hauteur de surface d'une tuile suivable quelque part entre les deux (parcours de toutes les
 * lignes concernées, pas seulement la position finale — pour ne jamais « sauter » une pente à
 * grande vitesse de chute, même si le balayage classique ne la voit pas comme solide), renvoie la
 * position de calage. Ignore toute surface si @p velocityY est **négative** (le personnage monte
 * — vient de sauter) : le suivi de pente ne doit jamais annuler un saut volontaire.
 *
 * La colonne testée est celle du **centre horizontal** de @p newBox (limite connue : un
 * déplacement horizontal très rapide au sein d'un même pas pourrait changer de colonne sans que
 * cette fonction ne le détecte — acceptable pour une pente traversée en marchant, à revoir si des
 * cas d'usage à grande vitesse horizontale combinée à une pente apparaissent).
 *
 * @param previousBottomY Bord bas de la boîte avant le pas (avant le balayage sur grille).
 * @param newBox          Boîte après résolution du balayage classique sur grille (murs/sols).
 * @param velocityY       Vitesse verticale courante (`> 0` = chute, `< 0` = monte).
 * @param tiles           Grille de niveau (pour lire le type de tuile aux positions testées).
 * @return Le résultat du calage, ou `grounded == false` si aucune surface n'a été franchie.
 */
[[nodiscard]] SlopeFollowResult resolveSlopeFollow(float previousBottomY, const Aabb& newBox,
                                                    float velocityY,
                                                    const TileMap& tiles) noexcept;

}  // namespace core
