// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_parallax.cpp
 * @brief Tests unitaires du décalage de parallaxe, relatif à la salle courante (LOT-49 TACHE-03,
 *        généralisé au LOT-69 : le facteur est porté par le plan, plus par une couche figée).
 */

#include <gtest/gtest.h>

#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/Parallax.h"

/**
 * @brief À facteur `1.0`, la position de rendu est strictement inchangée, quel que soit le
 * cadrage de la caméra.
 * \castest{<b>Facteur 1 laisse la position inchangee.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la position de rendu a facteur 1.0.<br/>
 * \tattendu La position renvoyee est identique a la position simulee.
 * }
 */
TEST(ParallaxTest, FacteurUnLaissePositionInchangee) {
    const core::Rect bounds{core::Vector2{40.0f, 45.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 position{12.0f, 7.5f};

    const core::Vector2 rendered = hmi::parallaxRenderPosition(position, 1.0f, bounds);

    EXPECT_FLOAT_EQ(rendered.x, position.x);
    EXPECT_FLOAT_EQ(rendered.y, position.y);
}

/**
 * @brief Deux facteurs équidistants de `1.0` produisent des décalages de sens opposés et de même
 * amplitude (proportionnalité au facteur).
 * \castest{<b>Des facteurs opposes autour de 1 produisent des decalages opposes.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le decalage pour un facteur inferieur et un facteur superieur a 1,
 * equidistants.<br/>
 * \tattendu Les deux decalages sont de signe oppose et de meme amplitude.
 * }
 */
TEST(ParallaxTest, FacteursOpposesProduisentDesDecalagesOpposesEtProportionnels) {
    const core::Rect bounds{core::Vector2{0.0f, 0.0f}, core::Vector2{100.0f, 100.0f}};
    const core::Vector2 position{80.0f, 50.0f};  // centre de bounds : (50, 50)

    const core::Vector2 lower = hmi::parallaxRenderPosition(position, 0.5f, bounds);
    const core::Vector2 higher = hmi::parallaxRenderPosition(position, 1.5f, bounds);

    const float offsetLower = lower.x - position.x;
    const float offsetHigher = higher.x - position.x;
    EXPECT_FLOAT_EQ(offsetLower, -offsetHigher);
    EXPECT_NE(offsetLower, 0.0f);
}

/**
 * @brief Le décalage se calcule depuis le centre de la salle **courante** : un contenu à la même
 * position relative dans deux salles différentes se retrouve au même endroit à l'écran — c'est le
 * test qui verrouille l'arbitrage (`decors.md`).
 * \castest{<b>Meme position relative -> meme position ecran dans deux salles.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer un point a la meme position RELATIVE dans deux salles de centres
 * differents.<br/>2. Calculer sa position de rendu dans chaque salle.<br/>
 * \tattendu Ramenee au centre de sa salle, la position de rendu est identique dans les deux cas.
 * }
 */
TEST(ParallaxTest, MemePositionRelativeDonneMemePositionEcranEntreSalles) {
    constexpr float FACTOR = 0.5f;
    const core::Vector2 relativeOffset{6.0f, -2.0f};

    const core::Vector2 room1Center{50.0f, 50.0f};
    const core::Rect room1{room1Center - core::Vector2{10.0f, 5.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 position1 = room1Center + relativeOffset;

    const core::Vector2 room2Center{500.0f, 200.0f};  // salle tres eloignee dans le niveau
    const core::Rect room2{room2Center - core::Vector2{10.0f, 5.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 position2 = room2Center + relativeOffset;

    const core::Vector2 rendered1 = hmi::parallaxRenderPosition(position1, FACTOR, room1);
    const core::Vector2 rendered2 = hmi::parallaxRenderPosition(position2, FACTOR, room2);

    // Position ECRAN : ramenee au centre de sa propre salle (la camera est toujours centree
    // dessus, EX-REN-015) -- c'est cette position relative qui doit coincider.
    const core::Vector2 screen1 = rendered1 - room1Center;
    const core::Vector2 screen2 = rendered2 - room2Center;
    EXPECT_FLOAT_EQ(screen1.x, screen2.x);
    EXPECT_FLOAT_EQ(screen1.y, screen2.y);
}

/**
 * @brief roundToScreenPixel arrondit une position monde au pixel écran entier le plus proche.
 * \castest{<b>roundToScreenPixel arrondit au pixel ecran le plus proche.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Arrondir une position fractionnaire a 16 px/unite.<br/>
 * \tattendu Le resultat correspond a un nombre entier de pixels ecran.
 * }
 */
TEST(ParallaxTest, RoundToScreenPixelArrondiAuPixelEcran) {
    const core::Vector2 rounded = hmi::roundToScreenPixel(core::Vector2{1.03f, 2.49f}, 16.0f);

    EXPECT_FLOAT_EQ(rounded.x, 1.0f);  // round(1.03*16)=16 -> 16/16 = 1.0
    EXPECT_FLOAT_EQ(rounded.y, 2.5f);  // round(2.49*16)=40 -> 40/16 = 2.5
}

/**
 * @brief `parallaxModelPosition` est l'inverse exact de `parallaxRenderPosition` : convertir une
 * position modèle en rendu puis revenir en arrière redonne la position de départ.
 * \castest{<b>parallaxModelPosition est l'inverse de parallaxRenderPosition.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la position de rendu d'une position modele (facteur != 1).<br/>2. Appliquer
 * parallaxModelPosition au resultat.<br/>
 * \tattendu La position modele d'origine est retrouvee.
 * }
 */
TEST(ParallaxTest, ParallaxModelPositionEstLInverseDeParallaxRenderPosition) {
    const core::Rect bounds{core::Vector2{0.0f, 0.0f}, core::Vector2{100.0f, 100.0f}};
    const core::Vector2 modelPosition{80.0f, 30.0f};
    constexpr float FACTOR = 0.5f;

    const core::Vector2 rendered = hmi::parallaxRenderPosition(modelPosition, FACTOR, bounds);
    const core::Vector2 roundTrip = hmi::parallaxModelPosition(rendered, FACTOR, bounds);

    EXPECT_FLOAT_EQ(roundTrip.x, modelPosition.x);
    EXPECT_FLOAT_EQ(roundTrip.y, modelPosition.y);
}

/**
 * @brief À facteur `1.0`, `parallaxModelPosition` laisse aussi la position inchangée (couche de
 * référence, symétrique de `parallaxRenderPosition`).
 * \castest{<b>parallaxModelPosition a facteur 1 laisse la position inchangee.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appliquer parallaxModelPosition a facteur 1.0.<br/>
 * \tattendu La position renvoyee est identique a la position de rendu.
 * }
 */
TEST(ParallaxTest, ParallaxModelPositionFacteurUnLaissePositionInchangee) {
    const core::Rect bounds{core::Vector2{10.0f, 10.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 renderPosition{15.0f, 12.0f};

    const core::Vector2 model = hmi::parallaxModelPosition(renderPosition, 1.0f, bounds);

    EXPECT_FLOAT_EQ(model.x, renderPosition.x);
    EXPECT_FLOAT_EQ(model.y, renderPosition.y);
}
