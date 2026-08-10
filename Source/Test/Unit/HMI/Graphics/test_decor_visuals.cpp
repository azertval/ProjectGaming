/**
 * @file test_decor_visuals.cpp
 * @brief Tests unitaires de la projection couche → calque et du rendu des décors (LOT-49
 *        TACHE-02, `EX-DEC-002`, `EX-ARCH-012`).
 */

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Decor.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/DecorVisuals.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/PlayerSpriteTag.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/RenderMode.h"

namespace {

using core::DecorLayer;

// Textures factices : la composition ne fait que comparer des identites de texture (cf.
// test_quad_recorder.cpp).
int decorTextureStorage = 0;
int missingTextureStorage = 0;
hmi::TextureHandle decorTexture = &decorTextureStorage;
hmi::TextureHandle missingTexture = &missingTextureStorage;

// Textures de reference : un decor "tree.png" charge (48x32 px) et un damier de repli.
hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.missing = missingTexture;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    textures.decors.push_back(hmi::SkinTexture{"tree.png", std::nullopt, decorTexture, 48, 32});
    return textures;
}

// Ajoute une entite decor (Transform + Sprite + DecorVisualTag + RenderLayerTag), meme
// construction que core::buildLevelScene/hmi::GameSession.
core::Entity addDecorEntity(core::World& world, const std::string& asset, DecorLayer decorLayer,
                            std::int32_t rank, core::Vector2 position = {},
                            core::Vector2 scale = core::Vector2{1.0f, 1.0f}) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Transform{position, scale, 0.0f});
    core::Sprite sprite;
    sprite.layer = rank;
    world.addComponent(entity, sprite);
    world.addComponent(entity, hmi::DecorVisualTag{asset, decorLayer});
    world.addComponent(entity, hmi::RenderLayerTag{hmi::decorRenderLayer(decorLayer)});
    return entity;
}

// Ajoute une entite tuile minimale (Transform + Sprite), calque par defaut (RenderLayer::Tile).
void addTile(core::World& world, float x, float y) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{x, y}, core::Vector2{1.0f, 1.0f}, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 0, 16, 16};
    world.addComponent(entity, sprite);
}

// Ajoute une entite personnage minimale (RenderLayer::Player), comme hmi::GameSession::spawnPlayer.
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
 * @brief `core::DecorLayer::Background` et `core::DecorLayer::Decor` projettent sur le même
 * `RenderLayer::Decor` (un seul calque « arrière-plan » réservé) ; `Foreground` projette sur
 * `RenderLayer::Foreground`.
 * \castest{<b>decorRenderLayer projette les trois couches sur les deux calques reserves.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appeler decorRenderLayer pour chaque DecorLayer.<br/>
 * \tattendu Background et Decor donnent RenderLayer::Decor ; Foreground donne
 * RenderLayer::Foreground.
 * }
 */
TEST(DecorVisualsTest, DecorRenderLayerProjetteLesTroisCouches) {
    EXPECT_EQ(hmi::decorRenderLayer(DecorLayer::Background), hmi::RenderLayer::Decor);
    EXPECT_EQ(hmi::decorRenderLayer(DecorLayer::Decor), hmi::RenderLayer::Decor);
    EXPECT_EQ(hmi::decorRenderLayer(DecorLayer::Foreground), hmi::RenderLayer::Foreground);
}

/**
 * @brief resolveDecorAppearance renvoie la texture chargée et ses dimensions réelles pour un
 * asset désigné et disponible.
 * \castest{<b>resolveDecorAppearance renvoie l'asset charge.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre l'apparence d'un decor dont l'asset est charge.<br/>
 * \tattendu La texture et les dimensions reelles de l'asset sont renvoyees.
 * }
 */
TEST(DecorVisualsTest, ResolveDecorAppearanceRenvoieLAssetCharge) {
    const hmi::DecorAppearance appearance =
        hmi::resolveDecorAppearance(hmi::DecorVisualTag{"tree.png"}, testTextures());

    EXPECT_EQ(appearance.texture, decorTexture);
    EXPECT_EQ(appearance.pixelWidth, 48);
    EXPECT_EQ(appearance.pixelHeight, 32);
}

/**
 * @brief Un décor dont l'asset est introuvable retombe sur le damier, à sa taille normale
 * (`EX-NFR-040`) — un décor désigné est toujours censé exister, contrairement au fond.
 * \castest{<b>resolveDecorAppearance retombe sur le damier si l'asset est introuvable.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre l'apparence d'un decor dont l'asset n'est pas charge.<br/>
 * \tattendu Le damier de repli est renvoye, a sa taille normale.
 * }
 */
TEST(DecorVisualsTest, ResolveDecorAppearanceRetombeSurLeDamierSiIntrouvable) {
    const hmi::DecorAppearance appearance =
        hmi::resolveDecorAppearance(hmi::DecorVisualTag{"absent.png"}, testTextures());

    EXPECT_EQ(appearance.texture, missingTexture);
    EXPECT_EQ(appearance.pixelWidth, hmi::MISSING_TEXTURE_SIZE);
    EXPECT_EQ(appearance.pixelHeight, hmi::MISSING_TEXTURE_SIZE);
}

/**
 * @brief Un décor n'est jamais composé en mode Physique : la lecture des collisions reste sans
 * distraction (`EX-ARCH-012`).
 * \castest{<b>Aucun decor n'est compose en mode Physique.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un monde contenant un decor, en mode Physique.<br/>
 * \tattendu Aucune primitive n'est soumise.
 * }
 */
