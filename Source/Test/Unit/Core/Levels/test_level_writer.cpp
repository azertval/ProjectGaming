// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_writer.cpp
 * @brief Tests unitaires de la sérialisation de niveau (round-trip avec LevelLoader, EX-EDIT-011).
 */

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
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
 * @brief Une clé et une porte verrouillée survivent au round-trip (sérialisation puis
 *        rechargement), `TileType` et liaison préservés (`EX-GP-023`).
 * \castest{<b>Une clé et une porte verrouillée survivent au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La clé et la porte verrouillée survivent au round-trip : `TileType` et liaison
 * préservés.
 * }
 */
TEST(LevelWriterTest, CleEtPorteVerrouilleeSurvitAuRoundTrip) {
    constexpr const char* LEVEL = R"({
      "name": "Cle",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "key", "id": "k1" },
        { "x": 3, "y": 0, "type": "lockedDoor", "opensWith": "k1" }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::Key);
    EXPECT_EQ(reloaded.level->tileMap().tile(3, 0), core::TileType::LockedDoor);
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
 * @brief Les quatre arrondis concaves (sol et plafond) survivent à un aller-retour JSON
 * (`EX-GP-007`).
 * \castest{<b>Les quatre arrondis concaves survivent à un aller-retour JSON.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les quatre arrondis concaves survivent à un aller-retour JSON.
 * }
 */
TEST(LevelWriterTest, ArrondisConcavesSurviventAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(0, 0), core::TileType::ConcaveUpRight);
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 0), core::TileType::ConcaveUpLeft);
    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::ConcaveDownRight);
    EXPECT_EQ(reloaded.level->tileMap().tile(3, 0), core::TileType::ConcaveDownLeft);
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
 * @brief Le fond et le jeu de skins survivent au round-trip (sérialisation puis rechargement,
 * `EX-REN-044`, `EX-EDIT-024`).
 * \castest{<b>Le fond et le jeu de skins survivent au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le fond et le jeu de skins survivent au round-trip.
 * }
 */
TEST(LevelWriterTest, FondEtJeuDeSkinsSurviventAuRoundTrip) {
    constexpr const char* LEVEL = R"({
      "background": "forest.png",
      "skinSet": "foret",
      "width": 3, "height": 3,
      "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    ASSERT_TRUE(reloaded.level->background().has_value());
    EXPECT_EQ(*reloaded.level->background(), "forest.png");
    ASSERT_TRUE(reloaded.level->skinSet().has_value());
    EXPECT_EQ(*reloaded.level->skinSet(), "foret");
}

/**
 * @brief Un niveau sans fond ni jeu de skins n'écrit ni l'une ni l'autre clé dans le JSON produit
 * (`EX-REN-044`, `EX-EDIT-024`) ; le champ `"version"`, lui, est toujours écrit.
 * \castest{<b>Sans fond ni jeu de skins, les cles sont absentes du JSON produit.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans fond ni jeu de skins, les cles sont absentes du JSON produit.
 * }
 */
TEST(LevelWriterTest, SansFondNiJeuDeSkinsLesClesSontOmisesDuJson) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("\"background\""), std::string::npos);
    EXPECT_EQ(json.find("\"skinSet\""), std::string::npos);
    EXPECT_NE(json.find("\"version\""), std::string::npos);
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

/**
 * @brief Les quatre dangers directionnels survivent à un aller-retour JSON (`EX-GP-050`).
 * \castest{<b>Les quatre dangers directionnels survivent à un aller-retour JSON.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les quatre dangers directionnels survivent à un aller-retour JSON.
 * }
 */
TEST(LevelWriterTest, DangersDirectionnelsSurviventAuRoundTrip) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 0, "y": 0, "type": "dangerUp" },
        { "x": 1, "y": 0, "type": "dangerDown" },
        { "x": 2, "y": 0, "type": "dangerLeft" },
        { "x": 3, "y": 0, "type": "dangerRight" }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(0, 0), core::TileType::DangerUp);
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 0), core::TileType::DangerDown);
    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::DangerLeft);
    EXPECT_EQ(reloaded.level->tileMap().tile(3, 0), core::TileType::DangerRight);
}

/**
 * @brief Un danger mobile survit au round-trip avec son axe et sa portée (`EX-GP-051`).
 * \castest{<b>Un danger mobile survit au round-trip avec sa configuration.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger mobile survit au round-trip avec son axe et sa portée.
 * }
 */
