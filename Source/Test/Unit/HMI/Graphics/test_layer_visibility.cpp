// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_layer_visibility.cpp
 * @brief Tests unitaires du jeu de visibilités par calque du mode d'inspection de l'éditeur
 * (`LOT-51`, `EX-EDIT-044`).
 */

#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Decor.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/DecorVisuals.h"
#include "HMI/Graphics/LayerVisibility.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderLayer.h"

namespace {

using core::DecorLayer;

// Textures factices : la composition ne fait que comparer des identites (cf. test_quad_recorder).
int textureStorage = 0;
int missingStorage = 0;
hmi::TextureHandle texture = &textureStorage;

/// Textures de reference des tests (un decor "tree.png" charge, et le damier de repli).
hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = texture;
    textures.atlasWidth = 16;
    textures.atlasHeight = 16;
    textures.missing = &missingStorage;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    textures.decors.push_back(hmi::SkinTexture{"tree.png", std::nullopt, texture, 48, 32});
    return textures;
}

/// Ajoute une entite tuile minimale (Transform + Sprite), calque par defaut (RenderLayer::Tile).
void addTile(core::World& world) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{0.0f, 0.0f}, core::Vector2{1.0f, 1.0f}, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 0, 16, 16};
    world.addComponent(entity, sprite);
}

/// Ajoute une entite decor (Transform + Sprite + DecorVisualTag + RenderLayerTag), meme
/// construction que core::buildLevelScene/hmi::GameSession (cf. test_decor_visuals.cpp).
void addDecor(core::World& world, DecorLayer decorLayer) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{0.0f, 0.0f}, core::Vector2{1.0f, 1.0f}, 0.0f});
    core::Sprite sprite;
    world.addComponent(entity, sprite);
    world.addComponent(entity, hmi::DecorVisualTag{"tree.png", decorLayer});
    world.addComponent(entity, hmi::RenderLayerTag{hmi::decorRenderLayer(decorLayer)});
}

/// Ajoute une entite personnage minimale (RenderLayer::Player), comme
/// hmi::GameSession::spawnPlayer (cf. test_decor_visuals.cpp).
void addPlayer(core::World& world) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{2.0f, 2.0f}, core::Vector2{0.4f, 0.8f}, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 80, 16, 16};
    world.addComponent(entity, sprite);
    world.addComponent(entity, hmi::PlayerSpriteTag{});
    world.addComponent(entity, hmi::RenderLayerTag{hmi::RenderLayer::Player});
}

}  // namespace

/**
 * @brief Tous les calques sont visibles par défaut.
 * \castest{<b>Tous les calques sont visibles par defaut.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un jeu de visibilités par defaut.<br/>2. Interroger chaque calque.<br/>
 * \tattendu Tous les calques répondent visibles.
 * }
 */
TEST(LayerVisibilityTest, TousLesCalquesSontVisiblesParDefaut) {
    const hmi::LayerVisibility visibility;

    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Background));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Decor));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Shadow));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Tile));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Object));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Player));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Foreground));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::UI));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::EditorOverlay));
}

/**
 * @brief Masquer un calque ne change que ce calque.
 * \castest{<b>Masquer un calque ne change que ce calque.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Masquer le calque Décor.<br/>2. Interroger Décor et Personnage.<br/>
 * \tattendu Décor est masqué, Personnage reste visible.
 * }
 */
TEST(LayerVisibilityTest, MasquerUnCalqueNeChangeQueCeCalque) {
    hmi::LayerVisibility visibility;
    visibility.setVisible(hmi::RenderLayer::Decor, false);

    EXPECT_FALSE(visibility.visible(hmi::RenderLayer::Decor));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Player));
}

/**
 * @brief showAll rétablit tous les calques, même après plusieurs masquages.
 * \castest{<b>showAll retablit tous les calques.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Masquer plusieurs calques.<br/>2. Appeler showAll.<br/>
 * \tattendu Tous les calques redeviennent visibles.
 * }
 */
