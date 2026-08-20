// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pixel_art_scale.cpp
 * @brief Tests unitaires du facteur d'agrandissement des écrans du jeu (`LOT-68`, `EX-IHM-070`).
 */

#include <gtest/gtest.h>

#include "HMI/Interface/PixelArtScale.h"

/**
 * @brief Le facteur est toujours un entier d'au moins 1 et n'excède jamais le plafond, pour
 *        n'importe quelle hauteur — y compris absurde (nulle, négative, gigantesque). Un facteur
 *        nul ferait disparaître l'interface ; un facteur non borné remplirait l'écran de trois
 *        entrées de menu.
 * \castest{<b>Le facteur reste entier, superieur a 1 et borne par le plafond.</b><br/>
 * \tcat Unitaire · Echelle pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander le facteur pour des hauteurs allant du negatif au tres grand.<br/>
 * \tattendu Chaque resultat est compris entre 1 et PIXEL_ART_MAX_SCALE inclus.
 * }
 */
TEST(PixelArtScaleTest, FacteurToujoursBorneEntreUnEtLePlafond) {
    constexpr int HEIGHTS[] = {-1080, -1, 0, 1, 200, 359, 360, 719, 720, 1079, 1080, 2160, 100000};
    for (const int height : HEIGHTS) {
        const int scale = hmi::pixelArtScale(height);
        EXPECT_GE(scale, 1) << "hauteur " << height;
        EXPECT_LE(scale, hmi::PIXEL_ART_MAX_SCALE) << "hauteur " << height;
    }
}

/**
 * @brief Le facteur ne décroît jamais quand la fenêtre grandit. C'est ce qui rend le
 *        redimensionnement prévisible : agrandir la fenêtre ne peut pas rapetisser l'interface.
 * \castest{<b>Le facteur croit de facon monotone avec la hauteur de fenetre.</b><br/>
 * \tcat Unitaire · Echelle pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Parcourir les hauteurs de 0 a 2400 par pas de 1.<br/>2. Comparer chaque facteur au
 * precedent.<br/>
 * \tattendu Le facteur ne diminue jamais.
 * }
 */
TEST(PixelArtScaleTest, FacteurMonotoneEnHauteur) {
    int previous = hmi::pixelArtScale(0);
    for (int height = 1; height <= 2400; ++height) {
        const int scale = hmi::pixelArtScale(height);
        EXPECT_GE(scale, previous) << "le facteur a diminue a la hauteur " << height;
        previous = scale;
    }
}

/**
 * @brief Une hauteur juste sous un seuil garde le facteur **inférieur** : la maquette doit tenir
 *        ENTIÈREMENT dans la fenêtre. Arrondir plutôt que tronquer ferait passer une fenêtre de
 *        700 px à l'échelle 2, pour laquelle il manque 20 px — la dernière entrée du menu
 *        passerait sous le bord.
 * \castest{<b>Une hauteur juste insuffisante ne declenche pas le facteur superieur.</b><br/>
 * \tcat Unitaire · Echelle pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander le facteur juste avant et juste apres chaque multiple de la hauteur de
 * base.<br/>
 * \tattendu Le facteur ne monte qu'a partir du multiple exact, jamais avant.
 * }
 */
TEST(PixelArtScaleTest, LeSeuilEstAtteintParDefautJamaisParArrondi) {
    for (int factor = 1; factor <= hmi::PIXEL_ART_MAX_SCALE; ++factor) {
        const int threshold = factor * hmi::PIXEL_ART_BASE_HEIGHT;
        EXPECT_EQ(hmi::pixelArtScale(threshold), factor);
        EXPECT_EQ(hmi::pixelArtScale(threshold - 1), factor - 1 < 1 ? 1 : factor - 1);
    }
}

/**
 * @brief Les trois hauteurs de fenêtre courantes tombent sur les facteurs annoncés dans les
 *        maquettes : 720p à ×2, 1080p à ×3, et 1440p reste à ×3 (plafond). Ce test fige la
 *        promesse faite au level designer, pas seulement la formule.
 * \castest{<b>Les hauteurs d'ecran courantes donnent les facteurs annonces.</b><br/>
 * \tcat Unitaire · Echelle pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander le facteur pour 360, 720, 1080 et 1440 pixels logiques.<br/>
 * \tattendu Respectivement 1, 2, 3 et 3.
 * }
 */
TEST(PixelArtScaleTest, HauteursCourantesDonnentLesFacteursAnnonces) {
    EXPECT_EQ(hmi::pixelArtScale(360), 1);
    EXPECT_EQ(hmi::pixelArtScale(720), 2);
    EXPECT_EQ(hmi::pixelArtScale(1080), 3);
    EXPECT_EQ(hmi::pixelArtScale(1440), 3);
}
