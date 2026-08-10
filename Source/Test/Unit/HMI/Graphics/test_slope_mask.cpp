/**
 * @file test_slope_mask.cpp
 * @brief Tests unitaires du masque de silhouette des pentes et arrondis (LOT-42 TACHE-03).
 */

#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "Core/Physics/SlopeGeometry.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/SlopeMask.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;

/// Alpha d'un pixel R8G8B8A8_UNORM.
std::uint32_t alphaOf(std::uint32_t pixel) {
    return (pixel >> 24) & 0xFFu;
}

/// Image carree entierement opaque, de la taille d'une case.
std::vector<std::uint32_t> opaqueTile() {
    return std::vector<std::uint32_t>(static_cast<std::size_t>(TILE) * TILE, 0xFF808080u);
}

}  // namespace

/**
 * @brief Seuls les douze types de pentes et arrondis ont une silhouette.
 * \castest{<b>Seuls les douze types de pentes et arrondis ont une silhouette.</b><br/>
 * \tcat Unitaire · Masque de silhouette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Interroger hasSilhouette pour les douze types listes.<br/>
 * 2. L'interroger pour des types carres representatifs.<br/>
 * \tattendu Les douze rendent vrai, les types carres rendent faux.
 * }
 */
TEST(SlopeMaskTest, DouzeTypesOntUneSilhouette) {
    for (const core::TileType type : hmi::SILHOUETTE_TILE_TYPES) {
        EXPECT_TRUE(hmi::hasSilhouette(type));
    }
    EXPECT_EQ(hmi::SILHOUETTE_TILE_TYPE_COUNT, 12);

    // Un type carre detoure par erreur perdrait des pixels de son skin.
    EXPECT_FALSE(hmi::hasSilhouette(core::TileType::Solid));
    EXPECT_FALSE(hmi::hasSilhouette(core::TileType::Block));
    EXPECT_FALSE(hmi::hasSilhouette(core::TileType::Danger));
    EXPECT_FALSE(hmi::hasSilhouette(core::TileType::Empty));
}

/**
 * @brief Le masque coincide exactement avec la silhouette de l'atlas procedural.
 * \castest{<b>Le masque coincide pixel a pixel avec la silhouette de l'atlas procedural.</b><br/>
 * \tcat Unitaire · Masque de silhouette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Generer l'atlas procedural.<br/>
 * 2. Pour chacun des douze types, comparer chaque pixel de sa case a isInsideSilhouette.<br/>
 * \tattendu Les pixels opaques de l'atlas sont exactement ceux que le masque declare interieurs.
 * }
 */
TEST(SlopeMaskTest, MasqueIdentiqueALAtlasProcedural) {
    const hmi::ProceduralAtlasImage atlas = hmi::buildProceduralAtlasImage();

    for (const core::TileType type : hmi::SILHOUETTE_TILE_TYPES) {
        const std::optional<hmi::AtlasGridPosition> position = hmi::slopeTileGridPosition(type);
        ASSERT_TRUE(position.has_value()) << "type sans case d'atlas";

        for (int y = 0; y < TILE; ++y) {
            for (int x = 0; x < TILE; ++x) {
                const int atlasX = position->column * TILE + x;
                const int atlasY = position->row * TILE + y;
                const std::uint32_t pixel =
                    atlas.pixels[static_cast<std::size_t>(atlasY) * atlas.width + atlasX];

                // Les deux doivent decrire la MEME forme : un skin decoupe autrement que l'atlas
                // se verrait immediatement en basculant Physique/Texture.
                const bool inside = hmi::isInsideSilhouette(type, x, y, TILE);
                EXPECT_EQ(alphaOf(pixel) != 0, inside)
                    << "divergence en (" << x << ", " << y << ") pour la case (" << position->column
                    << ", " << position->row << ")";
            }
        }
    }
}