TEST(LayerVisibilityTest, ShowAllRetablitTousLesCalques) {
    hmi::LayerVisibility visibility;
    visibility.setVisible(hmi::RenderLayer::Background, false);
    visibility.setVisible(hmi::RenderLayer::Player, false);
    visibility.setVisible(hmi::RenderLayer::Foreground, false);

    visibility.showAll();

    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Background));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Player));
    EXPECT_TRUE(visibility.visible(hmi::RenderLayer::Foreground));
}

/**
 * @brief Un calque masqué (Personnage) n'émet aucune primitive ; un autre calque (Décor de premier
 * plan) reste inchangé.
 * \castest{<b>Un calque masque n'emet aucune primitive ; les autres sont inchanges.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une entite Personnage et une entite Decor de premier plan.<br/>2. Masquer
 * Personnage et composer en mode Texture.<br/>
 * \tattendu Aucune primitive sur Personnage ; une primitive sur Decor de premier plan.
 * }
 */
TEST(LayerVisibilityTest, CalqueMasqueNEmetAucunePrimitiveLesAutresInchanges) {
    core::World world;
    addPlayer(world);
    addDecor(world, DecorLayer::Foreground);

    hmi::LayerVisibility visibility;
    visibility.setVisible(hmi::RenderLayer::Player, false);

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f, nullptr,
                             visibility);

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Player), 0) << recorder.describe();
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Foreground), 1) << recorder.describe();
}

/**
 * @brief Combinaisons de visibilité : deux calques visibles, un seul, aucun.
 * \castest{<b>Combinaisons de visibilite : deux, un seul, aucun.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler un Decor d'arriere-plan et un Personnage.<br/>2. Composer en mode Texture
 * avec les deux visibles, puis un seul, puis aucun.<br/>
 * \tattendu Le nombre de primitives suit exactement les calques visibles a chaque etape.
 * }
 */
TEST(LayerVisibilityTest, CombinaisonsDeuxUnSeulAucun) {
    core::World world;
    addDecor(world, DecorLayer::Background);
    addPlayer(world);

    hmi::LayerVisibility visibility;

    // Deux calques visibles (defaut) : les deux entites sont composees.
    {
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 nullptr, visibility);
        EXPECT_EQ(scene.size(), 2u);
    }

    // Un seul calque visible : une seule entite composee.
    visibility.setVisible(hmi::RenderLayer::Player, false);
    {
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 nullptr, visibility);
        EXPECT_EQ(scene.size(), 1u);
        EXPECT_EQ(scene.quads().front().layer, hmi::RenderLayer::Decor);
    }

    // Aucun calque visible : rien n'est compose.
    visibility.setVisible(hmi::RenderLayer::Decor, false);
    {
        hmi::ComposedScene scene;
        hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f,
                                 nullptr, visibility);
        EXPECT_EQ(scene.size(), 0u);
    }
}

/**
 * @brief Un jeu de visibilités par défaut ne change rien à la composition d'une scène — la valeur
 * par défaut passée implicitement par `hmi::GameSession` (qui ne fournit jamais cet argument) doit
 * rester strictement neutre.
 * \castest{<b>Un jeu de visibilite par defaut est neutre sur la composition.</b><br/>
 * \tcat Unitaire · Visibilité par calque<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler tuile, decor et personnage.<br/>2. Composer avec et sans jeu de visibilites
 * explicite, en mode Texture.<br/>
 * \tattendu Les deux scenes ont le meme nombre de primitives.
 * }
 */
TEST(LayerVisibilityTest, DefautNeutreSurLaCompositionDUneScene) {
    core::World world;
    addTile(world);
    addDecor(world, DecorLayer::Decor);
    addPlayer(world);

    hmi::ComposedScene withoutVisibility;
    hmi::composeWorldSprites(withoutVisibility, world, hmi::RenderMode::Texture, testTextures(),
                             0.0f);

    hmi::ComposedScene withDefaultVisibility;
    hmi::composeWorldSprites(withDefaultVisibility, world, hmi::RenderMode::Texture, testTextures(),
                             0.0f, nullptr, hmi::LayerVisibility{});

    EXPECT_EQ(withoutVisibility.size(), withDefaultVisibility.size());
    EXPECT_EQ(withoutVisibility.size(), 3u);
}
