/**
 * @file test_level_writer.cpp
 * @brief Tests unitaires de la sérialisation de niveau (round-trip avec LevelLoader, EX-EDIT-011).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileType.h"

namespace {

constexpr const char* LEVEL_WITH_MECHANISM = R"({
  "name": "Tutoriel",
  "width": 4,
  "height": 3,
  "jumpBudget": 2,
  "dashBudget": 1,
  "tiles": [
    { "x": 0, "y": 0, "type": "solid" },
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 2, "type": "exit" },
    { "x": 0, "y": 1, "type": "danger" },
    { "x": 2, "y": 0, "type": "switch", "id": "s1" },
    { "x": 3, "y": 0, "type": "door", "opensWith": "s1" }
  ]
})";

}  // namespace

/**
 * @brief Sérialiser puis recharger un niveau produit un niveau équivalent (round-trip).
 * \castest{<b>Sérialiser puis recharger un niveau produit un niveau équivalent
 * (round-trip).</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sérialiser puis recharger un niveau produit un niveau équivalent (round-trip).
 * }
 */
TEST(LevelWriterTest, RoundTripPreserveLeContenu) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_MECHANISM);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    const core::Level& original = *loaded.level;
    const core::Level& roundTripped = *reloaded.level;

    EXPECT_EQ(roundTripped.name(), original.name());
    EXPECT_EQ(roundTripped.tileMap().width(), original.tileMap().width());
    EXPECT_EQ(roundTripped.tileMap().height(), original.tileMap().height());
    for (int row = 0; row < original.tileMap().height(); ++row) {
        for (int column = 0; column < original.tileMap().width(); ++column) {
            EXPECT_EQ(roundTripped.tileMap().tile(column, row),
                      original.tileMap().tile(column, row))
                << "case (" << column << ", " << row << ")";
        }
    }
    EXPECT_EQ(roundTripped.entry(), original.entry());
    EXPECT_EQ(roundTripped.exit(), original.exit());
    EXPECT_EQ(roundTripped.jumpBudget(), original.jumpBudget());
    EXPECT_EQ(roundTripped.dashBudget(), original.dashBudget());

    ASSERT_EQ(roundTripped.mechanisms().size(), original.mechanisms().size());
    EXPECT_EQ(roundTripped.mechanisms().front().switchPosition,
              original.mechanisms().front().switchPosition);
    EXPECT_EQ(roundTripped.mechanisms().front().doorPosition,
              original.mechanisms().front().doorPosition);
}

/**
 * @brief Une plaque de pression survit au round-trip (sérialisation puis rechargement),
 * `TileType` et liaison préservés (`EX-GP-025`).
 * \castest{<b>Une plaque de pression survit au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une plaque de pression survit au round-trip : `TileType` et liaison préservés.
 * }
 */
TEST(LevelWriterTest, PlaqueDePressionSurvitAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::PressurePlate);
    ASSERT_EQ(reloaded.level->mechanisms().size(), 1u);
    EXPECT_EQ(reloaded.level->mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(reloaded.level->mechanisms().front().doorPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Un bloc poussable survit au round-trip (sérialisation puis rechargement), `TileType`
 * préservé (`EX-GP-022`).
 * \castest{<b>Un bloc poussable survit au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc poussable survit au round-trip : `TileType` préservé.
 * }
 */
TEST(LevelWriterTest, BlocPoussableSurvitAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::Block);
    EXPECT_TRUE(reloaded.level->mechanisms().empty());
}

/**
 * @brief Les deux blocs à taille réduite survivent à un aller-retour JSON (écriture puis
 * rechargement, `EX-GP-005`).
 * \castest{<b>Les deux blocs à taille réduite survivent à un aller-retour JSON.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux blocs à taille réduite survivent à un aller-retour JSON.
 * }
 */
TEST(LevelWriterTest, BlocsATailleReduiteSurviventAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(0, 0), core::TileType::BlockHalf);
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 0), core::TileType::BlockQuarter);
}

/**
 * @brief Les deux orientations de pente survivent au round-trip (`EX-GP-003`).
 * \castest{<b>Les deux orientations de pente survivent au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux orientations de pente survivent au round-trip.
 * }
 */
TEST(LevelWriterTest, PentesSurviventAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(0, 0), core::TileType::SlopeUpRight);
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 0), core::TileType::SlopeUpLeft);
}

