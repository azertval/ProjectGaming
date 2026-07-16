/**
 * @file test_level_loader.cpp
 * @brief Tests unitaires du chargement de niveau (format JSON, erreurs récupérables).
 */

#include <gtest/gtest.h>

#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileType.h"

namespace {

// Un niveau JSON valide et minimal (entrée, sortie, un solide, une paire interrupteur/porte).
constexpr const char* VALID_LEVEL = R"({
  "name": "Tutoriel",
  "width": 4,
  "height": 3,
  "tiles": [
    { "x": 0, "y": 0, "type": "solid" },
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 2, "type": "exit" },
    { "x": 2, "y": 0, "type": "switch", "id": "s1" },
    { "x": 3, "y": 0, "type": "door", "opensWith": "s1" }
  ]
})";

}  // namespace

/// Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et mécanismes.
TEST(LevelLoaderTest, ChargeUnNiveauValide) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(VALID_LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.name(), "Tutoriel");
    EXPECT_EQ(level.tileMap().width(), 4);
    EXPECT_EQ(level.tileMap().height(), 3);
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::Solid);
    EXPECT_EQ(level.tileMap().tile(1, 1), core::TileType::Entry);
    EXPECT_EQ(level.entry(), (core::GridPosition{1, 1}));
    EXPECT_EQ(level.exit(), (core::GridPosition{3, 2}));

    ASSERT_EQ(level.mechanisms().size(), 1u);
    EXPECT_EQ(level.mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(level.mechanisms().front().doorPosition, (core::GridPosition{3, 0}));
}

/// Un JSON syntaxiquement invalide est rejeté sans plantage.
TEST(LevelLoaderTest, JsonMalformeRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString("{ pas du json");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}

/// Un champ obligatoire manquant est rejeté.
TEST(LevelLoaderTest, ChampManquantRejete) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromString(R"({ "width": 4, "height": 3 })");
    EXPECT_FALSE(result.ok());
}

/// Un type de tuile inconnu est rejeté avec un message.
TEST(LevelLoaderTest, TypeDeTuileInconnuRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 0, "y": 0, "type": "lave" } ] })");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}

/// Une tuile hors des bornes est rejetée.
TEST(LevelLoaderTest, TuileHorsBornesRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 9, "y": 0, "type": "solid" } ] })");
    EXPECT_FALSE(result.ok());
}

/// Une porte liée à un interrupteur inexistant est rejetée.
TEST(LevelLoaderTest, LiaisonMecanismeNonResolueRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 3, "y": 0, "type": "door", "opensWith": "inconnu" }
      ]
    })");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}
