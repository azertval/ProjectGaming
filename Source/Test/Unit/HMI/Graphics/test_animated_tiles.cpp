/**
 * @file test_animated_tiles.cpp
 * @brief Tests unitaires des tuiles animées (`LOT-46` TACHE-05) : horloge partagée par asset,
 *        mise en phase, exclusion des combinaisons non supportées. Sans GPU.
 */

#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include "Core/Ecs/AnimationClip.h"
#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileSkinTag.h"

namespace {

constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;
constexpr float STEP = 1.0f / 60.0f;

// Jeu de clips d'un asset anime simple (eau) : un seul clip boucle de 4 images, 0,1 s chacune.
std::shared_ptr<const core::ClipSet> waterClips() {
    auto clips = std::make_shared<core::ClipSet>();
    core::AnimationClip wave;
    wave.name = "wave";
    wave.frames = {0, 1, 2, 3};
    wave.frameDuration = 0.1f;
    wave.endMode = core::ClipEndMode::Loop;
    clips->addClip(wave);
    return clips;
}

core::Animation freshWaterAnimation() {
    core::Animation animation;
    animation.clips = waterClips();
    return animation;
}

hmi::AnimationDescription waterDescription() {
    hmi::AnimationDescription description;
    description.frameWidth = TILE;
    description.frameHeight = TILE;
    description.clips = *waterClips();
    return description;
}

int storageA = 0;
int storageB = 0;

}  // namespace

TEST(AnimatedTilesTest, SkinAnimeProduitDesRegionsDifferentesAuFilDesPas) {
    core::Animation animation = freshWaterAnimation();
    const hmi::AnimationDescription description = waterDescription();

    const core::AtlasRegion first = hmi::AnimationCatalog::currentFrameRegion(description, animation);
    for (int i = 0; i < 6; ++i) {  // 6 pas = 0,1 s : une image complete a la cadence du clip.
        core::advanceAnimation(animation, STEP);
    }
    const core::AtlasRegion second = hmi::AnimationCatalog::currentFrameRegion(description, animation);

    EXPECT_NE(first.x, second.x);
    EXPECT_EQ(first.y, second.y);  // spritesheet a un seul rang.
}

TEST(AnimatedTilesTest, SkinNonAnimeProduitUneRegionConstante) {
    // Sans animatedFrame renseigne (asset non anime, TACHE-03) : resolveTileAppearance retombe
    // sur l'image entiere, quel que soit le nombre d'appels.
    hmi::SkinCatalog catalog;
    catalog.assign("defaut", core::TileType::Solid, hmi::SkinEntry{"stone.png", hmi::SkinMode::Single});

    hmi::SceneTextures textures;
    textures.atlas = &storageA;
    textures.atlasWidth = TILE * 5;
    textures.atlasHeight = TILE * 5;
    textures.skins = {hmi::SkinTexture{"stone.png", std::nullopt, &storageB, TILE, TILE, std::nullopt}};
    textures.skinCatalog = &catalog;
    textures.skinSet = "defaut";

    const hmi::TileSkinTag tag{core::TileType::Solid, 0};
    const hmi::TileAppearance first =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, core::AtlasRegion{}, &tag, textures);
    const hmi::TileAppearance second =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, core::AtlasRegion{}, &tag, textures);

    EXPECT_EQ(first.region.x, 0);
    EXPECT_EQ(first.region.x, second.region.x);
    EXPECT_EQ(first.region.width, TILE);
}

TEST(AnimatedTilesTest, SkinAnimeUtiliseLaRegionCouranteDeAnimatedFrame) {
    hmi::SkinCatalog catalog;
    catalog.assign("defaut", core::TileType::Solid, hmi::SkinEntry{"water.png", hmi::SkinMode::Single});

    const core::AtlasRegion currentFrame{TILE * 2, 0, TILE, TILE};  // 3e image de la spritesheet.
    hmi::SceneTextures textures;
    textures.atlas = &storageA;
    textures.atlasWidth = TILE * 5;
    textures.atlasHeight = TILE * 5;
    textures.skins = {
        hmi::SkinTexture{"water.png", std::nullopt, &storageB, TILE * 4, TILE, currentFrame}};
    textures.skinCatalog = &catalog;
    textures.skinSet = "defaut";

    const hmi::TileSkinTag tag{core::TileType::Solid, 0};
    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, core::AtlasRegion{}, &tag, textures);

    EXPECT_EQ(appearance.region.x, currentFrame.x);
    EXPECT_EQ(appearance.region.width, TILE);
}

TEST(AnimatedTilesTest, MiseEnPhaseDeuxTuilesDuMemeTypeAffichentLaMemeImageAuMemePas) {
    // Deux "tuiles" independantes, meme jeu de clips partage : si chacune progressait pour son
    // propre compte (au lieu d'une horloge unique par asset), elles resteraient identiques tant
    // que la sequence de pas est identique -- ce test verifie precisement cette garantie de
    // determinisme, condition necessaire a la mise en phase reelle (horloge unique, LOT-46
    // TACHE-05).
    core::Animation tileOne = freshWaterAnimation();
    core::Animation tileTwo = freshWaterAnimation();
    const hmi::AnimationDescription description = waterDescription();

    for (int step = 0; step < 37; ++step) {
        core::advanceAnimation(tileOne, STEP);
        core::advanceAnimation(tileTwo, STEP);
        const core::AtlasRegion regionOne = hmi::AnimationCatalog::currentFrameRegion(description, tileOne);
        const core::AtlasRegion regionTwo = hmi::AnimationCatalog::currentFrameRegion(description, tileTwo);
        ASSERT_EQ(regionOne.x, regionTwo.x) << "pas " << step;
        ASSERT_EQ(tileOne.frameIndex, tileTwo.frameIndex) << "pas " << step;
    }
}

TEST(AnimatedTilesTest, Bitmask16ExclutLAnimation) {
    EXPECT_TRUE(hmi::animationExcludedForTile(hmi::SkinMode::Bitmask16, core::TileType::Solid));
}

TEST(AnimatedTilesTest, SilhouetteDetoureeExclutLAnimation) {
    EXPECT_TRUE(hmi::animationExcludedForTile(hmi::SkinMode::Single, core::TileType::SlopeUpRight));
}

TEST(AnimatedTilesTest, SingleSansSilhouetteNExclutPasLAnimation) {
    EXPECT_FALSE(hmi::animationExcludedForTile(hmi::SkinMode::Single, core::TileType::Solid));
    EXPECT_FALSE(hmi::animationExcludedForTile(hmi::SkinMode::Single, core::TileType::Block));
}