TEST(LevelWriterTest, DangerMobileSurvitAuRoundTripAvecSaConfiguration) {
    constexpr const char* LEVEL = R"({
      "width": 5, "height": 6,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 4, "y": 5, "type": "exit" },
        { "x": 1, "y": 1, "type": "dangerMover", "axis": "vertical", "range": 3 }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(1, 1), core::TileType::DangerMover);
    ASSERT_EQ(reloaded.level->moverConfigs().size(), 1u);
    EXPECT_EQ(reloaded.level->moverConfigs().front().axis, core::DangerMoverAxis::Vertical);
    EXPECT_EQ(reloaded.level->moverConfigs().front().range, 3);
}

/**
 * @brief Un danger commuté survit au round-trip avec sa liaison à l'interrupteur (`EX-GP-052`).
 * \castest{<b>Un danger commuté survit au round-trip avec sa liaison.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger commuté survit au round-trip avec sa liaison à l'interrupteur.
 * }
 */
TEST(LevelWriterTest, DangerCommuteSurvitAuRoundTrip) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "switch", "id": "s1" },
        { "x": 3, "y": 0, "type": "dangerSwitched", "opensWith": "s1" }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(3, 0), core::TileType::DangerSwitched);
    ASSERT_EQ(reloaded.level->dangerLinks().size(), 1u);
    EXPECT_EQ(reloaded.level->dangerLinks().front().triggerPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(reloaded.level->dangerLinks().front().dangerPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Un danger temporisé survit au round-trip avec sa période, son déphasage et sa durée
 * active (`EX-GP-053`).
 * \castest{<b>Un danger temporisé survit au round-trip avec sa configuration.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger temporisé survit au round-trip avec sa période, son déphasage et sa durée
 * active.
 * }
 */
TEST(LevelWriterTest, DangerTemporiseSurvitAuRoundTripAvecSaConfiguration) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "dangerBlink", "period": 90, "phase": 15,
          "activeDuration": 30 }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    ASSERT_EQ(reloaded.level->blinkConfigs().size(), 1u);
    const core::DangerBlinkConfig& config = reloaded.level->blinkConfigs().front();
    EXPECT_EQ(config.period, 90);
    EXPECT_EQ(config.phase, 15);
    EXPECT_EQ(config.activeDuration, 30);
}

/**
 * @brief Chacun des trois modes de cadrage, avec une taille de salle personnalisée pour le mode
 * *par salle*, survit à un aller-retour JSON à l'identique (`EX-LVL-006`).
 * \castest{<b>Chacun des trois modes de cadrage survit au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau déclarant chacun des trois modes de cadrage.<br/>2. Sérialiser
 * puis recharger.<br/>
 * \tattendu Le cadrage résolu du niveau rechargé est identique à celui du niveau d'origine,
 * y compris la taille de salle personnalisée du mode *par salle*.
 * }
 */
TEST(LevelWriterTest, ChacunDesTroisModesDeCadrageSurvitAuRoundTrip) {
    constexpr const char* WHOLE_LEVEL = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "wholeLevel" },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    constexpr const char* PER_ROOM = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 10, "roomHeightTiles": 8 },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    // La taille de vue du mode suivi (EX-REN-017, memes champs que la taille de salle du mode par
    // salle) doit elle aussi survivre au round-trip.
    constexpr const char* FOLLOW = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "follow", "roomWidthTiles": 8, "roomHeightTiles": 6 },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";

    for (const char* levelJson : {WHOLE_LEVEL, PER_ROOM, FOLLOW}) {
        const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(levelJson);
        ASSERT_TRUE(loaded.ok()) << loaded.error;

        const std::string json = core::LevelWriter::toJsonString(*loaded.level);
        const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
        ASSERT_TRUE(reloaded.ok()) << reloaded.error;

        EXPECT_EQ(reloaded.level->cameraFraming().mode, loaded.level->cameraFraming().mode);
        EXPECT_EQ(reloaded.level->cameraFraming().roomWidthTiles,
                  loaded.level->cameraFraming().roomWidthTiles);
        EXPECT_EQ(reloaded.level->cameraFraming().roomHeightTiles,
                  loaded.level->cameraFraming().roomHeightTiles);
    }
}

