// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_replay_file.cpp
 * @brief Tests unitaires de aisolver::ReplayFile / writeReplay / readReplay (LOT-ANNEXE-07,
 * TACHE-03/04).
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "AiSolver/Env/ActionDecoding.h"
#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "Core/Levels/LevelOutcome.h"

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

/// Repertoire temporaire du test courant, nettoye a la destruction (RAII), meme patron que
/// Source/Test/Unit/AiSolver/Nn/test_serialization.cpp.
class TempDirectory {
public:
    TempDirectory() : _path(std::filesystem::temp_directory_path() / "aisolver_test_replay_file") {
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
 * @brief Un `ReplayFile` ecrit puis relu produit une structure identique champ par champ.
 * \castest{<b>Round-trip complet d'un `ReplayFile` (metadonnees et sequence de plusieurs
 * milliers de pas).</b><br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un `ReplayFile` avec une sequence de 5000 pas varies.<br/>2.
 * `writeReplay` puis `readReplay` sur le meme chemin.<br/>
 * \tattendu La lecture reussit ; toutes les metadonnees et chaque pas de la sequence relue sont
 * identiques a l'original.}
 */
TEST(ReplayFileTest, RoundTripCompletAvecMetadonnees) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_round_trip.json");

    aisolver::ReplayFile original;
    original.levelPath = "demo-deplacement.json";
    original.levelFingerprint = 123456789ULL;
    original.algorithmName = "reinforce";
    original.exportedAtIso8601 = "2026-08-23T08:30:00Z";
    original.seed = 987654321ULL;
    original.finalReward = 42.5f;
    for (int step = 0; step < 5000; ++step) {
        const aisolver::Action action = aisolver::actionAt(static_cast<std::size_t>(step) % aisolver::actionCount());
        original.steps.push_back(aisolver::toPlayerInput(action));
    }

    ASSERT_TRUE(aisolver::writeReplay(path, original));
    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    ASSERT_TRUE(result.ok()) << result.error;

    const aisolver::ReplayFile& reread = *result.replay;
    EXPECT_EQ(reread.formatVersion, original.formatVersion);
    EXPECT_EQ(reread.levelPath, original.levelPath);
    EXPECT_EQ(reread.levelFingerprint, original.levelFingerprint);
    EXPECT_EQ(reread.algorithmName, original.algorithmName);
    EXPECT_EQ(reread.exportedAtIso8601, original.exportedAtIso8601);
    EXPECT_EQ(reread.seed, original.seed);
    EXPECT_FLOAT_EQ(reread.finalReward, original.finalReward);
    ASSERT_EQ(reread.steps.size(), original.steps.size());
    for (std::size_t i = 0; i < original.steps.size(); ++i) {
        EXPECT_FLOAT_EQ(reread.steps[i].moveX, original.steps[i].moveX) << "pas " << i;
        EXPECT_EQ(reread.steps[i].jumpPressed, original.steps[i].jumpPressed) << "pas " << i;
        EXPECT_EQ(reread.steps[i].jumpHeld, original.steps[i].jumpHeld) << "pas " << i;
        EXPECT_EQ(reread.steps[i].dashPressed, original.steps[i].dashPressed) << "pas " << i;
    }
}

/**
 * @brief Un fichier JSON sans champ `formatVersion` est lu comme la version initiale, sans erreur.
 * \castest{<b>Absence de `formatVersion` -> version initiale, sans erreur.</b><br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Ecrire a la main un fichier JSON minimal sans champ `formatVersion`.<br/>2.
 * `readReplay` sur ce fichier.<br/>
 * \tattendu La lecture reussit (`ok()` vrai), `formatVersion` vaut 0.}
 */
TEST(ReplayFileTest, AbsenceDeVersionLueCommeVersionInitiale) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_no_version.json");
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file << R"({"levelPath": "demo-deplacement.json", "steps": []})";
    }

    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.replay->formatVersion, 0u);
}

/**
 * @brief `readReplay` sur un chemin inexistant renvoie une erreur recuperable, jamais d'exception.
 * \castest{<b>Fichier introuvable -> erreur recuperable.</b><br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `readReplay` sur un chemin qui n'existe pas.<br/>
 * \tattendu `ok()` est faux, `error` est non vide.}
 */
TEST(ReplayFileTest, FichierIntrouvableRenvoieUneErreur) {
    TempDirectory tempDir;
    const aisolver::ReplayLoadResult result = aisolver::readReplay(tempDir.file("inexistant.json"));
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}

