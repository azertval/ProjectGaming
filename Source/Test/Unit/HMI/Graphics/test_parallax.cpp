// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_parallax.cpp
 * @brief Tests unitaires du facteur de parallaxe par couche et du décalage relatif à la salle
 *        courante (LOT-49 TACHE-03, `EX-DEC-006`).
 */

#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Decor.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/DecorVisuals.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Graphics/RenderMode.h"

namespace {

using core::DecorLayer;

int decorTextureStorage = 0;
hmi::TextureHandle decorTexture = &decorTextureStorage;

// Un decor "tree.png" de 16x16 px (une unite monde de cote), pour des boites de culling simples.
hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.decors.push_back(hmi::SkinTexture{"tree.png", std::nullopt, decorTexture, 16, 16});
    return textures;
}

// Camera de reference : viewport 320x160 px, zoom 1 (echelle 16 px/unite) -> 20x10 unites
// visibles, centree sur @p center.
hmi::Camera2D referenceCamera(core::Vector2 center) {
    hmi::Camera2D camera(320, 160);
    camera.setCenter(center);
    camera.setZoom(1.0f);
    return camera;
}

// Ajoute une entite decor a la position/couche donnee.
void addDecorEntity(core::World& world, core::Vector2 position, DecorLayer layer) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Transform{position, core::Vector2{1.0f, 1.0f}, 0.0f});
    core::Sprite sprite;
    world.addComponent(entity, sprite);
    world.addComponent(entity, hmi::DecorVisualTag{"tree.png", layer});
    world.addComponent(entity, hmi::RenderLayerTag{hmi::decorRenderLayer(layer)});
}

}  // namespace

/**
 * @brief La couche `core::DecorLayer::Decor` porte le facteur de référence `1.0` ; `Background`
 * est inférieur (défile moins vite), `Foreground` supérieur (défile plus vite).
 * \castest{<b>Le facteur de la couche Decor vaut 1.0, valeur de reference.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire parallaxFactor pour les trois couches.<br/>
 * \tattendu Decor vaut 1.0 ; Background < 1.0 < Foreground.
 * }
 */
TEST(ParallaxTest, LeFacteurDeLaCoucheDecorVautUn) {
    EXPECT_FLOAT_EQ(hmi::parallaxFactor(DecorLayer::Decor), 1.0f);
    EXPECT_LT(hmi::parallaxFactor(DecorLayer::Background), 1.0f);
    EXPECT_GT(hmi::parallaxFactor(DecorLayer::Foreground), 1.0f);
}

/**
 * @brief À facteur `1.0`, la position de rendu est strictement inchangée, quel que soit le
 * cadrage de la caméra.
 * \castest{<b>Facteur 1 laisse la position inchangee.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la position de rendu d'un decor a facteur 1.0.<br/>
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
 * @brief Le décalage se calcule depuis le centre de la salle **courante** : un décor à la même
 * position relative dans deux salles différentes se retrouve au même endroit à l'écran — c'est le
 * test qui verrouille l'arbitrage (`decors.md`).
 * \castest{<b>Meme position relative -> meme position ecran dans deux salles.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer un decor a la meme position RELATIVE dans deux salles de centres
 * differents.<br/>2. Calculer sa position de rendu dans chaque salle.<br/>
 * \tattendu Ramenee au centre de sa salle, la position de rendu est identique dans les deux cas.
 * }
 */
