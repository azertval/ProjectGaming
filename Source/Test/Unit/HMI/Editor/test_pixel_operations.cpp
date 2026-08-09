/**
 * @file test_pixel_operations.cpp
 * @brief Tests unitaires des opérations pures sur tampon de pixels (LOT-54 TACHE-02,
 *        `EX-EDIT-045`).
 */

#include <cstdint>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Editor/PixelOperations.h"

namespace {

constexpr std::uint32_t RED = 0xFF0000FFu;
constexpr std::uint32_t GREEN = 0xFF00FF00u;
constexpr std::uint32_t BLUE = 0xFFFF0000u;
constexpr std::uint32_t TRANSPARENT_BLACK = 0x00000000u;

// Image width x height uniformement de couleur color.
hmi::DecodedImage uniformImage(int width, int height, std::uint32_t color) {
    hmi::DecodedImage image;
    image.width = width;
    image.height = height;
    image.pixels.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), color);
    return image;
}

}  // namespace

/**
 * @brief Poser un pixel dans les bornes change sa couleur et renvoie ce seul pixel comme région
 *        modifiée ; hors bornes, l'image n'est pas touchée et la région renvoyée est vide.
 * \castest{<b>setPixel change la couleur du pixel vise et renvoie la region attendue.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Poser un pixel dans les bornes d'une image.<br/>2. Poser un pixel hors bornes.<br/>
 * \tattendu Le premier change la couleur et renvoie la region (x,y)-(x,y) ; le second ne change
 * rien et renvoie une region vide.
 * }
 */
TEST(PixelOperationsTest, SetPixelChangeLaCouleurEtRenvoieLaRegion) {
    hmi::DecodedImage image = uniformImage(4, 4, RED);

    const hmi::PixelRegion region = hmi::setPixel(image, 1, 2, GREEN);
    EXPECT_FALSE(region.empty());
    EXPECT_EQ(region.minX, 1);
    EXPECT_EQ(region.maxX, 1);
    EXPECT_EQ(region.minY, 2);
    EXPECT_EQ(region.maxY, 2);
    EXPECT_EQ(hmi::pickColor(image, 1, 2), GREEN);

    const hmi::PixelRegion outOfBounds = hmi::setPixel(image, 10, 10, GREEN);
    EXPECT_TRUE(outOfBounds.empty());
}

/**
 * @brief Effacer un pixel met son alpha à zéro sans changer sa teinte RVB.
 * \castest{<b>erasePixel met l'alpha a zero sans changer la teinte RVB.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Effacer un pixel colore.<br/>2. Lire sa couleur.<br/>
 * \tattendu Le canal alpha est nul, les canaux RVB sont inchanges.
 * }
 */
TEST(PixelOperationsTest, ErasePixelPreserveLaTeinteRvb) {
    hmi::DecodedImage image = uniformImage(2, 2, RED);

    hmi::erasePixel(image, 0, 0);
    const std::optional<std::uint32_t> erased = hmi::pickColor(image, 0, 0);
    ASSERT_TRUE(erased.has_value());
    EXPECT_EQ(*erased & 0xFF000000u, 0u) << "alpha doit etre nul";
    EXPECT_EQ(*erased & 0x00FFFFFFu, RED & 0x00FFFFFFu) << "la teinte RVB doit etre conservee";
}

/**
 * @brief La pipette renvoie la couleur du pixel visé, ou `nullopt` hors bornes.
 * \castest{<b>pickColor renvoie la couleur du pixel ou nullopt hors bornes.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser une couleur connue sur un pixel.<br/>2. La prelever avec pickColor.<br/>
 * 3. Prelever hors bornes.<br/>
 * \tattendu La couleur prelevee correspond ; hors bornes, le resultat est absent.
 * }
 */
TEST(PixelOperationsTest, PickColorRenvoieLaCouleurOuAbsentHorsBornes) {
    hmi::DecodedImage image = uniformImage(3, 3, BLUE);

    EXPECT_EQ(hmi::pickColor(image, 1, 1), BLUE);
    EXPECT_FALSE(hmi::pickColor(image, -1, 0).has_value());
    EXPECT_FALSE(hmi::pickColor(image, 3, 0).has_value());
}