/**
 * @brief Des zones de caméra dessinées à la main survivent à un aller-retour JSON, dans le même
 * ordre (`EX-LVL-007`).
 * \castest{<b>Les zones de caméra dessinées à la main survivent au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau déclarant deux zones de caméra.<br/>2. Sérialiser puis
 * recharger.<br/>
 * \tattendu Les zones rechargées sont identiques, dans le même ordre.
 * }
 */
TEST(LevelWriterTest, ZonesDeCameraSurviventAuRoundTrip) {
    constexpr const char* LEVEL = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "perRoom", "zones": [
        { "x": 0, "y": 0, "width": 24, "height": 14 },
        { "x": 24, "y": 0, "width": 6, "height": 20 }
      ] },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->cameraFraming().zones, loaded.level->cameraFraming().zones);
}

/**
 * @brief Un niveau dont le cadrage résolu coïncide avec la règle de repli est réécrit **sans** le
 * champ `"cameraFraming"` : aucune régression de round-trip pour les niveaux jamais retouchés sur
 * ce point (`EX-LVL-006`).
 * \castest{<b>Un cadrage identique au repli reste omis du JSON écrit.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un petit niveau sans champ `cameraFraming` (repli : niveau entier).<br/>
 * 2. Sérialiser.<br/>
 * \tattendu Le JSON produit ne contient pas le champ `"cameraFraming"`.
 * }
 */
TEST(LevelWriterTest, CadrageIdentiqueAuRepliResteOmisDuJson) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("cameraFraming"), std::string::npos);
}

namespace {

// Un niveau 6x6 dont l'unique plateforme mobile porte les champs de route donnes.
std::string levelWithPlatform(const std::string& platformFields) {
    return R"({
      "name": "N", "width": 6, "height": 6,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 5, "y": 5, "type": "exit" },
        { "x": 1, "y": 1, "type": "movingPlatform")" +
           platformFields + R"( }
      ]
    })";
}

}  // namespace

/**
 * @brief La route et le mode d'une plateforme survivent à un aller-retour écriture/relecture
 *        (`EX-GP-054`, `EX-EDIT-011`).
 * \castest{<b>La route et le mode d'une plateforme survivent à un aller-retour.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau dont la plateforme suit une route a deux points en mode
 * boucle.<br/>2. Serialiser puis recharger.<br/>
 * \tattendu La route, le mode, la vitesse et le dephasage sont identiques a l'original.
 * }
 */
TEST(LevelWriterTest, RouteEtModeDePlateformeSurviventAuRoundTrip) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(levelWithPlatform(
        R"(, "waypoints": [ {"x": 4, "y": 1}, {"x": 4, "y": 3} ], "mode": "loop", "speed": 1.5, "phase": 7)"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const core::LevelLoadResult reloaded =
        core::LevelLoader::loadFromString(core::LevelWriter::toJsonString(*loaded.level));
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    ASSERT_EQ(reloaded.level->platformConfigs().size(), 1u);

    const core::MovingPlatformConfig& config = reloaded.level->platformConfigs().front();
    EXPECT_EQ(config.startPosition, (core::GridPosition{1, 1}));
    EXPECT_EQ(config.waypoints, (std::vector<core::GridPosition>{core::GridPosition{4, 1},
                                                                 core::GridPosition{4, 3}}));
    EXPECT_EQ(config.mode, core::PlatformPathMode::Loop);
    EXPECT_FLOAT_EQ(config.speed, 1.5f);
    EXPECT_EQ(config.phase, 7);
}

/**
 * @brief Le mode aller-retour, valeur par défaut, reste omis du JSON écrit — même convention
 *        « omis si défaut » que les autres champs du format (`EX-LVL-008`).
 * \castest{<b>Le mode par défaut reste omis du JSON écrit.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Charger une plateforme en mode aller-retour, puis une en mode boucle.<br/>2.
 * Serialiser les deux.<br/>
 * \tattendu Le champ "mode" n'apparait que pour le circuit ferme.
 * }
 */
TEST(LevelWriterTest, ModeParDefautResteOmisDuJson) {
    const core::LevelLoadResult pingPong = core::LevelLoader::loadFromString(
        levelWithPlatform(R"(, "waypoints": [ {"x": 4, "y": 1} ])"));
    ASSERT_TRUE(pingPong.ok()) << pingPong.error;
    EXPECT_EQ(core::LevelWriter::toJsonString(*pingPong.level).find("\"mode\""), std::string::npos);

    const core::LevelLoadResult loop = core::LevelLoader::loadFromString(
        levelWithPlatform(R"(, "waypoints": [ {"x": 4, "y": 1} ], "mode": "loop")"));
    ASSERT_TRUE(loop.ok()) << loop.error;
    EXPECT_NE(core::LevelWriter::toJsonString(*loop.level).find("\"loop\""), std::string::npos);
}

/**
 * @brief Une plateforme immobile (route vide) n'écrit aucun champ de route : ni `waypoints`, ni le
 *        couple `endX`/`endY` que l'éditeur ne produit plus (`EX-LVL-008`).
 * \castest{<b>Une plateforme sans route n'écrit aucun champ de route.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Charger une plateforme sans waypoints ni endX/endY.<br/>2. Serialiser.<br/>
 * \tattendu Le JSON produit ne contient ni "waypoints" ni "endX".
 * }
 */
TEST(LevelWriterTest, PlateformeSansRouteNEcritAucunChampDeRoute) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(levelWithPlatform(""));
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("waypoints"), std::string::npos);
    EXPECT_EQ(json.find("endX"), std::string::npos);
}

