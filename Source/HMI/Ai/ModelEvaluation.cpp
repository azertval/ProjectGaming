// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Ai/ModelEvaluation.h"

#include <cmath>
#include <filesystem>
#include <memory>

#include "AiSolver/Cli/TrainingConfig.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/ActionDecodingMode.h"
#include "AiSolver/Eval/ActorCriticTrainedPolicy.h"
#include "AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h"
#include "AiSolver/Eval/BenchmarkConfig.h"
#include "AiSolver/Eval/BenchmarkReport.h"
#include "AiSolver/Eval/BenchmarkResult.h"
#include "AiSolver/Eval/BenchmarkRunner.h"
#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"
#include "AiSolver/Eval/ReinforceTrainedPolicy.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/Serialization.h"
#include "AiSolver/Training/ArgmaxRollout.h"
#include "AiSolver/Training/Dqn/QNetwork.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/ReplayExport.h"
#include "Core/Diagnostics/ScopedLogLevel.h"

namespace hmi {

// Meme dispatch que aisolver::cli::runEvaluate (Cli/Commands.cpp, LOT-ANNEXE-19) : adapte a un
// resultat en types simples plutot qu'un code de sortie/flux console, jamais une regle differente.
std::optional<EvaluationOutcome> evaluateModel(
    const EvaluationRequest& request,
    const std::function<bool(int completed, int total)>& onRepetition) {
    using namespace aisolver;

    // Voir TrainingWorker::run / aisolver::cli::runTrain : `repetitions` rejeux headless
    // produiraient sinon un volume de traces Levels/Gameplay sans rapport avec une partie reelle.
    const core::ScopedLogLevel quietDuringEvaluation(core::defaultLogger(),
                                                     core::LogLevel::Warning);

    const std::filesystem::path model = request.modelPath.toStdString();
    const std::filesystem::path level = request.levelPath.toStdString();
    const std::string algorithmIdStd = request.algorithmId.toStdString();

    const std::size_t inputSize = ObservationEncoder().inputSize();
    // Taille relue du run qui a produit le modele, jamais supposee : voir
    // `cli::hiddenSizeForModel`.
    const std::size_t hiddenSize = cli::hiddenSizeForModel(model);
    const training::evolutionary::NetworkTopology topology =
        training::evolutionary::policyTopology(inputSize, hiddenSize);
    Rng scratchRng(0);

    eval::BenchmarkConfig config;
    config.repetitions = request.repetitions;
    config.maxStepsPerEpisode = request.maxStepsPerEpisode;
    config.rngSeedBase = request.seed;
    config.decodingMode = request.stochasticDecoding ? eval::ActionDecodingMode::Stochastic
                                                     : eval::ActionDecodingMode::Argmax;

    eval::BenchmarkResult result;
    if (algorithmIdStd == "avance") {
        training::QNetwork network(inputSize, hiddenSize, scratchRng);
        if (!nn::loadWeights(network.network(), model)) {
            return std::nullopt;
        }
        eval::AdvancedAlgorithmTrainedPolicy policy(network);
        result = eval::BenchmarkRunner::run(policy, level, config, onRepetition);
    } else {
        std::unique_ptr<nn::Network> network =
            training::evolutionary::buildNetwork(topology, scratchRng);
        if (!nn::loadWeights(*network, model)) {
            return std::nullopt;
        }
        if (algorithmIdStd == "pg") {
            eval::ReinforceTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, level, config, onRepetition);
        } else if (algorithmIdStd == "ac") {
            eval::ActorCriticTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, level, config, onRepetition);
        } else {
            eval::EvolutionaryTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, level, config, onRepetition);
        }
    }

    return EvaluationOutcome{result.successRate(), result.meanStepsOnSuccess(),
                             result.meanStepsAll(), result.stepVariance(),
                             static_cast<int>(result.episodes.size())};
}

