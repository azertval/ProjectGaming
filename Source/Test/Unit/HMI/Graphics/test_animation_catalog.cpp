/**
 * @file test_animation_catalog.cpp
 * @brief Tests unitaires du format `nom-asset.anim.json` (`hmi::AnimationCatalog`, LOT-46
 *        TACHE-03) : lecture, validation, traduction en région de texture. Sans GPU ni Qt.
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/AnimationCatalog.h"

namespace {
constexpr const char* VALID_JSON = R"({
  "version": 1,
  "frameWidth": 16,
  "frameHeight": 16,
  "clips": {
    "closed":  { "frames": [0], "loop": true },
    "opening": { "frames": [1, 2, 3, 4], "frameDuration": 0.06, "loop": false, "next": "open" },
    "open":    { "frames": [5], "loop": true }
  }
})";
}  // namespace

TEST(AnimationCatalogTest, RoundTripClipsMultiplesDureeParDefautEtOneShot) {
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok()) << result.error;

    const hmi::AnimationDescription& description = *result.description;
    EXPECT_EQ(description.frameWidth, 16);
    EXPECT_EQ(description.frameHeight, 16);
    ASSERT_EQ(description.clips.clipCount(), 3);

    const core::AnimationClip& closed = description.clips.clipAt(description.clips.indexOf("closed"));
    EXPECT_EQ(closed.frames, (std::vector<int>{0}));
    EXPECT_EQ(closed.endMode, core::ClipEndMode::Loop);
    // "frameDuration" absent : durée par défaut documentée (AnimationCatalog::
    // DEFAULT_FRAME_DURATION_SECONDS).
    EXPECT_FLOAT_EQ(closed.frameDuration, hmi::AnimationCatalog::DEFAULT_FRAME_DURATION_SECONDS);

    const core::AnimationClip& opening =
        description.clips.clipAt(description.clips.indexOf("opening"));
    EXPECT_EQ(opening.frames, (std::vector<int>{1, 2, 3, 4}));
    EXPECT_FLOAT_EQ(opening.frameDuration, 0.06f);
    EXPECT_EQ(opening.endMode, core::ClipEndMode::OneShot);
    EXPECT_EQ(opening.nextClip, "open");
    EXPECT_GE(description.clips.indexOf(opening.nextClip), 0);
}

TEST(AnimationCatalogTest, JsonInvalideEstUneErreurExploitable) {
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString("{ pas du json");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::ParseError);
    EXPECT_FALSE(result.error.empty());
}

TEST(AnimationCatalogTest, VersionInconnueEstRefusee) {
    constexpr const char* json = R"({
      "version": 99,
      "frameWidth": 16, "frameHeight": 16,
      "clips": { "idle": { "frames": [0] } }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::UnsupportedVersion);
}

TEST(AnimationCatalogTest, ClipSuivantInexistantEstRefuse) {
    constexpr const char* json = R"({
      "version": 1,
      "frameWidth": 16, "frameHeight": 16,
      "clips": {
        "opening": { "frames": [0, 1], "loop": false, "next": "inconnu" }
      }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::MalformedStructure);
}

TEST(AnimationCatalogTest, IndiceDImageNegatifEstRefuse) {
    constexpr const char* json = R"({
      "version": 1,
      "frameWidth": 16, "frameHeight": 16,
      "clips": { "idle": { "frames": [-1] } }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::MalformedStructure);
}

TEST(AnimationCatalogTest, ClipSansFramesEstRefuse) {
    constexpr const char* json = R"({
      "version": 1,
      "frameWidth": 16, "frameHeight": 16,
      "clips": { "idle": { "loop": true } }
    })";
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::MalformedStructure);
}

TEST(AnimationCatalogTest, FichierAbsentEstFileNotFoundSansException) {
    const hmi::AnimationDescriptionResult result =
        hmi::AnimationCatalog::loadFromFile("Z:/chemin/totalement/inexistant.anim.json");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::AnimationCatalogError::FileNotFound);
}

TEST(AnimationCatalogTest, DescriptorFileNameRemplaceLExtension) {
    EXPECT_EQ(hmi::AnimationCatalog::descriptorFileName("water.png"), "water.anim.json");
    EXPECT_EQ(hmi::AnimationCatalog::descriptorFileName("Skins/lava.png"), "Skins/lava.anim.json");
}

TEST(AnimationCatalogTest, CoherenceAvecLePngValideeSurSpritesheetAUnRang) {
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok());

    // 6 images de 16 px sur un seul rang de 16 px de haut : coherent.
    const hmi::AssetValidation valid =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 96, 16);
    EXPECT_TRUE(valid.valid) << valid.message;
}

TEST(AnimationCatalogTest, TailleDImageIncoherenteAvecLePngEstRefusee) {
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok());

    // Hauteur du PNG differente de frameHeight : plus d'un rang, non supporte (TACHE-03).
    const hmi::AssetValidation wrongHeight =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 96, 32);
    EXPECT_FALSE(wrongHeight.valid);
    EXPECT_FALSE(wrongHeight.message.empty());

    // Largeur qui n'est pas un multiple de frameWidth.
    const hmi::AssetValidation wrongWidth =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 90, 16);
    EXPECT_FALSE(wrongWidth.valid);
}

TEST(AnimationCatalogTest, IndiceDImageHorsBornesDeLaSpritesheetReelleEstRefuse) {
    const hmi::AnimationDescriptionResult result = hmi::AnimationCatalog::loadFromString(VALID_JSON);
    ASSERT_TRUE(result.ok());

    // Le clip "open" reference l'image 5 : une spritesheet de 5 images (80 px / 16) est trop
    // courte, bien que sa largeur soit un multiple valide de frameWidth.
    const hmi::AssetValidation tooShort =
        hmi::AnimationCatalog::validateAgainstTexture(*result.description, "door.png", 80, 16);
    EXPECT_FALSE(tooShort.valid);
    EXPECT_FALSE(tooShort.message.empty());
}

TEST(AnimationCatalogTest, FrameRegionPremiereEtDerniereImageSpritesheetAUnRang) {
    hmi::AnimationDescription description;
    description.frameWidth = 16;
    description.frameHeight = 16;

    const core::AtlasRegion first = hmi::AnimationCatalog::frameRegion(description, 0);
    EXPECT_EQ(first.x, 0);
    EXPECT_EQ(first.y, 0);
    EXPECT_EQ(first.width, 16);
    EXPECT_EQ(first.height, 16);

    const core::AtlasRegion last = hmi::AnimationCatalog::frameRegion(description, 5);
    EXPECT_EQ(last.x, 5 * 16);
    EXPECT_EQ(last.y, 0);  // un seul rang : y ne varie jamais.
    EXPECT_EQ(last.width, 16);
    EXPECT_EQ(last.height, 16);
}
