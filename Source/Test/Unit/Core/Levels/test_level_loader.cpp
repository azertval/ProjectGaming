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

/**
 * @brief Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et mécanismes.
 * \castest{<b>Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et
 * mécanismes.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et mécanismes.
 * }
 */
TEST(LevelLoaderTest, ChargeUnNiveauValide) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(VALID_LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.errorCode, core::LevelValidationError::None);

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

/**
 * @brief Une plaque de pression se charge comme un interrupteur : même règle d'identifiant, même
 * résolution de liaison vers une porte (`EX-GP-025`).
 * \castest{<b>Une plaque de pression se charge comme un interrupteur.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une plaque de pression se charge comme un interrupteur : même règle d'identifiant,
 * même résolution de liaison vers une porte.
 * }
 */
TEST(LevelLoaderTest, ChargeUnePlaqueDePression) {
    constexpr const char* LEVEL = R"({
      "name": "Poids",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "pressurePlate", "id": "p1" },
        { "x": 3, "y": 0, "type": "door", "opensWith": "p1" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::PressurePlate);
    ASSERT_EQ(level.mechanisms().size(), 1u);
    EXPECT_EQ(level.mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(level.mechanisms().front().doorPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Un bloc poussable se charge comme une simple tuile, sans identifiant ni liaison
 * (`EX-GP-022`).
 * \castest{<b>Un bloc poussable se charge comme une simple tuile, sans identifiant ni
 * liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc poussable se charge comme une simple tuile, sans identifiant ni liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeUnBlocPoussable) {
    constexpr const char* LEVEL = R"({
      "name": "Bloc",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "block" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::Block);
    EXPECT_TRUE(level.mechanisms().empty());  // un bloc n'est pas un mecanisme lie
}

/**
 * @brief Les deux orientations de pente se chargent comme de simples tuiles, sans identifiant ni
 * liaison (`EX-GP-003`).
 * \castest{<b>Les deux orientations de pente se chargent comme de simples tuiles, sans
 * identifiant ni liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux orientations de pente se chargent comme de simples tuiles, sans identifiant
 * ni liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeLesDeuxPentes) {
    constexpr const char* LEVEL = R"({
      "name": "Pentes",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "slopeUpRight" },
        { "x": 1, "y": 0, "type": "slopeUpLeft" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::SlopeUpRight);
    EXPECT_EQ(level.tileMap().tile(1, 0), core::TileType::SlopeUpLeft);
    EXPECT_TRUE(level.mechanisms().empty());
}

/**
 * @brief Les deux orientations d'arrondi se chargent comme de simples tuiles, sans identifiant ni
 * liaison (`EX-GP-004`).
 * \castest{<b>Les deux orientations d'arrondi se chargent comme de simples tuiles, sans
 * identifiant ni liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux orientations d'arrondi se chargent comme de simples tuiles, sans identifiant
 * ni liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeLesDeuxArrondis) {
    constexpr const char* LEVEL = R"({
      "name": "Arrondis",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "roundedUpRight" },
        { "x": 1, "y": 0, "type": "roundedUpLeft" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::RoundedUpRight);
    EXPECT_EQ(level.tileMap().tile(1, 0), core::TileType::RoundedUpLeft);
    EXPECT_TRUE(level.mechanisms().empty());
}

/**
 * @brief Les budgets de mouvements (EX-GP-024) sont chargés s'ils sont présents, illimités (-1)
 * sinon.
 * \castest{<b>Les budgets de mouvements (EX-GP-024) sont chargés s'ils sont présents, illimités
 * (-1) sinon.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les budgets de mouvements (EX-GP-024) sont chargés s'ils sont présents, illimités (-1)
 * sinon.
 * }
 */
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

/**
 * @brief Un JSON syntaxiquement invalide est rejeté sans plantage.
 * \castest{<b>Un JSON syntaxiquement invalide est rejeté sans plantage.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un JSON syntaxiquement invalide est rejeté sans plantage.
 * }
 */
TEST(LevelLoaderTest, JsonMalformeRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString("{ pas du json");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::ParseError);
}

/**
 * @brief Un champ obligatoire manquant est rejeté.
 * \castest{<b>Un champ obligatoire manquant est rejeté.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un champ obligatoire manquant est rejeté.
 * }
 */
TEST(LevelLoaderTest, ChampManquantRejete) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromString(R"({ "width": 4, "height": 3 })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::ParseError);
}

/**
 * @brief Un type de tuile inconnu est rejeté avec un message.
 * \castest{<b>Un type de tuile inconnu est rejeté avec un message.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un type de tuile inconnu est rejeté avec un message.
 * }
 */
TEST(LevelLoaderTest, TypeDeTuileInconnuRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 0, "y": 0, "type": "lave" } ] })");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnknownTileType);
}

/**
 * @brief Une tuile hors des bornes est rejetée.
 * \castest{<b>Une tuile hors des bornes est rejetée.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une tuile hors des bornes est rejetée.
 * }
 */
