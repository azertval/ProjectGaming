// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_replay_format.cpp
 * @brief Tests unitaires de la stabilisation du format de rejeu v1 (LOT-ANNEXE-17, TACHE-02) :
 * `totalDurationSeconds`/`algorithmId`, compatibilite ascendante `formatVersion` 1 et 2.
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "AiSolver/Replay/ReplayFile.h"

namespace {

/// Repertoire temporaire du test courant, nettoye a la destruction (RAII), meme patron que
/// Source/Test/Unit/AiSolver/Replay/test_replay_file.cpp.
class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() / "aisolver_test_replay_format") {
        std::filesystem::create_directories(_path);
    }
    ~TempDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    [[nodiscard]] std::filesystem::path file(const char* name) const {
        return _path / name;
    }

private:
    std::filesystem::path _path;
};

}  // namespace

/**
 * @brief Un fichier `formatVersion == 1` (sans `totalDurationSeconds`/`algorithmId`) reste
 * lisible ; les deux nouveaux champs prennent leur valeur sentinelle.
 * \castest{Lecture d'un fichier formatVersion == 1 : valeurs sentinelle.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Ecrire a la main un fichier JSON `formatVersion: 1`, sans les deux nouveaux
 * champs.<br/>2. `readReplay`.<br/>
 * \tattendu La lecture reussit ; `totalDurationSeconds == 0.0f` et `algorithmId == ""`.}
 */
TEST(ReplayFormatTest, FichierVersion1SansNouveauxChampsResteLisible) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_v1.json");
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file << R"({
            "formatVersion": 1,
            "levelPath": "demo-deplacement.json",
            "levelFingerprint": 0,
            "algorithmName": "evolutionnaire",
            "exportedAtIso8601": "2026-08-23T08:30:00Z",
            "seed": 1,
            "finalReward": 1.0,
            "steps": []
        })";
    }

    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.replay->formatVersion, 1u);
    EXPECT_FLOAT_EQ(result.replay->totalDurationSeconds, 0.0f);
    EXPECT_EQ(result.replay->algorithmId, "");
}

/**
 * @brief Un fichier `formatVersion == 2` restitue exactement les valeurs de
 * `totalDurationSeconds`/`algorithmId` ecrites.
 * \castest{Lecture d'un fichier formatVersion == 2 : champs restitues.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Ecrire a la main un fichier JSON `formatVersion: 2` avec les deux nouveaux
 * champs.<br/>2. `readReplay`.<br/>
 * \tattendu `totalDurationSeconds` et `algorithmId` relus sont identiques a ceux ecrits.}
 */
TEST(ReplayFormatTest, FichierVersion2RestitueLesNouveauxChamps) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_v2.json");
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file << R"({
            "formatVersion": 2,
            "levelPath": "demo-deplacement.json",
            "levelFingerprint": 0,
            "algorithmName": "reinforce",
            "exportedAtIso8601": "2026-08-23T08:30:00Z",
            "seed": 1,
            "finalReward": 1.0,
            "totalDurationSeconds": 12.5,
            "algorithmId": "pg",
            "steps": []
        })";
    }

    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.replay->formatVersion, 2u);
    EXPECT_FLOAT_EQ(result.replay->totalDurationSeconds, 12.5f);
    EXPECT_EQ(result.replay->algorithmId, "pg");
}

/**
 * @brief Ecrire puis relire un `ReplayFile` produit un `algorithmId`/`totalDurationSeconds`
 * identiques a ceux fournis a l'export ; `formatVersion` par defaut vaut 2 (nouvel export).
 * \castest{Round-trip des nouveaux champs.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `ReplayFile` avec `totalDurationSeconds`/`algorithmId`
 * renseignes.<br/>2. `writeReplay` puis `readReplay`.<br/>
 * \tattendu Les deux champs relus sont identiques a l'original ; `formatVersion == 2`.}
 */
TEST(ReplayFormatTest, RoundTripDesNouveauxChamps) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_round_trip_v2.json");

    aisolver::ReplayFile original;
    original.levelPath = "demo-deplacement.json";
    original.algorithmName = "acteur-critique";
    original.totalDurationSeconds = 3.75f;
    original.algorithmId = "ac";

    ASSERT_TRUE(aisolver::writeReplay(path, original));
    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    ASSERT_TRUE(result.ok()) << result.error;

    EXPECT_EQ(result.replay->formatVersion, aisolver::kReplayFormatVersion);
    EXPECT_EQ(result.replay->formatVersion, 2u);
    EXPECT_FLOAT_EQ(result.replay->totalDurationSeconds, original.totalDurationSeconds);
    EXPECT_EQ(result.replay->algorithmId, original.algorithmId);
}
