/**
 * @file test_level_loader.cpp
 * @brief Tests unitaires du chargement de niveau (format JSON, erreurs récupérables).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
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
 * @brief Un niveau sans champ `"version"` se charge sans erreur ni avertissement, comme la
 * version initiale du format (`EX-LVL-005`, rétrocompatibilité des niveaux antérieurs à ce
 * champ).
 * \castest{<b>Un niveau sans champ version se charge sans erreur.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans champ version se charge sans erreur.
 * }
 */
TEST(LevelLoaderTest, NiveauSansVersionSeChargeSansErreur) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(VALID_LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.errorCode, core::LevelValidationError::None);
}

/**
 * @brief Un niveau dont la version dépasse celle gérée échoue avec une erreur exploitable
 * (`EX-LVL-005`), pas une lecture au mieux.
 * \castest{<b>Un niveau dont la version depasse celle geree echoue proprement.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau dont la version depasse celle geree echoue proprement.
 * }
 */
TEST(LevelLoaderTest, VersionSuperieureALaVersionGereeEchoueProprement) {
    constexpr const char* LEVEL = R"({
      "version": 999,
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnsupportedFormatVersion);
}

/**
 * @brief Un niveau désignant un fond et un jeu de skins restitue les deux chaînes ; sans eux,
 * elles sont absentes (`EX-REN-044`, `EX-EDIT-024`).
 * \castest{<b>Un niveau designant un fond et un jeu de skins restitue les deux champs.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau designant un fond et un jeu de skins restitue les deux champs.
 * }
 */
TEST(LevelLoaderTest, ChargeLeFondEtLeJeuDeSkins) {
    constexpr const char* LEVEL = R"({
      "background": "forest.png",
      "skinSet": "foret",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    ASSERT_TRUE(result.level->background().has_value());
    EXPECT_EQ(*result.level->background(), "forest.png");
    ASSERT_TRUE(result.level->skinSet().has_value());
    EXPECT_EQ(*result.level->skinSet(), "foret");
}

/**
 * @brief Un niveau sans fond ni jeu de skins désignés n'en restitue aucun (état normal, pas une
 * anomalie, `EX-REN-044`).
 * \castest{<b>Un niveau sans fond ni jeu de skins n'en restitue aucun.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans fond ni jeu de skins n'en restitue aucun.
 * }
 */
TEST(LevelLoaderTest, SansFondNiJeuDeSkinsLesDeuxChampsSontAbsents) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(VALID_LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_FALSE(result.level->background().has_value());
    EXPECT_FALSE(result.level->skinSet().has_value());
}

/**
 * @brief Un niveau déclarant un mode de cadrage explicite le restitue tel quel, y compris une
 * taille de salle personnalisée pour le mode *par salle* (`EX-LVL-006`).
 * \castest{<b>Un mode de cadrage déclaré explicitement est restitué tel quel.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau déclarant le mode *par salle* avec une taille personnalisée.<br/>
 * 2. Vérifier le cadrage résolu.<br/>
 * \tattendu Le mode et la taille de salle déclarés sont restitués sans modification.
 * }
 */
TEST(LevelLoaderTest, ModeDeCadrageDeclareExplicitementEstRestitue) {
    constexpr const char* LEVEL = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 10, "roomHeightTiles": 8 },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.level->cameraFraming().mode, core::CameraFramingMode::PerRoom);
    ASSERT_TRUE(result.level->cameraFraming().roomWidthTiles.has_value());
    EXPECT_EQ(*result.level->cameraFraming().roomWidthTiles, 10);
    ASSERT_TRUE(result.level->cameraFraming().roomHeightTiles.has_value());
    EXPECT_EQ(*result.level->cameraFraming().roomHeightTiles, 8);
}

/**
 * @brief Un niveau sans champ `cameraFraming` reproduit exactement la règle historique : niveau
 * entier s'il tient dans une salle de taille par défaut, par salle sinon (`EX-LVL-006`, critère
 * d'acceptation numéro un du lot).
 * \castest{<b>L'absence de cameraFraming reproduit la règle historique.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un petit niveau (tient dans une salle) sans champ `cameraFraming`.<br/>
 * 2. Charger un grand niveau (dépasse une salle) sans champ `cameraFraming`.<br/>
 * \tattendu Le petit niveau résout en mode *niveau entier* ; le grand résout en mode *par salle*,
 * avec une taille de salle par défaut (absente, pas explicitement 24×14).
 * }
 */