/**
 * @brief Une plateforme lue à l'ancien format (`endX`/`endY`) est réécrite en `waypoints` sans
 *        changer le parcours : la migration du contenu est portée par la sérialisation elle-même
 *        (`EX-LVL-008`).
 * \castest{<b>Une plateforme au format endX/endY est réécrite en waypoints.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger une plateforme ecrite avec endX/endY.<br/>2. Serialiser puis
 * recharger.<br/>
 * \tattendu Le JSON produit porte "waypoints" et non "endX", et la route rechargee est la meme.
 * }
 */
TEST(LevelWriterTest, PlateformeAncienFormatEstReecriteEnWaypoints) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(
        levelWithPlatform(R"(, "endX": 4, "endY": 1, "speed": 3.0)"));
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_NE(json.find("waypoints"), std::string::npos);
    EXPECT_EQ(json.find("endX"), std::string::npos);

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    ASSERT_EQ(reloaded.level->platformConfigs().size(), 1u);
    EXPECT_EQ(reloaded.level->platformConfigs().front().waypoints,
              (std::vector<core::GridPosition>{core::GridPosition{4, 1}}));
}

/**
 * @brief Les capacités du tableau survivent à un aller-retour et restent omises quand le niveau
 *        s'en remet aux réglages du moteur (`EX-GP-055`).
 * \castest{<b>Les capacités du tableau survivent à un aller-retour et sont omises par
 * défaut.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau declarant airJumps et dashCharges, serialiser puis
 * recharger.<br/>2. Faire de meme avec un niveau qui n'en declare aucun.<br/>
 * \tattendu Les capacites declarees sont conservees ; sans declaration, les champs n'apparaissent
 * pas dans le JSON produit et restent absents apres rechargement.
 * }
 */
TEST(LevelWriterTest, CapacitesDuTableauSurviventAuRoundTripEtSontOmisesParDefaut) {
    constexpr const char* AVEC = R"({
      "name": "N", "width": 4, "height": 3, "airJumps": 2, "dashCharges": 3,
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(AVEC);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const core::LevelLoadResult reloaded =
        core::LevelLoader::loadFromString(core::LevelWriter::toJsonString(*loaded.level));
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    ASSERT_TRUE(reloaded.level->airJumps().has_value());
    EXPECT_EQ(*reloaded.level->airJumps(), 2);
    ASSERT_TRUE(reloaded.level->dashCharges().has_value());
    EXPECT_EQ(*reloaded.level->dashCharges(), 3);

    constexpr const char* SANS = R"({
      "name": "N", "width": 4, "height": 3,
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    const core::LevelLoadResult defauts = core::LevelLoader::loadFromString(SANS);
    ASSERT_TRUE(defauts.ok()) << defauts.error;
    EXPECT_FALSE(defauts.level->airJumps().has_value());
    EXPECT_FALSE(defauts.level->dashCharges().has_value());

    const std::string json = core::LevelWriter::toJsonString(*defauts.level);
    EXPECT_EQ(json.find("airJumps"), std::string::npos);
    EXPECT_EQ(json.find("dashCharges"), std::string::npos);
}