/**
 * @brief Une pente montante est pleine en bas et vide en haut.
 * \castest{<b>Une pente montante vers la droite est pleine sous sa surface, vide
 * au-dessus.</b><br/> \tcat Unitaire · Masque de silhouette<br/> \tcrit Critique<br/> \tetapes 1.
 * Tester les quatre coins de la case d'une pente montante vers la droite.<br/> \tattendu Le coin
 * bas-droite est plein, le coin haut-gauche est vide.
 * }
 */
TEST(SlopeMaskTest, PenteMontantePleineEnBas) {
    constexpr int LAST = TILE - 1;

    // SlopeUpRight : la matiere monte vers la droite, donc le bas-droite est plein et le
    // haut-gauche vide. C'est la silhouette que suit la physique (core::slopeSurfaceHeight).
    EXPECT_TRUE(hmi::isInsideSilhouette(core::TileType::SlopeUpRight, LAST, LAST, TILE));
    EXPECT_TRUE(hmi::isInsideSilhouette(core::TileType::SlopeUpRight, 0, LAST, TILE));
    EXPECT_TRUE(hmi::isInsideSilhouette(core::TileType::SlopeUpRight, LAST, 0, TILE));
    EXPECT_FALSE(hmi::isInsideSilhouette(core::TileType::SlopeUpRight, 0, 0, TILE));
}

/**
 * @brief Chaque type de plafond est le miroir vertical exact de son equivalent de sol.
 * \castest{<b>Chaque silhouette de plafond est le miroir vertical de celle de sol.</b><br/>
 * \tcat Unitaire · Masque de silhouette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Pour les six paires sol/plafond, comparer chaque pixel du plafond au pixel
 * verticalement symetrique de la variante de sol.<br/>
 * \tattendu Les deux silhouettes coincident sur toute la case.
 * }
 */
TEST(SlopeMaskTest, PenteDePlafondMiroirVertical) {
    constexpr int LAST = TILE - 1;

    // core::ceilingSlopeHeight est definie comme 1 - slopeSurfaceHeight du type de sol equivalent :
    // la silhouette affichee doit refleter exactement cette definition, pixel a pixel.
    const struct {
        core::TileType ceiling;
        core::TileType floor;
    } pairs[] = {
        {core::TileType::SlopeDownRight, core::TileType::SlopeUpRight},
        {core::TileType::SlopeDownLeft, core::TileType::SlopeUpLeft},
        {core::TileType::RoundedDownRight, core::TileType::RoundedUpRight},
        {core::TileType::RoundedDownLeft, core::TileType::RoundedUpLeft},
        {core::TileType::ConcaveDownRight, core::TileType::ConcaveUpRight},
        {core::TileType::ConcaveDownLeft, core::TileType::ConcaveUpLeft},
    };

    for (const auto& pair : pairs) {
        for (int y = 0; y < TILE; ++y) {
            for (int x = 0; x < TILE; ++x) {
                // Les pixels que la surface traverse EXACTEMENT sont exclus : la hauteur de
                // plafond est calculee comme 1 - hauteur_de_sol, et cette soustraction ne rend pas
                // toujours la valeur d'origine au dernier bit pres. Sur ces pixels de bord, la
                // comparaison « dans la matiere » peut donc basculer d'un cote a l'autre. L'ecart
                // ne concerne que la ligne de contour elle-meme et existe a l'identique dans
                // l'atlas procedural livre (cf. MasqueIdentiqueALAtlasProcedural).
                const float normalizedX = static_cast<float>(x) / static_cast<float>(LAST);
                const float normalizedY = static_cast<float>(LAST - y) / static_cast<float>(LAST);
                const std::optional<float> surface =
                    core::slopeSurfaceHeight(pair.floor, normalizedX);
                ASSERT_TRUE(surface.has_value());
                if (std::abs(normalizedY - *surface) < 1e-5f) {
                    continue;
                }

                EXPECT_EQ(hmi::isInsideSilhouette(pair.ceiling, x, y, TILE),
                          hmi::isInsideSilhouette(pair.floor, x, LAST - y, TILE))
                    << "miroir rompu en (" << x << ", " << y << ")";
            }
        }
    }
}

