// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rejeu_ia.cpp
 * @brief Test système : un fichier de rejeu réel (produit par un entraînement) rejoue jusqu'à
 * `Won` (`LOT-ANNEXE-18`, TACHE-03, `EX-IA-019`).
 *
 * Pendant du test symétrique de `test_parcours_complet.cpp` : celui-là prouve qu'un script
 * d'entrées codé en dur mène à `Won`, celui-ci prouve la même chose pour la relecture d'un fichier
 * de rejeu au format v2 (`aisolver::ReplayFile`, `LOT-ANNEXE-07`/`LOT-ANNEXE-17`), avec sa
 * validation (`aisolver::validateReplay`).
 *
 * **Écart assumé par rapport au texte de TACHE-01/TACHE-03** : la simulation est rejouée via
 * `aisolver::HeadlessLevelEnvironment` (`LOT-ANNEXE-05`), pas via `hmi::GameSession`/
 * `hmi::ReplayPlayback` directement. `hmi::GameSession` exige un `QRhi*` réel (`SpriteBatch`,
 * `TextureAtlas`, `TextureCache` — voir leurs constructeurs) : aucune fenêtre ni GPU n'existe dans
 * ce binaire de tests (`SystemTests`, headless par construction, même contrainte que le reste du
 * programme Lot-Annexe), et le code `Source/HMI` compile directement dans l'exécutable du jeu
 * (`ProjectGaming`), pas dans une bibliothèque réutilisable séparément. `GameSession::update`
 * (surchargée par `LOT-ANNEXE-18` TACHE-01 pour accepter un `core::PlayerInput` déjà résolu) et
 * `HeadlessLevelEnvironment::step` partagent la même physique déterministe (`core::
 * CharacterPhysicsSystem`, `EX-NFR-002`) : ce test couvre la fidélité de la séquence enregistrée
 * face à cette physique, la couche `hmi::GameSession`/rendu restant, comme pour tout le reste du
 * projet, vérifiée visuellement par l'utilisateur.
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "AiSolver/Replay/ReplayValidation.h"
#include "Core/Levels/LevelOutcome.h"

namespace {

std::filesystem::path fixturePath() {
    return std::filesystem::path(PROJECTGAMING_REPLAY_FIXTURES_DIR) /
           "rejeu-test-deplacement.json";
}

std::filesystem::path levelsDir() {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR);
}

}  // namespace

/**
 * \castest{Rejeu de la fixture aboutit a Won.<br/>
 * \tcat Systeme<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Lire et valider la fixture de rejeu.<br/>2. Rejouer sa sequence sur
 * HeadlessLevelEnvironment jusqu'a Won/Lost/epuisement.<br/>
 * \tattendu L'issue finale est core::LevelOutcome::Won.}
 */
TEST(RejeuIaSysteme, LaFixtureAboutitAWon) {
    const aisolver::ReplayLoadResult loaded = aisolver::readReplay(fixturePath());
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::optional<aisolver::ReplayValidationError> validationError =
        aisolver::validateReplay(*loaded.replay, levelsDir());
    ASSERT_EQ(validationError, std::nullopt);

    aisolver::HeadlessLevelEnvironment environment;
    ASSERT_TRUE(environment.reset(levelsDir() / loaded.replay->levelPath));

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (const core::PlayerInput& input : loaded.replay->steps) {
        outcome = environment.step(input).outcome;
        if (outcome != core::LevelOutcome::Playing) {
            break;
        }
    }
    EXPECT_EQ(outcome, core::LevelOutcome::Won);
}

/**
 * \castest{Rejeu invalide si la fixture de niveau change.<br/>
 * \tcat Systeme<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Copier le niveau reference dans un repertoire temporaire, en le modifiant.<br/>2.
 * Valider le rejeu contre ce repertoire modifie.<br/>
 * \tattendu ReplayValidationError::LevelFingerprintMismatch (jamais un plantage).}
 */
TEST(RejeuIaSysteme, RejeuInvalideSiLeNiveauChange) {
    const aisolver::ReplayLoadResult loaded = aisolver::readReplay(fixturePath());
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::filesystem::path tempDir =
        std::filesystem::temp_directory_path() / "aisolver_test_rejeu_ia_niveau_modifie";
    std::error_code ignored;
    std::filesystem::remove_all(tempDir, ignored);
    std::filesystem::create_directories(tempDir);

    // Copie MODIFIEE du niveau reference (jamais le fichier committe) : un octet suffit a faire
    // diverger l'empreinte FNV-1a (LOT-ANNEXE-17).
    {
        std::ifstream original(levelsDir() / loaded.replay->levelPath, std::ios::binary);
        ASSERT_TRUE(original);
        std::ostringstream contents;
        contents << original.rdbuf();
        std::string modified = contents.str();
        modified += " ";
        std::ofstream copy(tempDir / loaded.replay->levelPath, std::ios::binary | std::ios::trunc);
        copy << modified;
    }

    const std::optional<aisolver::ReplayValidationError> validationError =
        aisolver::validateReplay(*loaded.replay, tempDir);
    EXPECT_EQ(validationError, aisolver::ReplayValidationError::LevelFingerprintMismatch);

    std::filesystem::remove_all(tempDir, ignored);
}
