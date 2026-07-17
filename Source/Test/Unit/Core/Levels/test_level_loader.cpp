/**
 * @file test_level_loader.cpp
 * @brief Tests unitaires du chargement de niveau (format JSON, erreurs récupérables).
 */

#include <filesystem>

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

/// Les budgets de mouvements (EX-GP-024) sont chargés s'ils sont présents, illimités (-1) sinon.
TEST(LevelLoaderTest, BudgetsOptionnels) {
    const core::LevelLoadResult withBudget = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3, "jumpBudget": 2, "dashBudget": 1,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(withBudget.ok()) << withBudget.error;
    EXPECT_EQ(withBudget.level->jumpBudget(), 2);
    EXPECT_EQ(withBudget.level->dashBudget(), 1);

    const core::LevelLoadResult noBudget = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(noBudget.ok()) << noBudget.error;
    EXPECT_EQ(noBudget.level->jumpBudget(), -1);  // illimité par défaut
    EXPECT_EQ(noBudget.level->dashBudget(), -1);
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

/// Plusieurs entrées sont rejetées (une seule attendue).
TEST(LevelLoaderTest, PlusieursEntreesRejetees) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 2, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })");
    EXPECT_FALSE(result.ok());
}

/// Plusieurs sorties sont rejetées (une seule attendue).
TEST(LevelLoaderTest, PlusieursSortiesRejetees) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 2, "y": 2, "type": "exit" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })");
    EXPECT_FALSE(result.ok());
}

/// Deux tuiles à la même position sont rejetées.
TEST(LevelLoaderTest, PositionEnDoubleRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "solid" },
        { "x": 0, "y": 0, "type": "danger" }
      ]
    })");
    EXPECT_FALSE(result.ok());
}

/// Un champ 'tiles' qui n'est pas une liste est rejeté.
TEST(LevelLoaderTest, TilesNonListeRejete) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromString(R"({ "width": 4, "height": 3, "tiles": 5 })");
    EXPECT_FALSE(result.ok());
}

/// Des dimensions non positives sont rejetées.
TEST(LevelLoaderTest, DimensionsNonPositivesRejetees) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromString(R"({ "width": 0, "height": 3, "tiles": [] })");
    EXPECT_FALSE(result.ok());
}

/// Un interrupteur sans 'id' est rejeté.
TEST(LevelLoaderTest, InterrupteurSansIdRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "switch" }
      ]
    })");
    EXPECT_FALSE(result.ok());
}

/// Deux interrupteurs avec le même identifiant sont rejetés.
TEST(LevelLoaderTest, IdentifiantInterrupteurEnDoubleRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "switch", "id": "s1" },
        { "x": 3, "y": 0, "type": "switch", "id": "s1" }
      ]
    })");
    EXPECT_FALSE(result.ok());
}

/// Un niveau sans entrée est rejeté.
TEST(LevelLoaderTest, NiveauSansEntreeRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 3, "y": 2, "type": "exit" } ] })");
    EXPECT_FALSE(result.ok());
}

/// Un niveau sans sortie est rejeté.
TEST(LevelLoaderTest, NiveauSansSortieRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 1, "y": 1, "type": "entry" } ] })");
    EXPECT_FALSE(result.ok());
}

/// Charger un fichier inexistant échoue proprement (récupérable).
TEST(LevelLoaderTest, FichierIntrouvableRejete) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromFile("chemin/inexistant/pas_la.json");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}

/// Une porte sans 'opensWith' est une simple tuile : chargement valide, aucun mécanisme.
TEST(LevelLoaderTest, PorteSansLiaisonEstValide) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 3, "y": 0, "type": "door" }
      ]
    })");
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_TRUE(result.level->mechanisms().empty());
}

/// Le niveau de démonstration livré (Source/Elements/Levels) se charge et se valide.
TEST(LevelLoaderTest, NiveauDeDemoLivreValide) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo.json";
    const core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    EXPECT_TRUE(result.ok()) << result.error;
}
