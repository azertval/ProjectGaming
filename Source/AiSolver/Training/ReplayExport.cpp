// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ReplayExport.h"

#include <ctime>
#include <utility>

#include "AiSolver/Replay/ReplayFile.h"

namespace aisolver::training {

namespace {

constexpr const char* kAlgorithmName = "evolutionnaire";

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
                                const std::string& algorithmName, std::uint64_t seed) {
    if (!solved) {
        return ReplayExportResult{false, ReplayExportError::NotSolved};
    }

    ReplayFile file;
    file.levelPath = levelPath.filename().string();
    file.steps = replay.steps;
    file.algorithmName = algorithmName;
    file.exportedAtIso8601 = currentIso8601();
    file.seed = seed;
    file.finalReward = replay.finalReward;

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
        replay, trainingResult.solved, levelPath, replayOutputPath, kAlgorithmName, seed);

    return TrainAndExportOutcome{std::move(trainingResult), exportResult};
}

}  // namespace aisolver::training
