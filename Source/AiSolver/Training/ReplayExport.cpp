// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ReplayExport.h"

#include <ctime>
#include <fstream>
#include <sstream>
#include <utility>

#include "AiSolver/Replay/LevelFingerprint.h"
#include "AiSolver/Replay/ReplayFile.h"

namespace aisolver::training {

namespace {

constexpr const char* kAlgorithmName = "evolutionnaire";
constexpr const char* kAlgorithmId = "evo";

// Meme pas fixe que HeadlessLevelEnvironment (kFixedDelta, prive a son .cpp) : duplique plutot que
// reexpose, meme convention que le defaut de core::FixedTimestep -- une seule source de simulation
// (le rejeu lui-meme, deja produit a ce pas) fait foi, cette valeur ne sert qu'a l'affichage de
// `totalDurationSeconds`.
constexpr float kFixedDeltaSeconds = 1.0f / 60.0f;

// Horodatage ISO 8601 (UTC), meme principe que TrainingRunPath.cpp::currentIso8601Compact mais
// avec les separateurs complets attendus par ReplayFile::exportedAtIso8601 (helper prive, non
// reexpose depuis AiSolver/Stats -- ce module ne depend pas de Stats).
std::string currentIso8601() {
    const std::time_t now = std::time(nullptr);
    std::tm utcTime{};
#ifdef _WIN32
    gmtime_s(&utcTime, &now);
#else
    gmtime_r(&now, &utcTime);
#endif
    char buffer[32] = {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
    return buffer;
}

}  // namespace

ReplayExportResult exportReplay(const DeterministicReplayResult& replay, bool solved,
                                const std::filesystem::path& levelPath,
                                const std::filesystem::path& outputPath,
                                const std::string& algorithmName, std::uint64_t seed,
                                const std::string& algorithmId) {
    if (!solved) {
        return ReplayExportResult{false, ReplayExportError::NotSolved};
    }

    ReplayFile file;
    file.levelPath = levelPath.filename().string();
    // Empreinte du niveau SOURCE (LOT-ANNEXE-17, EX-IA-018) : sans elle, aucun rejeu exporte ne
    // validerait jamais a la lecture (aisolver::validateReplay refuserait systematiquement un
    // rejeu a empreinte nulle des que le fichier de niveau existe) -- calculee une fois ici, sur
    // le meme fichier que celui entraine, meme convention de lecture (contenu brut) que
    // aisolver::validateReplay, jamais recalculee ailleurs. Fichier illisible (rarissime : le
    // meme fichier vient d'etre entraine avec succes) : empreinte nulle par repli, la meme
    // divergence sera alors detectee a la lecture plutot que masquee ici (EX-NFR-040).
    std::ifstream levelFile(levelPath, std::ios::binary);
    if (levelFile) {
        std::ostringstream levelContents;
        levelContents << levelFile.rdbuf();
        file.levelFingerprint = computeLevelFingerprint(levelContents.str());
    }
    file.steps = replay.steps;
    file.algorithmName = algorithmName;
    file.exportedAtIso8601 = currentIso8601();
    file.seed = seed;
    file.finalReward = replay.finalReward;
    file.totalDurationSeconds = static_cast<float>(replay.steps.size()) * kFixedDeltaSeconds;
    file.algorithmId = algorithmId;

    const bool written = writeReplay(outputPath, file);
    return ReplayExportResult{written, written ? ReplayExportError::None
                                               : ReplayExportError::WriteFailed};
}

TrainAndExportOutcome trainLevelAndExportReplay(const std::filesystem::path& levelPath,
                                                const evolutionary::NetworkTopology& topology,
                                                const evolutionary::EvolutionaryConfig& config,
                                                const StoppingConfig& stopping, std::uint64_t seed,
                                                const std::filesystem::path& statsCsvPath,
                                                const std::filesystem::path& replayOutputPath,
                                                EnvironmentConfig environmentConfig) {
    LevelTrainingSession session(levelPath, topology, config, stopping, seed, statsCsvPath,
                                 environmentConfig);
    TrainingResult trainingResult = session.run();

    HeadlessLevelEnvironment replayEnvironment(environmentConfig);
    const DeterministicReplayResult replay =
        replayBestIndividual(trainingResult.bestIndividual, replayEnvironment, levelPath);

    const ReplayExportResult exportResult = exportReplay(
        replay, trainingResult.solved, levelPath, replayOutputPath, kAlgorithmName, seed,
        kAlgorithmId);

    return TrainAndExportOutcome{std::move(trainingResult), exportResult};
}

}  // namespace aisolver::training