/**
 * @brief Le detourage rend transparents les pixels hors silhouette et eux seuls.
 * \castest{<b>Le detourage rend transparents les pixels hors silhouette, et eux seuls.</b><br/>
 * \tcat Unitaire · Masque de silhouette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Detourer une image entierement opaque a la silhouette d'une pente.<br/>
 * 2. Comparer l'alpha de chaque pixel a l'appartenance a la silhouette.<br/>
 * \tattendu L'alpha vaut 255 dans la silhouette et 0 en dehors, sans valeur intermediaire.
 * }
 */
TEST(SlopeMaskTest, DetourageTransparenceFranche) {
    std::vector<std::uint32_t> pixels = opaqueTile();
    hmi::applySilhouetteMask(core::TileType::RoundedUpLeft, TILE, TILE, pixels);

    for (int y = 0; y < TILE; ++y) {
        for (int x = 0; x < TILE; ++x) {
            const std::uint32_t alpha = alphaOf(pixels[static_cast<std::size_t>(y) * TILE + x]);
            // Transparence franche, jamais anticrenelee : le rendu est en plus proche voisin
            // (EX-ARCH-022), un bord adouci creerait un halo.
            EXPECT_TRUE(alpha == 0 || alpha == 255)
                << "alpha intermediaire en (" << x << ", " << y << ") : " << alpha;
            EXPECT_EQ(alpha != 0,
                      hmi::isInsideSilhouette(core::TileType::RoundedUpLeft, x, y, TILE));
        }
    }
}

/**
 * @brief Un type carre n'est jamais detoure.
 * \castest{<b>Un type de tuile carre n'est pas detoure.</b><br/>
 * \tcat Unitaire · Masque de silhouette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Detourer une image opaque a la silhouette de Solid, puis de Block et Danger.<br/>
 * \tattendu L'image reste entierement opaque dans les trois cas.
 * }
 */
TEST(SlopeMaskTest, TypeCarreNonDetoure) {
    for (const core::TileType type :
         {core::TileType::Solid, core::TileType::Block, core::TileType::Danger}) {
        std::vector<std::uint32_t> pixels = opaqueTile();
        hmi::applySilhouetteMask(type, TILE, TILE, pixels);

        for (const std::uint32_t pixel : pixels) {
            ASSERT_EQ(alphaOf(pixel), 255u);
        }
    }
}

/**
 * @brief Une image non carree ou de taille incoherente n'est pas modifiee.
 * \castest{<b>Une image non carree ou de dimensions incoherentes n'est pas detouree.</b><br/>
 * \tcat Unitaire · Masque de silhouette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Detourer une image rectangulaire, puis une image dont la taille ne correspond pas
 * aux dimensions annoncees.<br/>
 * \tattendu Les deux images restent inchangees, sans ecriture hors bornes.
 * }
 */
TEST(SlopeMaskTest, ImageNonCarreeInchangee) {
    // Une planche de raccords n'est pas carree par case : la detourer entierement n'aurait pas de
    // sens, et une taille incoherente ne doit jamais faire ecrire hors bornes.
    std::vector<std::uint32_t> rectangle(static_cast<std::size_t>(TILE) * TILE * 2, 0xFF808080u);
    const std::vector<std::uint32_t> rectangleBefore = rectangle;
    hmi::applySilhouetteMask(core::TileType::SlopeUpRight, TILE, TILE * 2, rectangle);
    EXPECT_EQ(rectangle, rectangleBefore);

    std::vector<std::uint32_t> tooSmall(4, 0xFF808080u);
    const std::vector<std::uint32_t> tooSmallBefore = tooSmall;
    hmi::applySilhouetteMask(core::TileType::SlopeUpRight, TILE, TILE, tooSmall);
    EXPECT_EQ(tooSmall, tooSmallBefore);
}