TEST(ParallaxTest, MemePositionRelativeDonneMemePositionEcranEntreSalles) {
    constexpr float FACTOR = 0.5f;
    const core::Vector2 relativeOffset{6.0f, -2.0f};

    const core::Vector2 room1Center{50.0f, 50.0f};
    const core::Rect room1{room1Center - core::Vector2{10.0f, 5.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 decor1Position = room1Center + relativeOffset;

    const core::Vector2 room2Center{500.0f, 200.0f};  // salle tres eloignee dans le niveau
    const core::Rect room2{room2Center - core::Vector2{10.0f, 5.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 decor2Position = room2Center + relativeOffset;

    const core::Vector2 rendered1 = hmi::parallaxRenderPosition(decor1Position, FACTOR, room1);
    const core::Vector2 rendered2 = hmi::parallaxRenderPosition(decor2Position, FACTOR, room2);

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

/**
 * @brief Un décor pris en défaut par le culling à sa position simulée devient visible une fois la
 * parallaxe appliquée (échelle réduite, couche arrière-plan) — le culling juge la position
 * **décalée**.
 * \castest{<b>Un decor parallaxe visible n'est pas ecarte par le culling.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un decor hors cadre a sa position simulee mais dont le facteur
 * arriere-plan le ramene dans le cadrage.<br/>
 * \tattendu Le decor est compose (non ecarte).
 * }
 */
TEST(ParallaxTest, DecorParallaxeVisibleNEstPasEcarte) {
    const core::Vector2 center{100.0f, 100.0f};
    const hmi::Camera2D camera = referenceCamera(center);
    const core::Vector2 decorPosition{118.0f, 100.0f};  // 18 unites a droite du centre

    // Sans parallaxe (couche Decor, facteur 1) : hors du cadrage (marge comprise) -> ecarte.
    {
        core::World world;
        addDecorEntity(world, decorPosition, DecorLayer::Decor);
        hmi::ComposedScene scene;
        scene.setVisibleBounds(camera.visibleBounds());
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 &camera);
        EXPECT_EQ(scene.size(), 0u);
    }

    // Avec parallaxe (couche arriere-plan, facteur < 1) : ramene dans le cadrage -> visible.
    {
        core::World world;
        addDecorEntity(world, decorPosition, DecorLayer::Background);
        hmi::ComposedScene scene;
        scene.setVisibleBounds(camera.visibleBounds());
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 &camera);
        EXPECT_EQ(scene.size(), 1u);
    }
}

/**
 * @brief Un décor visible à sa position simulée peut être écarté une fois la parallaxe appliquée
 * (couche premier plan, facteur > 1, l'exagère hors cadre).
 * \castest{<b>Un decor parallaxe hors cadre est ecarte par le culling.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un decor dans le cadrage a sa position simulee mais dont le facteur
 * premier plan le pousse hors du cadrage.<br/>
 * \tattendu Le decor est ecarte (non compose).
 * }
 */
TEST(ParallaxTest, DecorParallaxeHorsCadreEstEcarte) {
    const core::Vector2 center{100.0f, 100.0f};
    const hmi::Camera2D camera = referenceCamera(center);
    const core::Vector2 decorPosition{109.7f, 100.0f};  // 9.7 unites a droite du centre

    // Sans parallaxe (couche Decor, facteur 1) : dans le cadrage -> visible.
    {
        core::World world;
        addDecorEntity(world, decorPosition, DecorLayer::Decor);
        hmi::ComposedScene scene;
        scene.setVisibleBounds(camera.visibleBounds());
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 &camera);
        EXPECT_EQ(scene.size(), 1u);
    }

    // Avec parallaxe (couche premier plan, facteur > 1) : pousse hors du cadrage -> ecarte.
    {
        core::World world;
        addDecorEntity(world, decorPosition, DecorLayer::Foreground);
        hmi::ComposedScene scene;
        scene.setVisibleBounds(camera.visibleBounds());
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 &camera);
        EXPECT_EQ(scene.size(), 0u);
    }
}

/**
 * @brief `applyDecorParallax = false` (mode de cadrage *suivi*, LOT-64) neutralise le décalage de
 * parallaxe des couches Fond/Premier plan -- le décor reste composé à sa position simulée exacte,
 * comme la couche `Decor`, sans pour autant désactiver son alignement au pixel écran.
 * \castest{<b>applyDecorParallax=false neutralise le decalage de parallaxe.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un décor de couche Fond, décalé du centre de la caméra, avec
 * `applyDecorParallax` vrai puis faux.<br/>2. Comparer la position du quad composé à la position
 * simulée du décor.<br/>
 * \tattendu Avec `true` (défaut), la position composée diffère de la position simulée (parallaxe
 * appliquée) ; avec `false`, elle lui est exactement égale (aucun décalage), comme la couche
 * `Decor`.
 * }
 */
TEST(ParallaxTest, ApplyDecorParallaxFauxNeutraliseLeDecalage) {
    const core::Vector2 center{100.0f, 100.0f};
    const hmi::Camera2D camera = referenceCamera(center);
    const core::Vector2 decorPosition{104.0f, 100.0f};  // decale du centre : la parallaxe agirait.

    {
        core::World world;
        addDecorEntity(world, decorPosition, DecorLayer::Background);
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 &camera, hmi::LayerVisibility{}, /*applyDecorParallax=*/true);
        ASSERT_EQ(scene.size(), 1u);
        EXPECT_NE(scene.quads()[0].sprite.x, decorPosition.x);
    }
    {
        core::World world;
        addDecorEntity(world, decorPosition, DecorLayer::Background);
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 &camera, hmi::LayerVisibility{}, /*applyDecorParallax=*/false);
        ASSERT_EQ(scene.size(), 1u);
        EXPECT_FLOAT_EQ(scene.quads()[0].sprite.x, decorPosition.x);
        EXPECT_FLOAT_EQ(scene.quads()[0].sprite.y, decorPosition.y);
    }
}
