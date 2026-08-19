/**
 * @file test_pixel_frame_geometry.cpp
 * @brief Tests unitaires de la géométrie du cadre pixel art (`LOT-68`, `EX-IHM-070`).
 */

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Interface/PixelFrameGeometry.h"

namespace {

[[nodiscard]] bool containsRole(const std::vector<hmi::PixelFrameQuad>& quads,
                                hmi::PixelFrameRole role) {
    return std::any_of(quads.begin(), quads.end(),
                       [role](const hmi::PixelFrameQuad& quad) { return quad.role == role; });
}

}  // namespace

/**
 * @brief Aucun pavé ne déborde du cadre, à aucune taille ni aucune échelle — y compris quand le
 *        cadre est plus petit que ses propres bordures. Un débordement peindrait par-dessus le
 *        widget voisin, défaut invisible en revue de code et criant à l'écran. Même garde que
 *        `GeometrieDesIconesNonVideEtDansLeCadre` pour les icônes.
 * \castest{<b>Aucun pave du cadre ne sort du rectangle du widget.</b><br/>
 * \tcat Unitaire · Cadre pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire la geometrie pour un balayage de tailles et d'echelles.<br/>2. Verifier
 * que chaque pave a des dimensions positives et reste dans [0,largeur] x [0,hauteur].<br/>
 * \tattendu Aucun pave ne deborde ni n'est vide.
 * }
 */
TEST(PixelFrameGeometryTest, AucunPaveNeDeborde) {
    for (const int scale : {1, 2, 3}) {
        for (int size = 1; size <= 200; ++size) {
            const std::vector<hmi::PixelFrameQuad> quads = hmi::pixelFrameQuads(size, size, scale);
            for (const hmi::PixelFrameQuad& quad : quads) {
                EXPECT_GT(quad.width, 0) << "taille " << size << " echelle " << scale;
                EXPECT_GT(quad.height, 0) << "taille " << size << " echelle " << scale;
                EXPECT_GE(quad.x, 0);
                EXPECT_GE(quad.y, 0);
                EXPECT_LE(quad.x + quad.width, size) << "taille " << size << " echelle " << scale;
                EXPECT_LE(quad.y + quad.height, size) << "taille " << size << " echelle " << scale;
            }
        }
    }
}

/**
 * @brief Un cadre de taille utile porte les quatre rôles. Un cadre sans biseau serait une bordure
 *        carrée ordinaire : c'est le relief qui le fait lire comme du pixel art.
 * \castest{<b>Un cadre de taille utile porte les quatre roles de couleur.</b><br/>
 * \tcat Unitaire · Cadre pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la geometrie d'un cadre de 120 x 80 a l'echelle 2.<br/>2. Chercher chacun
 * des quatre roles.<br/>
 * \tattendu Les quatre roles sont presents.
 * }
 */
TEST(PixelFrameGeometryTest, LesQuatreRolesSontPresents) {
    const std::vector<hmi::PixelFrameQuad> quads = hmi::pixelFrameQuads(120, 80, 2);
    EXPECT_TRUE(containsRole(quads, hmi::PixelFrameRole::Fill));
    EXPECT_TRUE(containsRole(quads, hmi::PixelFrameRole::Outline));
    EXPECT_TRUE(containsRole(quads, hmi::PixelFrameRole::BevelLight));
    EXPECT_TRUE(containsRole(quads, hmi::PixelFrameRole::BevelDark));
}

/**
 * @brief Les quatre coins restent **vides** : aucun pavé de contour ne les couvre. C'est l'entaille
 *        qui distingue le cadre pixel art d'une bordure carrée, et elle disparaîtrait sans bruit si
 *        le contour devenait un rectangle plein.
 * \castest{<b>Les quatre coins du cadre ne sont couverts par aucun pave.</b><br/>
 * \tcat Unitaire · Cadre pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la geometrie d'un cadre de 120 x 80 a l'echelle 2.<br/>2. Verifier que le
 * pixel de chaque coin n'appartient a aucun pave.<br/>
 * \tattendu Les quatre coins sont vides.
 * }
 */