bool writeEvaluationReport(const EvaluationRequest& request, const EvaluationOutcome& outcome,
                           const QString& csvPath) {
    using namespace aisolver;

    // `BenchmarkReport::addResult` derive ses colonnes d'un `BenchmarkResult` complet, que l'ecran
    // ne conserve pas (il n'affiche que des agregats). On reconstruit donc la seule chose dont le
    // rapport a besoin : autant d'episodes que de repetitions jouees, dont la proportion de
    // reussites et la longueur moyenne redonnent exactement les agregats mesures.
    eval::BenchmarkResult result;
    const int total = outcome.repetitionsRun;
    const int successes = static_cast<int>(std::lround(outcome.successRate * total));
    const int stepsOnSuccess = static_cast<int>(std::lround(outcome.meanStepsOnSuccess));
    // Les episodes echoues portent le reste des pas, pour que `meanStepsAll` soit conserve.
    const double failedStepsTotal =
        outcome.meanStepsAll * total - static_cast<double>(successes) * stepsOnSuccess;
    const int failures = total - successes;
    const int stepsOnFailure =
        failures > 0 ? static_cast<int>(std::lround(failedStepsTotal / failures)) : 0;
    result.episodes.reserve(static_cast<std::size_t>(total));
    for (int index = 0; index < total; ++index) {
        const bool won = index < successes;
        result.episodes.push_back(
            eval::EpisodeOutcome{won ? core::LevelOutcome::Won : core::LevelOutcome::Lost,
                                 won ? stepsOnSuccess : stepsOnFailure});
    }

    eval::BenchmarkReport report;
    report.addResult(request.algorithmId.toStdString(),
                     std::filesystem::path(request.levelPath.toStdString()).stem().string(),
                     result);
    report.writeCsv(std::filesystem::path(csvPath.toStdString()));
    return std::filesystem::exists(std::filesystem::path(csvPath.toStdString()));
}

namespace {

/// Nom long ecrit dans les metadonnees du rejeu, meme table que `aisolver::cli` (`Commands.cpp`).
std::string longAlgorithmName(const std::string& algorithmId) {
    if (algorithmId == "pg") {
        return "reinforce";
    }
    if (algorithmId == "ac") {
        return "acteur-critique";
    }
    if (algorithmId == "avance") {
        return "dqn";
    }
    return "evolutionnaire";
}

}  // namespace

ReplayExportOutcome exportModelReplay(const EvaluationRequest& request, const QString& outputPath) {
    using namespace aisolver;

    // Voir evaluateModel : un rejeu headless journalise comme une partie reelle sans en etre une.
    const core::ScopedLogLevel quietDuringReplay(core::defaultLogger(), core::LogLevel::Warning);

    const std::filesystem::path model = request.modelPath.toStdString();
    const std::filesystem::path level = request.levelPath.toStdString();
    const std::string algorithmIdStd = request.algorithmId.toStdString();

    const std::size_t inputSize = ObservationEncoder().inputSize();
    const std::size_t hiddenSize = cli::hiddenSizeForModel(model);
    const training::evolutionary::NetworkTopology topology =
        training::evolutionary::policyTopology(inputSize, hiddenSize);
    Rng scratchRng(0);
    HeadlessLevelEnvironment environment;

    std::optional<training::DeterministicReplayResult> replay;
    if (algorithmIdStd == "avance") {
        training::QNetwork network(inputSize, hiddenSize, scratchRng);
        if (!nn::loadWeights(network.network(), model)) {
            return ReplayExportOutcome::Failed;
        }
        eval::AdvancedAlgorithmTrainedPolicy policy(network);
        replay = training::argmaxRollout(policy, environment, level);
    } else {
        std::unique_ptr<nn::Network> network =
            training::evolutionary::buildNetwork(topology, scratchRng);
        if (!nn::loadWeights(*network, model)) {
            return ReplayExportOutcome::Failed;
        }
        if (algorithmIdStd == "pg") {
            eval::ReinforceTrainedPolicy policy(*network);
            replay = training::argmaxRollout(policy, environment, level);
        } else if (algorithmIdStd == "ac") {
            eval::ActorCriticTrainedPolicy policy(*network);
            replay = training::argmaxRollout(policy, environment, level);
        } else {
            eval::EvolutionaryTrainedPolicy policy(*network);
            replay = training::argmaxRollout(policy, environment, level);
        }
    }

    if (!replay.has_value()) {
        return ReplayExportOutcome::Failed;
    }
    const bool solved = replay->status == EpisodeStatus::Won;
    const training::ReplayExportResult result = training::exportReplay(
        *replay, solved, level, std::filesystem::path(outputPath.toStdString()),
        longAlgorithmName(algorithmIdStd), request.seed, algorithmIdStd);
    if (result.exported) {
        return ReplayExportOutcome::Exported;
    }
    return result.error == training::ReplayExportError::NotSolved ? ReplayExportOutcome::NotSolved
                                                                  : ReplayExportOutcome::Failed;
}

}  // namespace hmi