TEST(LevelLoaderTest, AbsenceDeCameraFramingReproduitLaRegleHistorique) {
    constexpr const char* SMALL_LEVEL = R"({
      "width": 10, "height": 8,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult small = core::LevelLoader::loadFromString(SMALL_LEVEL);
    ASSERT_TRUE(small.ok()) << small.error;
    EXPECT_EQ(small.level->cameraFraming().mode, core::CameraFramingMode::WholeLevel);

    constexpr const char* LARGE_LEVEL = R"({
      "width": 30, "height": 20,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult large = core::LevelLoader::loadFromString(LARGE_LEVEL);
    ASSERT_TRUE(large.ok()) << large.error;
    EXPECT_EQ(large.level->cameraFraming().mode, core::CameraFramingMode::PerRoom);
    EXPECT_FALSE(large.level->cameraFraming().roomWidthTiles.has_value());
    EXPECT_FALSE(large.level->cameraFraming().roomHeightTiles.has_value());
}

/**
 * @brief Un cadrage invalide (mode inconnu, taille de salle nulle ou supérieure au niveau,
 * paramètre étranger au mode) échoue avec une erreur nommant le champ fautif (`EX-LVL-004`).
 * \castest{<b>Un cadrage invalide échoue avec une erreur exploitable.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger successivement un niveau par variante d'erreur (mode inconnu, taille
 * nulle, taille supérieure au niveau, paramètre étranger au mode).<br/>
 * \tattendu Chaque variante échoue avec `LevelValidationError::InvalidCameraFraming`.
 * }
 */
TEST(LevelLoaderTest, CadrageInvalideEchoueAvecErreurExploitable) {
    constexpr const char* UNKNOWN_MODE = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "zoomInfini" },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    constexpr const char* ZERO_ROOM_WIDTH = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 0 },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    constexpr const char* ROOM_WIDTH_EXCEEDS_LEVEL = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 11 },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    constexpr const char* FOREIGN_PARAMETER = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "wholeLevel", "roomWidthTiles": 5 },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";

    for (const char* levelJson :
         {UNKNOWN_MODE, ZERO_ROOM_WIDTH, ROOM_WIDTH_EXCEEDS_LEVEL, FOREIGN_PARAMETER}) {
        const core::LevelLoadResult result = core::LevelLoader::loadFromString(levelJson);
        EXPECT_FALSE(result.ok());
        EXPECT_EQ(result.errorCode, core::LevelValidationError::InvalidCameraFraming);
    }
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
 * @brief Les deux blocs à taille réduite se chargent comme de simples tuiles, sans identifiant ni
 * liaison (`EX-GP-005`).
 * \castest{<b>Les deux blocs à taille réduite se chargent comme de simples tuiles, sans
 * identifiant ni liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux blocs à taille réduite se chargent comme de simples tuiles, sans identifiant
 * ni liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeLesBlocsATailleReduite) {
    constexpr const char* LEVEL = R"({
      "name": "BlocsReduits",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "blockHalf" },
        { "x": 1, "y": 0, "type": "blockQuarter" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::BlockHalf);
    EXPECT_EQ(level.tileMap().tile(1, 0), core::TileType::BlockQuarter);
    EXPECT_TRUE(level.mechanisms().empty());
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
 * @brief Les quatre pentes/arrondis de plafond se chargent comme de simples tuiles, sans
 * identifiant ni liaison (`EX-GP-006`).
 * \castest{<b>Les quatre pentes/arrondis de plafond se chargent comme de simples tuiles, sans
 * identifiant ni liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les quatre pentes/arrondis de plafond se chargent comme de simples tuiles, sans
 * identifiant ni liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeLesQuatrePentesEtArrondisDePlafond) {
    constexpr const char* LEVEL = R"({
      "name": "Plafond",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "slopeDownRight" },
        { "x": 1, "y": 0, "type": "slopeDownLeft" },
        { "x": 2, "y": 0, "type": "roundedDownRight" },
        { "x": 3, "y": 0, "type": "roundedDownLeft" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::SlopeDownRight);
    EXPECT_EQ(level.tileMap().tile(1, 0), core::TileType::SlopeDownLeft);
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::RoundedDownRight);
    EXPECT_EQ(level.tileMap().tile(3, 0), core::TileType::RoundedDownLeft);
    EXPECT_TRUE(level.mechanisms().empty());
}

