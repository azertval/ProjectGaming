// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_replay_validation.cpp
 * @brief Tests unitaires de `aisolver::validateReplay` (LOT-ANNEXE-17, TACHE-01, `EX-IA-018`).
 */

#include <filesystem>
#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "AiSolver/Replay/LevelFingerprint.h"
#include "AiSolver/Replay/ReplayValidation.h"

namespace {

std::filesystem::path levelsDir() {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR);
}

/// Empreinte reelle du fichier de niveau @p fileName sous `levelsDir()`, lu tel quel.
aisolver::LevelFingerprint realFingerprintOf(const char* fileName) {
    std::ifstream file(levelsDir() / fileName, std::ios::binary);
    std::ostringstream contents;
    contents << file.rdbuf();
    return aisolver::computeLevelFingerprint(contents.str());
}

}  // namespace

/**
 * @brief Le niveau reference n'existe pas sous `levelsDir` : erreur `LevelFileMissing`.
 * \castest{Niveau absent -> LevelFileMissing.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `ReplayFile::levelPath` reference un fichier inexistant.<br/>2.
 * `validateReplay`.<br/>
 * \tattendu Le resultat contient `ReplayValidationError::LevelFileMissing`.}
 */
TEST(ReplayValidationTest, NiveauAbsentRenvoieLevelFileMissing) {
    aisolver::ReplayFile replay;
    replay.levelPath = "niveau-qui-n-existe-pas.json";
    replay.levelFingerprint = 0;

    const std::optional<aisolver::ReplayValidationError> error = aisolver::validateReplay(replay, levelsDir());
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(*error, aisolver::ReplayValidationError::LevelFileMissing);
}

/**
 * @brief Le niveau reference existe mais son empreinte diverge de celle enregistree : erreur
 * `LevelFingerprintMismatch`.
 * \castest{Empreinte divergente -> LevelFingerprintMismatch.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `ReplayFile` reference un niveau reel existant, avec une empreinte factice.<br/>2.
 * `validateReplay`.<br/>
 * \tattendu Le resultat contient `ReplayValidationError::LevelFingerprintMismatch`.}
 */
TEST(ReplayValidationTest, EmpreinteDivergenteRenvoieLevelFingerprintMismatch) {
    aisolver::ReplayFile replay;
    replay.levelPath = "demo-deplacement.json";
    replay.levelFingerprint = 0xDEADBEEFULL;  // Empreinte factice, ne correspond a aucun contenu reel.

    const std::optional<aisolver::ReplayValidationError> error = aisolver::validateReplay(replay, levelsDir());
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(*error, aisolver::ReplayValidationError::LevelFingerprintMismatch);
}

/**
 * @brief Le niveau reference existe et son empreinte correspond exactement : rejeu valide, aucun
 * faux positif.
 * \castest{Rejeu valide -> aucune erreur.<br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `ReplayFile` reference un niveau reel, avec sa veritable empreinte.<br/>2.
 * `validateReplay`.<br/>
 * \tattendu Le resultat est `std::nullopt`.}
 */
TEST(ReplayValidationTest, RejeuValideNeRenvoieAucuneErreur) {
    aisolver::ReplayFile replay;
    replay.levelPath = "demo-deplacement.json";
    replay.levelFingerprint = realFingerprintOf("demo-deplacement.json");

    const std::optional<aisolver::ReplayValidationError> error = aisolver::validateReplay(replay, levelsDir());
    EXPECT_FALSE(error.has_value());
}