TEST(PixelFrameGeometryTest, LesCoinsSontEntailles) {
    constexpr int WIDTH = 120;
    constexpr int HEIGHT = 80;
    const std::vector<hmi::PixelFrameQuad> quads = hmi::pixelFrameQuads(WIDTH, HEIGHT, 2);

    const auto covers = [&quads](int x, int y) {
        return std::any_of(quads.begin(), quads.end(), [x, y](const hmi::PixelFrameQuad& quad) {
            return x >= quad.x && x < quad.x + quad.width && y >= quad.y &&
                   y < quad.y + quad.height;
        });
    };
    EXPECT_FALSE(covers(0, 0));
    EXPECT_FALSE(covers(WIDTH - 1, 0));
    EXPECT_FALSE(covers(0, HEIGHT - 1));
    EXPECT_FALSE(covers(WIDTH - 1, HEIGHT - 1));
}

/**
 * @brief Le cadre est symétrique : la bordure gauche et la bordure droite ont la même épaisseur,
 *        de même que la haute et la basse. Une asymétrie d'un pixel est invisible en relecture et
 *        saute aux yeux une fois quatre cadres alignés à l'écran.
 * \castest{<b>Les bordures opposees du cadre ont la meme epaisseur.</b><br/>
 * \tcat Unitaire · Cadre pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la geometrie d'un cadre rectangulaire.<br/>2. Mesurer l'epaisseur des
 * quatre barres de contour.<br/>
 * \tattendu Gauche egale droite, haut egale bas.
 * }
 */
TEST(PixelFrameGeometryTest, LesBorduresOpposeesSontSymetriques) {
    constexpr int WIDTH = 160;
    constexpr int HEIGHT = 96;
    const std::vector<hmi::PixelFrameQuad> quads = hmi::pixelFrameQuads(WIDTH, HEIGHT, 3);

    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    for (const hmi::PixelFrameQuad& quad : quads) {
        if (quad.role != hmi::PixelFrameRole::Outline) {
            continue;
        }
        if (quad.x == 0) {
            left = quad.width;
        } else if (quad.x + quad.width == WIDTH) {
            right = quad.width;
        }
        if (quad.y == 0) {
            top = quad.height;
        } else if (quad.y + quad.height == HEIGHT) {
            bottom = quad.height;
        }
    }
    EXPECT_GT(left, 0);
    EXPECT_EQ(left, right);
    EXPECT_GT(top, 0);
    EXPECT_EQ(top, bottom);
}

/**
 * @brief Une taille nulle ou négative ne produit rien, et une taille trop petite pour porter des
 *        bordures produit un simple aplat plutôt qu'une géométrie dégénérée.
 * \castest{<b>Les tailles degenerees produisent le vide ou un aplat, jamais une geometrie
 * invalide.</b><br/>
 * \tcat Unitaire · Cadre pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander la geometrie pour une largeur ou une hauteur nulle, puis negative.<br/>
 * 2. Demander celle d'un cadre de 2 x 2 a l'echelle 3.<br/>
 * \tattendu Les cas nuls et negatifs sont vides ; le cadre minuscule porte un unique pave de
 * remplissage couvrant tout.
 * }
 */
TEST(PixelFrameGeometryTest, TaillesDegenereesSansGeometrieInvalide) {
    EXPECT_TRUE(hmi::pixelFrameQuads(0, 40, 2).empty());
    EXPECT_TRUE(hmi::pixelFrameQuads(40, 0, 2).empty());
    EXPECT_TRUE(hmi::pixelFrameQuads(-10, -10, 2).empty());

    const std::vector<hmi::PixelFrameQuad> tiny = hmi::pixelFrameQuads(2, 2, 3);
    ASSERT_EQ(tiny.size(), 1u);
    EXPECT_EQ(tiny.front().role, hmi::PixelFrameRole::Fill);
    EXPECT_EQ(tiny.front().width, 2);
    EXPECT_EQ(tiny.front().height, 2);
}