/**
 * @brief Un glisser rapide (diagonale) ne laisse aucun trou : tous les pixels du segment sont
 *        peints.
 * \castest{<b>drawLine peint un segment diagonal sans trou.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tracer une ligne diagonale entre deux positions eloignees.<br/>2. Lire chaque pixel
 * du chemin attendu.<br/>
 * \tattendu Tous les pixels du chemin diagonal sont de la couleur tracee.
 * }
 */
TEST(PixelOperationsTest, DrawLineDiagonaleSansTrou) {
    hmi::DecodedImage image = uniformImage(6, 6, TRANSPARENT_BLACK);

    const hmi::PixelRegion region = hmi::drawLine(image, 0, 0, 5, 5, GREEN);
    EXPECT_EQ(region.minX, 0);
    EXPECT_EQ(region.minY, 0);
    EXPECT_EQ(region.maxX, 5);
    EXPECT_EQ(region.maxY, 5);
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(hmi::pickColor(image, i, i), GREEN) << "pixel diagonal " << i;
    }
}

/**
 * @brief Un tracé purement vertical peint chaque ligne du segment, sans trou.
 * \castest{<b>drawLine peint un segment purement vertical sans trou.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Tracer une ligne verticale.<br/>2. Lire chaque pixel de la colonne.<br/>
 * \tattendu Tous les pixels de la colonne sont de la couleur tracee.
 * }
 */
TEST(PixelOperationsTest, DrawLineVerticaleSansTrou) {
    hmi::DecodedImage image = uniformImage(3, 6, TRANSPARENT_BLACK);

    hmi::drawLine(image, 1, 0, 1, 5, RED);
    for (int y = 0; y < 6; ++y) {
        EXPECT_EQ(hmi::pickColor(image, 1, y), RED) << "ligne " << y;
    }
    EXPECT_EQ(hmi::pickColor(image, 0, 3), TRANSPARENT_BLACK) << "colonne voisine non touchee";
}

/**
 * @brief Le remplissage d'une zone fermée colore exactement cette zone, sans déborder au-delà de
 *        sa frontière.
 * \castest{<b>floodFill remplit une zone fermee sans deborder.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une image avec un carre interieur cerne d'une bordure d'une autre
 * couleur.<br/>2. Remplir depuis le centre du carre.<br/>
 * \tattendu Seul l'interieur change de couleur ; la bordure reste intacte.
 * }
 */
TEST(PixelOperationsTest, FloodFillZoneFermeeNeDeborde) {
    hmi::DecodedImage image = uniformImage(5, 5, BLUE);
    // Bordure bleue conservee ; interieur 3x3 rouge, rempli en vert depuis son centre.
    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 3; ++x) {
            hmi::setPixel(image, x, y, RED);
        }
    }

    hmi::floodFill(image, 2, 2, GREEN);

    for (int y = 1; y <= 3; ++y) {
        for (int x = 1; x <= 3; ++x) {
            EXPECT_EQ(hmi::pickColor(image, x, y), GREEN) << x << "," << y;
        }
    }
    // La bordure (tout pixel a la frontiere du carre 5x5) reste bleue.
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(hmi::pickColor(image, i, 0), BLUE);
        EXPECT_EQ(hmi::pickColor(image, i, 4), BLUE);
        EXPECT_EQ(hmi::pickColor(image, 0, i), BLUE);
        EXPECT_EQ(hmi::pickColor(image, 4, i), BLUE);
    }
}

/**
 * @brief Une zone ouverte débouchant sur un bord de l'image est remplie jusqu'aux bornes, sans
 *        écriture hors limites.
 * \castest{<b>floodFill remplit une zone ouverte jusqu'au bord sans deborder du tampon.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une image entierement d'une couleur (zone "ouverte" par construction :
 * aucune bordure).<br/>2. Remplir depuis un coin.<br/>
 * \tattendu Toute l'image change de couleur, sans exception ni ecriture hors bornes.
 * }
 */