/**
 * @brief Les quatre arrondis concaves (sol et plafond) se chargent comme de simples tuiles, sans
 * identifiant ni liaison (`EX-GP-007`).
 * \castest{<b>Les quatre arrondis concaves se chargent comme de simples tuiles, sans identifiant ni
 * liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les quatre arrondis concaves se chargent comme de simples tuiles, sans identifiant ni
 * liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeLesQuatreArrondisConcaves) {
    constexpr const char* LEVEL = R"({
      "name": "Concave",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "concaveUpRight" },
        { "x": 1, "y": 0, "type": "concaveUpLeft" },
        { "x": 2, "y": 0, "type": "concaveDownRight" },
        { "x": 3, "y": 0, "type": "concaveDownLeft" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::ConcaveUpRight);
    EXPECT_EQ(level.tileMap().tile(1, 0), core::TileType::ConcaveUpLeft);
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::ConcaveDownRight);
    EXPECT_EQ(level.tileMap().tile(3, 0), core::TileType::ConcaveDownLeft);
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
 * @brief Une clé liée à une porte verrouillée se charge et résout la liaison dans `mechanisms()`,
 *        à égalité avec interrupteur↔porte (`EX-GP-023`).
 * \castest{<b>Une clé liée à une porte verrouillée se charge et résout la liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La clé et la porte verrouillée se chargent, la liaison apparaît dans `mechanisms()`.
 * }
 */
TEST(LevelLoaderTest, CleEtPorteVerrouilleeSeChargeEtSeResout) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "key", "id": "k1" },
        { "x": 3, "y": 0, "type": "lockedDoor", "opensWith": "k1" }
      ]
    })");
    ASSERT_TRUE(result.ok()) << result.error;
    ASSERT_EQ(result.level->mechanisms().size(), 1U);
    EXPECT_EQ(result.level->mechanisms()[0].switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(result.level->mechanisms()[0].doorPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Deux paires clé/porte verrouillée indépendantes dans le même tableau ne s'influencent
 *        pas l'une l'autre.
 * \castest{<b>Deux paires clé/porte verrouillée indépendantes coexistent sans
 * interférence.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux liaisons sont résolues chacune vers leur propre porte.
 * }
 */
TEST(LevelLoaderTest, DeuxPairesCleEtPorteIndependantes) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 6, "height": 3,
      "tiles": [
        { "x": 0, "y": 1, "type": "entry" },
        { "x": 5, "y": 2, "type": "exit" },
        { "x": 1, "y": 0, "type": "key", "id": "k1" },
        { "x": 2, "y": 0, "type": "lockedDoor", "opensWith": "k1" },
        { "x": 3, "y": 0, "type": "key", "id": "k2" },
        { "x": 4, "y": 0, "type": "lockedDoor", "opensWith": "k2" }
      ]
    })");
    ASSERT_TRUE(result.ok()) << result.error;
    ASSERT_EQ(result.level->mechanisms().size(), 2U);
}

/**
 * @brief Une clé sans identifiant est rejetée, comme un interrupteur sans identifiant.
 * \castest{<b>Une clé sans 'id' est rejetée.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une clé sans 'id' est rejetée.
 * }
 */
TEST(LevelLoaderTest, CleSansIdRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "key" }
      ]
    })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::MissingSwitchId);
}

/**
 * @brief Deux clés avec le même identifiant sont rejetées.
 * \castest{<b>Deux clés avec le même identifiant sont rejetées.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Deux clés avec le même identifiant sont rejetées.
 * }
 */
TEST(LevelLoaderTest, IdentifiantCleEnDoubleRejete) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "key", "id": "k1" },
        { "x": 3, "y": 0, "type": "key", "id": "k1" }
      ]
    })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::DuplicateSwitchId);
}

/**
 * @brief Une porte verrouillée sans 'opensWith' est rejetée — contrairement à une porte classique,
 *        le lien est obligatoire (`EX-GP-023`).
 * \castest{<b>Une porte verrouillée sans clé liée est rejetée.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une porte verrouillée sans 'opensWith' est rejetée.
 * }
 */
TEST(LevelLoaderTest, PorteVerrouilleeSansCleRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 3, "y": 0, "type": "lockedDoor" }
      ]
    })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnresolvedMechanism);
}

/**
 * @brief Une porte verrouillée liée à une clé inexistante est rejetée.
 * \castest{<b>Une porte verrouillée liée à une clé inexistante est rejetée.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une porte verrouillée liée à une clé inexistante est rejetée.
 * }
 */
TEST(LevelLoaderTest, PorteVerrouilleeLieeAUneCleInexistanteRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 3, "y": 0, "type": "lockedDoor", "opensWith": "inconnue" }
      ]
    })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnresolvedMechanism);
}