TEST(DecorVisualsTest, AucunDecorComposeEnModePhysique) {
    core::World world;
    addDecorEntity(world, "tree.png", DecorLayer::Decor, 0);

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Physique, testTextures(), 0.0f);

    EXPECT_EQ(scene.size(), 0u);
}

/**
 * @brief En mode Texture, un décor produit un quad dont la taille dérive des dimensions réelles
 * de son asset (et de l'échelle du décor), pas d'une case fixe.
 * \castest{<b>Un decor en mode Texture produit un quad a la taille de son asset.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un monde contenant un decor a l'echelle 2, en mode Texture.<br/>
 * \tattendu Un quad est soumis, de largeur/hauteur (pixels asset / 16) * echelle.
 * }
 */
TEST(DecorVisualsTest, DecorEnModeTextureProduitUnQuadALaTailleDeSonAsset) {
    core::World world;
    addDecorEntity(world, "tree.png", DecorLayer::Decor, 0, core::Vector2{3.0f, 4.0f},
                   core::Vector2{2.0f, 2.0f});

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f);

    ASSERT_EQ(scene.size(), 1u);
    const hmi::SpriteQuad& quad = scene.quads()[0].sprite;
    EXPECT_FLOAT_EQ(quad.x, 3.0f);
    EXPECT_FLOAT_EQ(quad.y, 4.0f);
    EXPECT_FLOAT_EQ(quad.width, 48.0f / hmi::Camera2D::PIXELS_PER_UNIT * 2.0f);
    EXPECT_FLOAT_EQ(quad.height, 32.0f / hmi::Camera2D::PIXELS_PER_UNIT * 2.0f);
    EXPECT_EQ(scene.quads()[0].texture, decorTexture);
}

/**
 * @brief La rotation d'un décor (`core::Transform::rotation`) atteint le quad composé : nécessaire
 * pour que la poignée de rotation de l'éditeur (`LOT-50`) ait un effet visible, contrairement au
 * choix (révolu) de LOT-49 TACHE-02 de l'ignorer.
 * \castest{<b>La rotation d'un decor atteint le quad compose.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un decor dont le Transform porte une rotation non nulle.<br/>
 * \tattendu Le quad soumis porte la meme rotation.
 * }
 */
TEST(DecorVisualsTest, LaRotationDUnDecorAtteintLeQuadCompose) {
    core::World world;
    const core::Entity entity = addDecorEntity(world, "tree.png", DecorLayer::Decor, 0);
    world.getComponent<core::Transform>(entity).rotation = 1.2f;

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f);

    ASSERT_EQ(scene.size(), 1u);
    EXPECT_FLOAT_EQ(scene.quads()[0].sprite.rotation, 1.2f);
}

/**
 * @brief Un décor de premier plan est soumis après le personnage ; un décor d'arrière-plan avant
 * les tuiles (`EX-DEC-002`, contrat de lecture du lot).
 * \castest{<b>Premier plan apres le personnage, arriere-plan avant les tuiles.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un monde avec un decor d'arriere-plan, une tuile, le personnage et un
 * decor de premier plan.<br/>
 * \tattendu L'ordre des calques rencontres est Decor, Tile, Player, Foreground.
 * }
 */
TEST(DecorVisualsTest, PremierPlanApresLePersonnageArrierePlanAvantLesTuiles) {
    core::World world;
    addDecorEntity(world, "tree.png", DecorLayer::Background, 0, core::Vector2{0.0f, 0.0f});
    addTile(world, 1.0f, 1.0f);
    addPlayer(world);
    addDecorEntity(world, "tree.png", DecorLayer::Foreground, 0, core::Vector2{2.0f, 2.0f});

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f);
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    EXPECT_TRUE(recorder.isLayerOrderRespected()) << recorder.describe();
    const std::vector<hmi::RenderLayer> sequence = recorder.layerSequence();
    ASSERT_EQ(sequence.size(), 4u);
    EXPECT_EQ(sequence[0], hmi::RenderLayer::Decor);
    EXPECT_EQ(sequence[1], hmi::RenderLayer::Tile);
    EXPECT_EQ(sequence[2], hmi::RenderLayer::Player);
    EXPECT_EQ(sequence[3], hmi::RenderLayer::Foreground);
}

/**
 * @brief Deux décors de la même couche et du même asset (donc du même groupe de texture)
 * respectent l'ordre de leur rang (`core::Sprite::layer`, LOT-49 TACHE-01).
 * \castest{<b>Deux decors de la meme couche respectent l'ordre du vecteur.</b><br/>
 * \tcat Unitaire · Decor Visuals<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer deux decors de meme asset et de meme couche, rangs 0 puis 1.<br/>
 * \tattendu Le rang 0 est soumis avant le rang 1.
 * }
 */
TEST(DecorVisualsTest, OrdreIntraCoucheRespecteLeRangDuVecteur) {
    core::World world;
    addDecorEntity(world, "tree.png", DecorLayer::Decor, 1, core::Vector2{5.0f, 5.0f});
    addDecorEntity(world, "tree.png", DecorLayer::Decor, 0, core::Vector2{1.0f, 1.0f});

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f);
    scene.sort();

    ASSERT_EQ(scene.size(), 2u);
    EXPECT_FLOAT_EQ(scene.quads()[0].sprite.x, 1.0f);
    EXPECT_FLOAT_EQ(scene.quads()[1].sprite.x, 5.0f);
}
