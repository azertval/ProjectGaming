#pragma once

#include <cstdint>
#include <vector>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/SlopeMask.h
 * @brief Silhouette des pentes et arrondis, pour détourer un skin carré (`EX-EDIT-042`).
 */

namespace hmi {

/**
 * @brief Les douze types de tuiles à silhouette **inclinée ou courbe**.
 *
 * Pentes et arrondis, convexes et concaves, au sol comme au plafond (`EX-GP-003`, `EX-GP-004`,
 * `EX-GP-006`, `EX-GP-007`). Seule liste à parcourir : `core::TileType` n'est pas énumérable en
 * C++, et ces douze types sont les seuls dont l'apparence n'est pas un carré plein.
 */
inline constexpr core::TileType SILHOUETTE_TILE_TYPES[] = {
    core::TileType::SlopeUpRight,     core::TileType::SlopeUpLeft,
    core::TileType::RoundedUpRight,   core::TileType::RoundedUpLeft,
    core::TileType::SlopeDownRight,   core::TileType::SlopeDownLeft,
    core::TileType::RoundedDownRight, core::TileType::RoundedDownLeft,
    core::TileType::ConcaveUpRight,   core::TileType::ConcaveUpLeft,
    core::TileType::ConcaveDownRight, core::TileType::ConcaveDownLeft,
};

/// Nombre de types à silhouette.
inline constexpr int SILHOUETTE_TILE_TYPE_COUNT =
    static_cast<int>(sizeof(SILHOUETTE_TILE_TYPES) / sizeof(SILHOUETTE_TILE_TYPES[0]));

/**
 * @brief Indique si un type de tuile a une silhouette autre qu'un carré plein.
 * @param type Type de tuile.
 * @return true pour les douze pentes et arrondis, false pour tous les autres types — qui ne
 *         doivent jamais être détourés.
 */
[[nodiscard]] bool hasSilhouette(core::TileType type) noexcept;

/**
 * @brief Indique si un pixel appartient à la **matière** d'une tuile à silhouette.
 *
 * **Point de vérité unique** de la silhouette affichée, partagé par la génération de l'atlas
 * procédural (`hmi::buildProceduralAtlas`) et par le détourage des skins (`LOT-42`). La géométrie
 * elle-même reste celle de `Core` — `core::slopeSurfaceHeight`, `core::ceilingSlopeHeight`,
 * `core::isCeilingSlope`, les fonctions que suit la **physique**. La réimplémenter ici ferait
 * diverger l'affichage de la hitbox, précisément le défaut que le projet évite depuis le
 * `LOT-22`.
 *
 * La bordure est **franche** (dedans/dehors, jamais d'anticrénelage) : le rendu est en pixel art
 * filtré en plus proche voisin (`EX-ARCH-022`), un bord adouci créerait un halo.
 *
 * L'échantillonnage se fait au **bord** des pixels de coin et non à leur centre : sans cela, la
 * colonne de pixels la plus proche du bord plein d'un arrondi **concave** (tangente quasi
 * verticale à cet endroit) manque la vraie valeur de bord d'environ un quart de case, visible
 * comme une encoche là où la silhouette doit au contraire être quasiment pleine.
 * @param type   Type de tuile ; un type sans silhouette rend toujours true (carré plein).
 * @param localX Colonne du pixel dans la case, 0-based.
 * @param localY Ligne du pixel dans la case, 0-based.
 * @param size   Côté de la case en pixels (doit être supérieur à 1).
 * @return true si le pixel est dans la matière, false s'il est du côté vide.
 */
[[nodiscard]] bool isInsideSilhouette(core::TileType type, int localX, int localY,
                                      int size) noexcept;

/**
 * @brief Applique la silhouette d'un type à une image carrée, en rendant transparent l'extérieur.
 *
 * L'artiste peint un carré plein ; le moteur le découpe à la forme exacte de la hitbox. Sur une
 * case de 16×16 le coût est négligeable, mais le résultat n'en est pas moins mis en cache
 * (`hmi::TextureCache`) plutôt que recalculé à chaque image.
 *
 * Sans effet si @p type n'a pas de silhouette, ou si l'image n'est pas carrée — un skin destiné à
 * un type carré ne doit pas être découpé par accident.
 * @param type   Type de tuile dont on applique la silhouette.
 * @param width  Largeur de l'image, en pixels.
 * @param height Hauteur de l'image, en pixels.
 * @param pixels Pixels `R8G8B8A8_UNORM`, modifiés sur place ; taille attendue `width * height`.
 */
void applySilhouetteMask(core::TileType type, int width, int height,
                         std::vector<std::uint32_t>& pixels);

}  // namespace hmi