/**
 * @brief Une clé sans porte verrouillée liée est rejetée — contrairement à un interrupteur, qui
 *        peut rester sans porte.
 * \castest{<b>Une clé sans porte verrouillée liée est rejetée.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une clé sans porte verrouillée liée est rejetée.
 * }
 */
TEST(LevelLoaderTest, CleSansPorteVerrouilleeLieeRejetee) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "key", "id": "k1" }
      ]
    })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnresolvedMechanism);
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
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo-deplacement.json";
    const core::LevelLoadResult result = core::LevelLoader::loadFromFile(path);
    EXPECT_TRUE(result.ok()) << result.error;
}

/**
 * @brief Les quatre dangers directionnels se chargent comme de simples tuiles, sans identifiant ni
 * liaison (`EX-GP-050`).
 * \castest{<b>Les quatre dangers directionnels se chargent comme de simples tuiles, sans
 * identifiant ni liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les quatre dangers directionnels se chargent comme de simples tuiles, sans identifiant
 * ni liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeLesQuatreDangersDirectionnels) {
    constexpr const char* LEVEL = R"({
      "name": "DangersDirectionnels",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "dangerUp" },
        { "x": 1, "y": 0, "type": "dangerDown" },
        { "x": 2, "y": 0, "type": "dangerLeft" },
        { "x": 3, "y": 0, "type": "dangerRight" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::DangerUp);
    EXPECT_EQ(level.tileMap().tile(1, 0), core::TileType::DangerDown);
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::DangerLeft);
    EXPECT_EQ(level.tileMap().tile(3, 0), core::TileType::DangerRight);
    EXPECT_TRUE(level.mechanisms().empty());
}

/**
 * @brief Un danger mobile sans champs explicites reçoit les valeurs de conception par défaut
 * (`EX-GP-051`).
 * \castest{<b>Un danger mobile sans champs explicites reçoit les valeurs par défaut.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger mobile sans champs explicites reçoit les valeurs de conception par défaut.
 * }
 */
TEST(LevelLoaderTest, ChargeUnDangerMobileValeursParDefaut) {
    constexpr const char* LEVEL = R"({
      "width": 5, "height": 3,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 4, "y": 2, "type": "exit" },
        { "x": 1, "y": 1, "type": "dangerMover" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(1, 1), core::TileType::DangerMover);
    ASSERT_EQ(level.moverConfigs().size(), 1u);
    EXPECT_EQ(level.moverConfigs().front().startPosition, (core::GridPosition{1, 1}));
    EXPECT_EQ(level.moverConfigs().front().axis, core::DangerMoverAxis::Horizontal);
    EXPECT_EQ(level.moverConfigs().front().range, 2);
}

/**
 * @brief Un danger mobile avec axe et portée explicites les préserve (`EX-GP-051`).
 * \castest{<b>Un danger mobile avec axe et portée explicites les préserve.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger mobile avec axe et portée explicites les préserve.
 * }
 */
TEST(LevelLoaderTest, ChargeUnDangerMobileValeursExplicites) {
    constexpr const char* LEVEL = R"({
      "width": 5, "height": 6,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 4, "y": 5, "type": "exit" },
        { "x": 1, "y": 1, "type": "dangerMover", "axis": "vertical", "range": 3 }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    ASSERT_EQ(result.level->moverConfigs().size(), 1u);
    EXPECT_EQ(result.level->moverConfigs().front().axis, core::DangerMoverAxis::Vertical);
    EXPECT_EQ(result.level->moverConfigs().front().range, 3);
}

/**
 * @brief Une portée de danger mobile qui sortirait de la grille est rejetée (`EX-LVL-004`).
 * \castest{<b>Une portée de danger mobile qui sortirait de la grille est rejetée.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une portée de danger mobile qui sortirait de la grille est rejetée.
 * }
 */
TEST(LevelLoaderTest, PorteeDangerMobileHorsBornesRejetee) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 3, "y": 1, "type": "dangerMover", "axis": "horizontal", "range": 2 }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::OutOfBounds);
}

/**
 * @brief Un danger commuté lié à un interrupteur est mortel selon l'état de celui-ci, résolu au
 * chargement comme une porte (`EX-GP-052`).
 * \castest{<b>Un danger commuté lié à un interrupteur est résolu au chargement.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger commuté lié à un interrupteur est résolu au chargement.
 * }
 */