/**
 * @brief `readReplay` sur un contenu non-JSON renvoie une erreur recuperable, jamais d'exception.
 * \castest{<b>JSON malforme -> erreur recuperable.</b><br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Ecrire un fichier contenant du texte non-JSON.<br/>2. `readReplay` sur ce
 * fichier.<br/>
 * \tattendu `ok()` est faux, `error` est non vide.}
 */
TEST(ReplayFileTest, JsonMalformeRenvoieUneErreur) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_malformed.json");
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file << "{ceci n'est pas du json valide";
    }

    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}

/**
 * @brief Un `ReplayFile` sans aucun pas (`steps` vide) est un cas valide a l'ecriture/lecture.
 * \castest{<b>Sequence vide valide a l'ecriture/lecture.</b><br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire un `ReplayFile` avec `steps` vide.<br/>2. `writeReplay` puis
 * `readReplay`.<br/>
 * \tattendu La lecture reussit, `steps` relu est vide.}
 */
TEST(ReplayFileTest, SequenceVideValide) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_empty.json");

    aisolver::ReplayFile replay;
    replay.levelPath = "demo-deplacement.json";
    replay.algorithmName = "evolutionnaire";

    ASSERT_TRUE(aisolver::writeReplay(path, replay));
    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_TRUE(result.replay->steps.empty());
}

/**
 * @brief Test de bout en bout : une sequence d'actions decodees par `decodeArgmax`, exportee puis
 * relue, rejoue exactement la meme trajectoire sur `HeadlessLevelEnvironment`.
 * \castest{<b>Pipeline decodage -> export -> rejeu, determinisme de bout en bout.</b><br/>
 * \tcat Unitaire · AiSolver Replay<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Decoder une sequence de 60 actions (`decodeArgmax` sur des distributions
 * synthetiques constantes : marche a droite), convertir en `core::PlayerInput`, assembler en
 * `ReplayFile`, ecrire puis relire.<br/>2. Rejouer la sequence d'origine et la sequence relue sur
 * deux `HeadlessLevelEnvironment` distincts, sur `demo-deplacement.json`.<br/>
 * \tattendu Meme issue finale (`core::LevelOutcome`) et meme position finale du personnage pour
 * les deux rejeux.}
 */
TEST(ReplayFileTest, PipelineDecodageExportRejeuDeterministe) {
    TempDirectory tempDir;
    const std::filesystem::path path = tempDir.file("replay_end_to_end.json");

    // Distribution synthetique constante : concentree sur l'action "marcher a droite, rien
    // d'autre" (indice de l'action Direction::Right, aucun saut/dash).
    const aisolver::Action rightAction{aisolver::Direction::Right, false, false, false};
    aisolver::Tensor<float> distribution({aisolver::actionCount()});
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = 0.0f;
    }
    distribution.data()[aisolver::indexOf(rightAction)] = 1.0f;

    aisolver::ReplayFile replay;
    replay.levelPath = "demo-deplacement.json";
    replay.algorithmName = "test";
    replay.exportedAtIso8601 = "2026-08-23T00:00:00Z";
    replay.seed = 1;
    for (int step = 0; step < 60; ++step) {
        const aisolver::Action decoded = aisolver::decodeArgmax(distribution);
        replay.steps.push_back(aisolver::toPlayerInput(decoded));
    }

    ASSERT_TRUE(aisolver::writeReplay(path, replay));
    const aisolver::ReplayLoadResult result = aisolver::readReplay(path);
    ASSERT_TRUE(result.ok()) << result.error;

    aisolver::HeadlessLevelEnvironment originalEnv;
    ASSERT_TRUE(originalEnv.reset(levelPath("demo-deplacement.json")));
    aisolver::StepObservation originalObservation;
    for (const core::PlayerInput& step : replay.steps) {
        originalObservation = originalEnv.step(step);
    }

    aisolver::HeadlessLevelEnvironment rereadEnv;
    ASSERT_TRUE(rereadEnv.reset(levelPath("demo-deplacement.json")));
    aisolver::StepObservation rereadObservation;
    for (const core::PlayerInput& step : result.replay->steps) {
        rereadObservation = rereadEnv.step(step);
    }

    EXPECT_EQ(originalObservation.outcome, rereadObservation.outcome);
    EXPECT_NEAR(originalObservation.playerBox.min.x, rereadObservation.playerBox.min.x, 1e-6f);
    EXPECT_NEAR(originalObservation.playerBox.min.y, rereadObservation.playerBox.min.y, 1e-6f);
}