TEST(PixelOperationsTest, FloodFillZoneOuverteRempliJusquAuBord) {
    hmi::DecodedImage image = uniformImage(4, 3, RED);

    const hmi::PixelRegion region = hmi::floodFill(image, 0, 0, GREEN);

    EXPECT_EQ(region.minX, 0);
    EXPECT_EQ(region.minY, 0);
    EXPECT_EQ(region.maxX, 3);
    EXPECT_EQ(region.maxY, 2);
    for (const std::uint32_t pixel : image.pixels) {
        EXPECT_EQ(pixel, GREEN);
    }
}

/**
 * @brief Remplir sur la couleur déjà présente ne change rien et ne boucle jamais.
 * \castest{<b>floodFill sur la couleur deja presente est un no-op sans boucle infinie.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une grande image uniforme.<br/>2. La remplir avec sa propre
 * couleur.<br/>
 * \tattendu Le test se termine (pas de boucle infinie), l'image est inchangee et la region
 * renvoyee est vide.
 * }
 */
TEST(PixelOperationsTest, FloodFillCouleurDejaPresenteNeChangeRien) {
    hmi::DecodedImage image = uniformImage(50, 50, BLUE);

    const hmi::PixelRegion region = hmi::floodFill(image, 25, 25, BLUE);

    EXPECT_TRUE(region.empty());
    for (const std::uint32_t pixel : image.pixels) {
        EXPECT_EQ(pixel, BLUE);
    }
}

/**
 * @brief Le remplissage fonctionne sur une image d'un seul pixel.
 * \castest{<b>floodFill fonctionne sur une image d'un seul pixel.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire une image 1x1.<br/>2. La remplir d'une autre couleur.<br/>
 * \tattendu L'unique pixel change de couleur, region = (0,0)-(0,0).
 * }
 */
TEST(PixelOperationsTest, FloodFillImageUnPixel) {
    hmi::DecodedImage image = uniformImage(1, 1, RED);

    const hmi::PixelRegion region = hmi::floodFill(image, 0, 0, GREEN);

    EXPECT_EQ(region, (hmi::PixelRegion{0, 0, 0, 0}));
    EXPECT_EQ(hmi::pickColor(image, 0, 0), GREEN);
}

/**
 * @brief Le remplissage d'une image entièrement d'une seule couleur la recolore intégralement.
 * \castest{<b>floodFill sur une image entierement uniforme la recolore integralement.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une image uniforme.<br/>2. La remplir d'une autre couleur depuis un
 * pixel quelconque.<br/>
 * \tattendu Tous les pixels sont de la nouvelle couleur.
 * }
 */
TEST(PixelOperationsTest, FloodFillImageEntierementUniformeRecoloreTout) {
    hmi::DecodedImage image = uniformImage(8, 8, RED);

    hmi::floodFill(image, 4, 4, GREEN);

    for (const std::uint32_t pixel : image.pixels) {
        EXPECT_EQ(pixel, GREEN);
    }
}

/**
 * @brief `readRegion` puis `writeRegion` restituent exactement le contenu d'origine — l'aller-
 *        retour dont dépend `hmi::PixelHistory` pour annuler/refaire.
 * \castest{<b>readRegion puis writeRegion restituent le contenu d'origine.</b><br/>
 * \tcat Unitaire · Operations pixel<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire une region d'une image.<br/>2. Modifier l'image dans cette region.<br/>
 * 3. Reecrire les pixels lus a l'etape 1.<br/>
 * \tattendu L'image retrouve exactement son contenu d'origine dans la region.
 * }
 */
TEST(PixelOperationsTest, ReadRegionPuisWriteRegionRestitueLeContenu) {
    hmi::DecodedImage image = uniformImage(5, 5, RED);
    const hmi::PixelRegion region{1, 1, 3, 3};

    const std::vector<std::uint32_t> before = hmi::readRegion(image, region);
    EXPECT_EQ(before.size(), 9U);

    hmi::floodFill(image, 2, 2, GREEN);
    ASSERT_EQ(hmi::pickColor(image, 2, 2), GREEN);

    hmi::writeRegion(image, region, before);
    for (int y = region.minY; y <= region.maxY; ++y) {
        for (int x = region.minX; x <= region.maxX; ++x) {
            EXPECT_EQ(hmi::pickColor(image, x, y), RED) << x << "," << y;
        }
    }
}