/**
 * @brief Les deux orientations d'arrondi survivent à un aller-retour JSON (écriture puis
 * rechargement, `EX-GP-004`).
 * \castest{<b>Les deux orientations d'arrondi survivent à un aller-retour JSON.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux orientations d'arrondi survivent à un aller-retour JSON.
 * }
 */
TEST(LevelWriterTest, ArrondisSurviventAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(0, 0), core::TileType::RoundedUpRight);
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 0), core::TileType::RoundedUpLeft);
}

/**
 * @brief Les quatre pentes/arrondis de plafond survivent à un aller-retour JSON (`EX-GP-006`).
 * \castest{<b>Les quatre pentes/arrondis de plafond survivent à un aller-retour JSON.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les quatre pentes/arrondis de plafond survivent à un aller-retour JSON.
 * }
 */
TEST(LevelWriterTest, PentesEtArrondisDePlafondSurviventAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(0, 0), core::TileType::SlopeDownRight);
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 0), core::TileType::SlopeDownLeft);
    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::RoundedDownRight);
    EXPECT_EQ(reloaded.level->tileMap().tile(3, 0), core::TileType::RoundedDownLeft);
}

/**
 * @brief Les budgets illimités (-1) ne sont pas écrits dans le JSON produit.
 * \castest{<b>Les budgets illimités (-1) ne sont pas écrits dans le JSON produit.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les budgets illimités (-1) ne sont pas écrits dans le JSON produit.
 * }
 */
TEST(LevelWriterTest, BudgetsIllimitesOmisDuJson) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("jumpBudget"), std::string::npos);
    EXPECT_EQ(json.find("dashBudget"), std::string::npos);

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_EQ(reloaded.level->jumpBudget(), -1);
    EXPECT_EQ(reloaded.level->dashBudget(), -1);
}

/**
 * @brief Un interrupteur non relié à une porte est tout de même sérialisé, avec un identifiant.
 * \castest{<b>Un interrupteur non relié à une porte est tout de même sérialisé, avec un
 * identifiant.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un interrupteur non relié à une porte est tout de même sérialisé, avec un
 * identifiant.
 * }
 */
TEST(LevelWriterTest, InterrupteurNonRelieRegenereUnIdentifiant) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"},
                   {"x":1,"y":1,"type":"switch","id":"quelconque"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 1), core::TileType::Switch);
    EXPECT_TRUE(reloaded.level->mechanisms().empty());
}

/**
 * @brief Le niveau puzzle livré (demo-interrupteur.json) survit à un round-trip.
 * \castest{<b>Le niveau puzzle livré (demo-interrupteur.json) survit à un round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau puzzle livré (demo-interrupteur.json) survit à un round-trip.
 * }
 */
TEST(LevelWriterTest, NiveauPuzzleLivreSurvieAuRoundTrip) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo-interrupteur.json";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_EQ(reloaded.level->mechanisms().size(), loaded.level->mechanisms().size());
    EXPECT_EQ(reloaded.level->entry(), loaded.level->entry());
    EXPECT_EQ(reloaded.level->exit(), loaded.level->exit());
}

/**
 * @brief saveToFile écrit un fichier qui se recharge à l'identique (round-trip disque).
 * \castest{<b>saveToFile écrit un fichier qui se recharge à l'identique (round-trip
 * disque).</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu saveToFile écrit un fichier qui se recharge à l'identique (round-trip disque).
 * }
 */
TEST(LevelWriterTest, SaveToFileEcritUnFichierRechargeable) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_MECHANISM);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "projectgaming_test_save_to_file.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*loaded.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);

    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_EQ(reloaded.level->name(), loaded.level->name());
    EXPECT_EQ(reloaded.level->entry(), loaded.level->entry());
}

/**
 * @brief saveToFile vers un dossier inexistant échoue proprement (récupérable, EX-NFR-040).
 * \castest{<b>saveToFile vers un dossier inexistant échoue proprement (récupérable,
 * EX-NFR-040).</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu saveToFile vers un dossier inexistant échoue proprement (récupérable, EX-NFR-040).
 * }
 */
TEST(LevelWriterTest, SaveToFileVersDossierInexistantEchoueProprement) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_MECHANISM);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::filesystem::path path = "chemin/inexistant/pas_la/niveau.json";
    EXPECT_FALSE(core::LevelWriter::saveToFile(*loaded.level, path));
}
