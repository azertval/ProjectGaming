// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_replay_playback.cpp
 * @brief Tests unitaires de `hmi::ReplayPlayback` (`LOT-ANNEXE-18`, TACHE-01, `EX-IA-019`).
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "AiSolver/Replay/LevelFingerprint.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "Core/Physics/PlayerInput.h"
#include "HMI/Game/ReplayPlayback.h"

using hmi::ReplayPlayback;

namespace {

// Un niveau reel (livre) sert de reference : son empreinte doit correspondre a la lecture reelle
// du fichier sur disque, pas a un niveau synthetise en memoire (LOT-ANNEXE-17).
constexpr const char* kLevelFileName = "demo-deplacement.json";

aisolver::LevelFingerprint realFingerprint() {
    const std::filesystem::path levelPath =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / kLevelFileName;
    std::ifstream file(levelPath, std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return aisolver::computeLevelFingerprint(contents.str());
}

std::filesystem::path writeReplayFixture(const char* suffix, aisolver::LevelFingerprint fingerprint) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                       (std::string("aisolver_test_replay_playback_") + suffix +
                                        ".json");
    aisolver::ReplayFile file;
    file.levelPath = kLevelFileName;
    file.levelFingerprint = fingerprint;
    file.steps = {core::PlayerInput{.moveX = 1.0f}, core::PlayerInput{.moveX = 0.0f}};
    file.algorithmName = "test";
    EXPECT_TRUE(aisolver::writeReplay(path, file));
    return path;
}

}  // namespace

/**
 * \castest{ReplayPlayback accepte un rejeu valide et rejoue sa sequence dans l'ordre.<br/>
 * \tcat Unitaire · HMI Game<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un fichier de rejeu avec l'empreinte reelle du niveau.<br/>2. Construire
 * ReplayPlayback dessus, appeler nextInput() jusqu'a epuisement.<br/>
 * \tattendu valid() est vrai ; les pas sont renvoyes dans l'ordre exact, puis std::nullopt.}
 */
TEST(ReplayPlaybackTest, RejeuValideRejoueLaSequenceDansLOrdre) {
    const std::filesystem::path path = writeReplayFixture("valide", realFingerprint());
    ReplayPlayback playback(path, std::filesystem::path(PROJECTGAMING_LEVELS_DIR));

    ASSERT_TRUE(playback.valid()) << playback.error();
    EXPECT_EQ(playback.levelPath(), kLevelFileName);

    const std::optional<core::PlayerInput> first = playback.nextInput();
    ASSERT_TRUE(first.has_value());
    EXPECT_FLOAT_EQ(first->moveX, 1.0f);

    const std::optional<core::PlayerInput> second = playback.nextInput();
    ASSERT_TRUE(second.has_value());
    EXPECT_FLOAT_EQ(second->moveX, 0.0f);

    EXPECT_EQ(playback.nextInput(), std::nullopt);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

/**
 * \castest{ReplayPlayback refuse un rejeu dont l'empreinte de niveau ne correspond pas.<br/>
 * \tcat Unitaire · HMI Game<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un fichier de rejeu avec une empreinte fausse.<br/>2. Construire
 * ReplayPlayback dessus.<br/>
 * \tattendu valid() est faux, error() non vide, nextInput() renvoie toujours std::nullopt.}
 */
TEST(ReplayPlaybackTest, RejeuInvalideEstRefuseSansPlantage) {
    const std::filesystem::path path = writeReplayFixture("invalide", /*fingerprint=*/999u);
    ReplayPlayback playback(path, std::filesystem::path(PROJECTGAMING_LEVELS_DIR));

    EXPECT_FALSE(playback.valid());
    EXPECT_FALSE(playback.error().empty());
    EXPECT_EQ(playback.nextInput(), std::nullopt);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

/**
 * \castest{ReplayPlayback refuse un fichier de rejeu introuvable.<br/>
 * \tcat Unitaire · HMI Game<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire ReplayPlayback sur un chemin inexistant.<br/>
 * \tattendu valid() est faux, sans plantage.}
 */
TEST(ReplayPlaybackTest, FichierDeRejeuIntrouvableEstRecuperable) {
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "aisolver_test_replay_playback_absent.json";
    std::error_code ignored;
    std::filesystem::remove(missing, ignored);

    ReplayPlayback playback(missing, std::filesystem::path(PROJECTGAMING_LEVELS_DIR));

    EXPECT_FALSE(playback.valid());
    EXPECT_EQ(playback.nextInput(), std::nullopt);
}