TEST(LevelLoaderTest, ChargeUnDangerCommuteLie) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "switch", "id": "s1" },
        { "x": 3, "y": 0, "type": "dangerSwitched", "opensWith": "s1" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(3, 0), core::TileType::DangerSwitched);
    EXPECT_TRUE(level.mechanisms().empty());  // pas une porte : pas un Mechanism classique
    ASSERT_EQ(level.dangerLinks().size(), 1u);
    EXPECT_EQ(level.dangerLinks().front().triggerPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(level.dangerLinks().front().dangerPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Un danger commuté sans 'opensWith' est une simple tuile inerte : chargement valide,
 * aucune liaison (`EX-GP-052`).
 * \castest{<b>Un danger commuté sans 'opensWith' est une simple tuile inerte.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger commuté sans 'opensWith' est une simple tuile inerte : chargement valide,
 * aucune liaison.
 * }
 */
TEST(LevelLoaderTest, DangerCommuteSansLiaisonEstValide) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 3, "y": 0, "type": "dangerSwitched" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_TRUE(result.level->dangerLinks().empty());
}

/**
 * @brief Un danger commuté lié à un interrupteur inexistant est rejeté (`EX-GP-052`).
 * \castest{<b>Un danger commuté lié à un interrupteur inexistant est rejeté.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger commuté lié à un interrupteur inexistant est rejeté.
 * }
 */
TEST(LevelLoaderTest, DangerCommuteLieAInterrupteurInexistantRejete) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 3, "y": 0, "type": "dangerSwitched", "opensWith": "inconnu" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnresolvedMechanism);
}

/**
 * @brief Un danger temporisé sans champs explicites reçoit les valeurs de conception par défaut
 * (`EX-GP-053`).
 * \castest{<b>Un danger temporisé sans champs explicites reçoit les valeurs par défaut.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger temporisé sans champs explicites reçoit les valeurs de conception par
 * défaut.
 * }
 */
TEST(LevelLoaderTest, ChargeUnDangerTemporiseValeursParDefaut) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "dangerBlink" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    ASSERT_EQ(result.level->blinkConfigs().size(), 1u);
    const core::DangerBlinkConfig& config = result.level->blinkConfigs().front();
    EXPECT_EQ(config.position, (core::GridPosition{2, 0}));
    EXPECT_EQ(config.period, 120);
    EXPECT_EQ(config.phase, 0);
    EXPECT_EQ(config.activeDuration, 60);
}

/**
 * @brief Un danger temporisé avec période/déphasage/durée active explicites les préserve
 * (`EX-GP-053`).
 * \castest{<b>Un danger temporisé avec valeurs explicites les préserve.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger temporisé avec période/déphasage/durée active explicites les préserve.
 * }
 */
TEST(LevelLoaderTest, ChargeUnDangerTemporiseValeursExplicites) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "dangerBlink", "period": 90, "phase": 15,
          "activeDuration": 30 }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    ASSERT_EQ(result.level->blinkConfigs().size(), 1u);
    const core::DangerBlinkConfig& config = result.level->blinkConfigs().front();
    EXPECT_EQ(config.period, 90);
    EXPECT_EQ(config.phase, 15);
    EXPECT_EQ(config.activeDuration, 30);
}

/**
 * @brief Les dix-sept niveaux de démonstration livrés, aucun ne portant de champ `"version"`, se
 * chargent tous sans erreur ni avertissement (`EX-LVL-005`, rétrocompatibilité, critère
 * d'acceptation du LOT-44).
 * \castest{<b>Les dix-sept niveaux de demonstration livres se chargent sans erreur.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Balayer le dossier des niveaux livres.<br/>2. Charger chaque fichier.<br/>
 * \tattendu Chaque niveau se charge sans erreur, avec errorCode == None.
 * }
 */
TEST(LevelLoaderTest, LesDixSeptNiveauxDeDemoSeChargentSansErreur) {
    const std::filesystem::path levelsDir(PROJECTGAMING_LEVELS_DIR);
    int checked = 0;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(levelsDir)) {
        // Filtre sur le prefixe "demo-" (meme convention que scripts/check_demo_sequence.py),
        // pas "tout .json du dossier" : celui-ci contient aussi sequence-demo.json (LOT-59
        // TACHE-04, EX-LVL-013), qui n'est pas un niveau et ne se chargerait pas comme tel.
        if (entry.path().extension() != ".json" ||
            entry.path().filename().string().rfind("demo-", 0) != 0) {
            continue;
        }
        const core::LevelLoadResult result = core::LevelLoader::loadFromFile(entry.path());
        EXPECT_TRUE(result.ok()) << entry.path().filename().string() << " : " << result.error;
        EXPECT_EQ(result.errorCode, core::LevelValidationError::None)
            << entry.path().filename().string();
        ++checked;
    }
    EXPECT_EQ(checked, 17);
}