TEST(LevelLoaderTest, TuileHorsBornesRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 9, "y": 0, "type": "solid" } ] })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::OutOfBounds);
}

/**
 * @brief Une porte liée à un interrupteur inexistant est rejetée.
 * \castest{<b>Une porte liée à un interrupteur inexistant est rejetée.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une porte liée à un interrupteur inexistant est rejetée.
 * }
 */
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
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnresolvedMechanism);
}

/**
 * @brief Plusieurs entrées sont rejetées (une seule attendue).
 * \castest{<b>Plusieurs entrées sont rejetées (une seule attendue).</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Plusieurs entrées sont rejetées (une seule attendue).
 * }
 */
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
    EXPECT_EQ(result.errorCode, core::LevelValidationError::InvalidEntryCount);
}

/**
 * @brief Plusieurs sorties sont rejetées (une seule attendue).
 * \castest{<b>Plusieurs sorties sont rejetées (une seule attendue).</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Plusieurs sorties sont rejetées (une seule attendue).
 * }
 */
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
    EXPECT_EQ(result.errorCode, core::LevelValidationError::InvalidExitCount);
}

/**
 * @brief Deux tuiles à la même position sont rejetées.
 * \castest{<b>Deux tuiles à la même position sont rejetées.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Deux tuiles à la même position sont rejetées.
 * }
 */
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
    EXPECT_EQ(result.errorCode, core::LevelValidationError::DuplicatePosition);
}

/**
 * @brief Un champ 'tiles' qui n'est pas une liste est rejeté.
 * \castest{<b>Un champ 'tiles' qui n'est pas une liste est rejeté.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un champ 'tiles' qui n'est pas une liste est rejeté.
 * }
 */
TEST(LevelLoaderTest, TilesNonListeRejete) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromString(R"({ "width": 4, "height": 3, "tiles": 5 })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::ParseError);
}

/**
 * @brief Des dimensions non positives sont rejetées.
 * \castest{<b>Des dimensions non positives sont rejetées.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Des dimensions non positives sont rejetées.
 * }
 */
TEST(LevelLoaderTest, DimensionsNonPositivesRejetees) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromString(R"({ "width": 0, "height": 3, "tiles": [] })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::ParseError);
}

/**
 * @brief Un interrupteur sans 'id' est rejeté.
 * \castest{<b>Un interrupteur sans 'id' est rejeté.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un interrupteur sans 'id' est rejeté.
 * }
 */
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
    EXPECT_EQ(result.errorCode, core::LevelValidationError::MissingSwitchId);
}

/**
 * @brief Deux interrupteurs avec le même identifiant sont rejetés.
 * \castest{<b>Deux interrupteurs avec le même identifiant sont rejetés.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Deux interrupteurs avec le même identifiant sont rejetés.
 * }
 */
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
    EXPECT_EQ(result.errorCode, core::LevelValidationError::DuplicateSwitchId);
}

/**
 * @brief Un niveau sans entrée est rejeté.
 * \castest{<b>Un niveau sans entrée est rejeté.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans entrée est rejeté.
 * }
 */
TEST(LevelLoaderTest, NiveauSansEntreeRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 3, "y": 2, "type": "exit" } ] })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::InvalidEntryCount);
}

/**
 * @brief Un niveau sans sortie est rejeté.
 * \castest{<b>Un niveau sans sortie est rejeté.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans sortie est rejeté.
 * }
 */
TEST(LevelLoaderTest, NiveauSansSortieRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(
        R"({ "width": 4, "height": 3, "tiles": [ { "x": 1, "y": 1, "type": "entry" } ] })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::InvalidExitCount);
}

/**
 * @brief Charger un fichier inexistant échoue proprement (récupérable).
 * \castest{<b>Charger un fichier inexistant échoue proprement (récupérable).</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Charger un fichier inexistant échoue proprement (récupérable).
 * }
 */
TEST(LevelLoaderTest, FichierIntrouvableRejete) {
    const core::LevelLoadResult result =
        core::LevelLoader::loadFromFile("chemin/inexistant/pas_la.json");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::FileNotFound);
}

/**
 * @brief Une porte sans 'opensWith' est une simple tuile : chargement valide, aucun mécanisme.
 * \castest{<b>Une porte sans 'opensWith' est une simple tuile : chargement valide, aucun
 * mécanisme.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une porte sans 'opensWith' est une simple tuile : chargement valide, aucun mécanisme.
 * }
 */
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

/**
 * @brief Le niveau de démonstration livré (Source/Elements/Levels) se charge et se valide.
 * \castest{<b>Le niveau de démonstration livré (Source/Elements/Levels) se charge et se
 * valide.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau de démonstration livré (Source/Elements/Levels) se charge et se valide.
 * }
 */
TEST(LevelLoaderTest, NiveauDeDemoLivreValide) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo.json";
    const core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    EXPECT_TRUE(result.ok()) << result.error;
}
