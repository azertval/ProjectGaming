// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Cli/Commands.h"

#include <iostream>
#include <limits>
#include <memory>

#include "AiSolver/Cli/ArgParsing.h"
#include "AiSolver/Cli/TrainingConfig.h"
#include "AiSolver/Env/ActionDecoding.h"
#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Env/Reward.h"
#include "AiSolver/Eval/ActionDecodingMode.h"
#include "AiSolver/Eval/ActorCriticTrainedPolicy.h"
#include "AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h"
#include "AiSolver/Eval/BenchmarkConfig.h"
#include "AiSolver/Eval/BenchmarkReport.h"
#include "AiSolver/Eval/BenchmarkResult.h"
#include "AiSolver/Eval/BenchmarkRunner.h"
#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"
#include "AiSolver/Eval/ReinforceTrainedPolicy.h"
#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/Serialization.h"
#include "AiSolver/Optim/Adam.h"
#include "AiSolver/Optim/Sgd.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/ActorCritic/ActorCriticTrainer.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/Advanced/DqnTrainer.h"
#include "AiSolver/Training/Advanced/QNetwork.h"
#include "AiSolver/Training/ArgmaxRollout.h"
#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/ReinforceTrainer.h"
#include "AiSolver/Training/ReplayExport.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver::cli {

namespace {

std::unique_ptr<optim::IOptimizer> makeOptimizer(const std::string& name, float learningRate) {
    if (name == "adam") {
        return std::make_unique<optim::Adam>(learningRate);
    }
    return std::make_unique<optim::Sgd>(learningRate);
}

/// Nom long (`ReplayFile::algorithmName`) et identifiant court (`ReplayFile::algorithmId`, même
/// valeur que `--algo`) associés à chaque valeur acceptée par `--algo`.
struct AlgorithmLabels {
    std::string longName;
    std::string shortId;
};

[[nodiscard]] AlgorithmLabels labelsFor(const std::string& algo) {
    if (algo == "pg") {
        return {"reinforce", "pg"};
    }
    if (algo == "ac") {
        return {"acteur-critique", "ac"};
    }
    if (algo == "avance") {
        return {"dqn", "avance"};
    }
    return {"evolutionnaire", "evo"};
}

}  // namespace

bool isKnownAlgorithm(const std::string& algo) {
    return algo == "evo" || algo == "pg" || algo == "ac" || algo == "avance";
}

std::optional<TrainArgs> parseTrainArgs(const std::vector<std::string>& args, std::string& error) {
    const std::optional<std::string> level = findOption(args, "--level");
    const std::optional<std::string> algo = findOption(args, "--algo");
    if (!level.has_value()) {
        error = "train : --level est requis";
        return std::nullopt;
    }
    if (!algo.has_value()) {
        error = "train : --algo est requis (evo|pg|ac|avance)";
        return std::nullopt;
    }
    if (!isKnownAlgorithm(*algo)) {
        error = "train : --algo invalide '" + *algo + "' (attendu : evo, pg, ac ou avance)";
        return std::nullopt;
    }

    TrainArgs result;
    result.level = *level;
    result.algo = *algo;
    if (const std::optional<std::string> seed = findOption(args, "--seed"); seed.has_value()) {
        result.seed = std::stoull(*seed);
    }
    if (const std::optional<std::string> config = findOption(args, "--config");
        config.has_value()) {
        result.configFile = std::filesystem::path(*config);
    }
    if (const std::optional<std::string> runsRoot = findOption(args, "--runs-root");
        runsRoot.has_value()) {
        result.runsRoot = *runsRoot;
    }
    if (const std::optional<std::string> value = findOption(args, "--population-size");
        value.has_value()) {
        result.populationSize = static_cast<std::size_t>(std::stoull(*value));
    }
    if (const std::optional<std::string> value = findOption(args, "--mutation-rate");
        value.has_value()) {
        result.mutationRate = std::stof(*value);
    }
    if (const std::optional<std::string> value = findOption(args, "--episodes");
        value.has_value()) {
        result.episodes = static_cast<std::size_t>(std::stoull(*value));
    }
    if (const std::optional<std::string> value = findOption(args, "--learning-rate");
        value.has_value()) {
        result.learningRate = std::stof(*value);
    }
    if (const std::optional<std::string> value = findOption(args, "--gamma"); value.has_value()) {
        result.gamma = std::stof(*value);
    }
    if (const std::optional<std::string> value = findOption(args, "--optimizer");
        value.has_value()) {
        result.optimizer = *value;
    }
    return result;
}

std::optional<EvaluateArgs> parseEvaluateArgs(const std::vector<std::string>& args,
                                              std::string& error) {
    const std::optional<std::string> model = findOption(args, "--model");
    const std::optional<std::string> algo = findOption(args, "--algo");
    const std::optional<std::string> level = findOption(args, "--level");
    if (!model.has_value()) {
        error = "evaluate : --model est requis";
        return std::nullopt;
    }
    if (!algo.has_value()) {
        error = "evaluate : --algo est requis (evo|pg|ac|avance)";
        return std::nullopt;
    }
    if (!isKnownAlgorithm(*algo)) {
        error = "evaluate : --algo invalide '" + *algo + "' (attendu : evo, pg, ac ou avance)";
        return std::nullopt;
    }
    if (!level.has_value()) {
        error = "evaluate : --level est requis";
        return std::nullopt;
    }

    EvaluateArgs result;
    result.model = *model;
    result.algo = *algo;
    result.level = *level;
    if (const std::optional<std::string> repetitions = findOption(args, "--repetitions");
        repetitions.has_value()) {
        result.repetitions = std::stoi(*repetitions);
    }
    if (const std::optional<std::string> report = findOption(args, "--report");
        report.has_value()) {
        result.report = std::filesystem::path(*report);
    }
    return result;
}

std::optional<ExportReplayArgs> parseExportReplayArgs(const std::vector<std::string>& args,
                                                      std::string& error) {
    const std::optional<std::string> model = findOption(args, "--model");
    const std::optional<std::string> algo = findOption(args, "--algo");
    const std::optional<std::string> level = findOption(args, "--level");
    const std::optional<std::string> output = findOption(args, "--output");
    if (!model.has_value()) {
        error = "export-replay : --model est requis";
        return std::nullopt;
    }
    if (!algo.has_value()) {
        error = "export-replay : --algo est requis (evo|pg|ac|avance)";
        return std::nullopt;
    }
    if (!isKnownAlgorithm(*algo)) {
        error = "export-replay : --algo invalide '" + *algo + "' (attendu : evo, pg, ac ou avance)";
        return std::nullopt;
    }
    if (!level.has_value()) {
        error = "export-replay : --level est requis";
        return std::nullopt;
    }
    if (!output.has_value()) {
        error = "export-replay : --output est requis";
        return std::nullopt;
    }

    ExportReplayArgs result;
    result.model = *model;
    result.algo = *algo;
    result.level = *level;
    result.output = *output;
    if (const std::optional<std::string> seed = findOption(args, "--seed"); seed.has_value()) {
        result.seed = std::stoull(*seed);
    }
    return result;
}

int runTrain(const TrainArgs& args) {
    if (!std::filesystem::exists(args.level)) {
        std::cerr << "train : niveau introuvable : " << args.level << "\n";
        return 1;
    }

    const CommandLineOverrides overrides{
        args.populationSize, args.mutationRate, args.episodes,
        args.learningRate,   args.gamma,        args.optimizer,
    };
    const TrainingConfig config = loadTrainingConfig(args.configFile, overrides);

    const std::string levelName = args.level.stem().string();
    const std::string runId = generateRunId();
    const std::filesystem::path statsPath = makeTrainingRunPath(args.runsRoot, levelName, runId);
    const std::filesystem::path runDir = statsPath.parent_path();
    const std::filesystem::path modelPath = runDir / "model.bin";
    const std::filesystem::path replayPath = runDir / "replay.json";
    const std::filesystem::path configPath = runDir / "config.json";

    if (!writeTrainingConfigJson(config, configPath)) {
        std::cerr << "train : impossible d'ecrire la configuration resolue : " << configPath
                  << "\n";
        return 1;
    }

    const std::size_t inputSize = ObservationEncoder().inputSize();
    const AlgorithmLabels labels = labelsFor(args.algo);
    bool solved = false;

    if (args.algo == "evo") {
        const training::evolutionary::NetworkTopology topology =
            training::evolutionary::policyTopology(inputSize, config.hiddenSize);
        const training::TrainAndExportOutcome outcome =
            training::trainLevelAndExportReplay(args.level, topology, config.evolutionary,
                                                config.stopping, args.seed, statsPath, replayPath);
        solved = outcome.trainingResult.solved;
        if (!nn::saveWeights(outcome.trainingResult.bestIndividual.network(), modelPath)) {
            std::cerr << "train : echec de sauvegarde du modele : " << modelPath << "\n";
            return 1;
        }
    } else if (args.algo == "pg") {
        Rng policyRng(args.seed);
        const auto topology = training::evolutionary::policyTopology(inputSize, config.hiddenSize);
        const std::unique_ptr<nn::Network> policy =
            training::evolutionary::buildNetwork(topology, policyRng);
        const std::unique_ptr<optim::IOptimizer> optimizer =
            makeOptimizer(config.optimizer, config.learningRate);
        HeadlessLevelEnvironment environment;
        TrainingStatsRecorder recorder(statsPath);
        training::ReinforceConfig reinforceConfig;
        reinforceConfig.gamma = config.gamma;
        reinforceConfig.seedBase = args.seed;
        training::ReinforceTrainer trainer(*policy, *optimizer, environment, args.level,
                                           reinforceConfig, recorder, levelName);
        trainer.run(config.episodes);

        if (!nn::saveWeights(*policy, modelPath)) {
            std::cerr << "train : echec de sauvegarde du modele : " << modelPath << "\n";
            return 1;
        }
        eval::ReinforceTrainedPolicy evalPolicy(*policy);
        HeadlessLevelEnvironment rolloutEnvironment;
        const std::optional<training::DeterministicReplayResult> replay =
            training::argmaxRollout(evalPolicy, rolloutEnvironment, args.level);
        if (replay.has_value()) {
            solved = replay->status == EpisodeStatus::Won;
            const training::ReplayExportResult exportResult =
                training::exportReplay(*replay, solved, args.level, replayPath, labels.longName,
                                       args.seed, labels.shortId);
            solved = solved && exportResult.exported;
        }
    } else if (args.algo == "ac") {
        Rng policyRng(args.seed);
        Rng criticRng(args.seed + 900);
        const auto topology = training::evolutionary::policyTopology(inputSize, config.hiddenSize);
        const std::unique_ptr<nn::Network> policy =
            training::evolutionary::buildNetwork(topology, policyRng);
        training::CriticNetwork critic(inputSize, config.hiddenSize, criticRng);
        const std::unique_ptr<optim::IOptimizer> policyOptimizer =
            makeOptimizer(config.optimizer, config.learningRate);
        const std::unique_ptr<optim::IOptimizer> criticOptimizer =
            makeOptimizer(config.optimizer, config.learningRate);
        HeadlessLevelEnvironment environment;
        TrainingStatsRecorder recorder(statsPath);
        training::ActorCriticConfig actorCriticConfig;
        actorCriticConfig.gamma = config.gamma;
        actorCriticConfig.seedBase = args.seed;
        training::ActorCriticTrainer trainer(*policy, *policyOptimizer, critic, *criticOptimizer,
                                             environment, args.level, actorCriticConfig, recorder,
                                             levelName);
        trainer.run(config.episodes);

        if (!nn::saveWeights(*policy, modelPath)) {
            std::cerr << "train : echec de sauvegarde du modele : " << modelPath << "\n";
            return 1;
        }
        eval::ActorCriticTrainedPolicy evalPolicy(*policy);
        HeadlessLevelEnvironment rolloutEnvironment;
        const std::optional<training::DeterministicReplayResult> replay =
            training::argmaxRollout(evalPolicy, rolloutEnvironment, args.level);
        if (replay.has_value()) {
            solved = replay->status == EpisodeStatus::Won;
            const training::ReplayExportResult exportResult =
                training::exportReplay(*replay, solved, args.level, replayPath, labels.longName,
                                       args.seed, labels.shortId);
            solved = solved && exportResult.exported;
        }
    } else {
        Rng mainRng(args.seed);
        Rng targetRng(args.seed + 1700);
        training::QNetwork mainNetwork(inputSize, config.hiddenSize, mainRng);
        training::QNetwork targetNetwork(inputSize, config.hiddenSize, targetRng);
        const std::unique_ptr<optim::IOptimizer> optimizer =
            makeOptimizer(config.optimizer, config.learningRate);
        HeadlessLevelEnvironment environment;
        TrainingStatsRecorder recorder(statsPath);
        training::DqnConfig dqnConfig;
        dqnConfig.hiddenSize = config.hiddenSize;
        dqnConfig.replayCapacity = config.dqnReplayCapacity;
        dqnConfig.batchSize = config.dqnBatchSize;
        dqnConfig.warmupSize = config.dqnWarmupSize;
        dqnConfig.updatePeriodSteps = config.dqnUpdatePeriodSteps;
        dqnConfig.targetSyncPeriodSteps = config.dqnTargetSyncPeriodSteps;
        dqnConfig.gamma = config.gamma;
        dqnConfig.epsilonStart = config.dqnEpsilonStart;
        dqnConfig.epsilonEnd = config.dqnEpsilonEnd;
        dqnConfig.epsilonDecaySteps = config.dqnEpsilonDecaySteps;
        dqnConfig.seedBase = args.seed;
        training::DqnTrainer trainer(mainNetwork, targetNetwork, *optimizer, environment,
                                     args.level, dqnConfig, recorder, levelName);
        trainer.run(config.episodes);

        if (!nn::saveWeights(mainNetwork.network(), modelPath)) {
            std::cerr << "train : echec de sauvegarde du modele : " << modelPath << "\n";
            return 1;
        }
        eval::AdvancedAlgorithmTrainedPolicy evalPolicy(mainNetwork);
        HeadlessLevelEnvironment rolloutEnvironment;
        const std::optional<training::DeterministicReplayResult> replay =
            training::argmaxRollout(evalPolicy, rolloutEnvironment, args.level);
        if (replay.has_value()) {
            solved = replay->status == EpisodeStatus::Won;
            const training::ReplayExportResult exportResult =
                training::exportReplay(*replay, solved, args.level, replayPath, labels.longName,
                                       args.seed, labels.shortId);
            solved = solved && exportResult.exported;
        }
    }

    std::cout << "train : termine (" << args.algo << ", niveau " << levelName
              << ", resolu=" << (solved ? "oui" : "non") << ")\n"
              << "  stats : " << statsPath << "\n"
              << "  modele : " << modelPath << "\n"
              << "  config : " << configPath << "\n";
    if (solved) {
        std::cout << "  rejeu : " << replayPath << "\n";
    }
    return 0;
}

int runEvaluate(const EvaluateArgs& args) {
    const std::size_t inputSize = ObservationEncoder().inputSize();
    const training::evolutionary::NetworkTopology topology = training::evolutionary::policyTopology(
        inputSize, training::evolutionary::DEFAULT_HIDDEN_SIZE);
    Rng scratchRng(0);

    eval::BenchmarkConfig config;
    config.repetitions = args.repetitions;
    config.decodingMode = eval::ActionDecodingMode::Argmax;

    eval::BenchmarkResult result;
    if (args.algo == "avance") {
        training::QNetwork network(inputSize, training::QNetwork::kDefaultHiddenSize, scratchRng);
        if (!nn::loadWeights(network.network(), args.model)) {
            std::cerr << "evaluate : impossible de charger le modele : " << args.model << "\n";
            return 1;
        }
        eval::AdvancedAlgorithmTrainedPolicy policy(network);
        result = eval::BenchmarkRunner::run(policy, args.level, config);
    } else {
        std::unique_ptr<nn::Network> network =
            training::evolutionary::buildNetwork(topology, scratchRng);
        if (!nn::loadWeights(*network, args.model)) {
            std::cerr << "evaluate : impossible de charger le modele : " << args.model << "\n";
            return 1;
        }
        if (args.algo == "pg") {
            eval::ReinforceTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, args.level, config);
        } else if (args.algo == "ac") {
            eval::ActorCriticTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, args.level, config);
        } else {
            eval::EvolutionaryTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, args.level, config);
        }
    }

    std::cout << "evaluate : taux de reussite=" << result.successRate()
              << " pas moyen (tous)=" << result.meanStepsAll()
              << " pas moyen (reussis)=" << result.meanStepsOnSuccess()
              << " variance=" << result.stepVariance() << "\n";

    if (args.report.has_value()) {
        eval::BenchmarkReport report;
        report.addResult(labelsFor(args.algo).longName, args.level.stem().string(), result);
        report.writeCsv(*args.report);
    }
    return 0;
}

int runExportReplay(const ExportReplayArgs& args) {
    const std::size_t inputSize = ObservationEncoder().inputSize();
    const training::evolutionary::NetworkTopology topology = training::evolutionary::policyTopology(
        inputSize, training::evolutionary::DEFAULT_HIDDEN_SIZE);
    Rng scratchRng(0);
    const AlgorithmLabels labels = labelsFor(args.algo);
    HeadlessLevelEnvironment environment;

    std::optional<training::DeterministicReplayResult> replay;
    if (args.algo == "avance") {
        training::QNetwork network(inputSize, training::QNetwork::kDefaultHiddenSize, scratchRng);
        if (!nn::loadWeights(network.network(), args.model)) {
            std::cerr << "export-replay : impossible de charger le modele : " << args.model << "\n";
            return 1;
        }
        eval::AdvancedAlgorithmTrainedPolicy policy(network);
        replay = training::argmaxRollout(policy, environment, args.level);
    } else {
        std::unique_ptr<nn::Network> network =
            training::evolutionary::buildNetwork(topology, scratchRng);
        if (!nn::loadWeights(*network, args.model)) {
            std::cerr << "export-replay : impossible de charger le modele : " << args.model << "\n";
            return 1;
        }
        if (args.algo == "pg") {
            eval::ReinforceTrainedPolicy policy(*network);
            replay = training::argmaxRollout(policy, environment, args.level);
        } else if (args.algo == "ac") {
            eval::ActorCriticTrainedPolicy policy(*network);
            replay = training::argmaxRollout(policy, environment, args.level);
        } else {
            eval::EvolutionaryTrainedPolicy policy(*network);
            replay = training::argmaxRollout(policy, environment, args.level);
        }
    }

    if (!replay.has_value()) {
        std::cerr << "export-replay : niveau introuvable ou illisible : " << args.level << "\n";
        return 1;
    }

    const bool solved = replay->status == EpisodeStatus::Won;
    const training::ReplayExportResult exportResult = training::exportReplay(
        *replay, solved, args.level, args.output, labels.longName, args.seed, labels.shortId);
    if (!exportResult.exported) {
        if (exportResult.error == training::ReplayExportError::NotSolved) {
            std::cerr << "export-replay : le modele ne resout pas ce niveau (statut="
                      << static_cast<int>(replay->status) << "), aucun rejeu ecrit\n";
        } else {
            std::cerr << "export-replay : echec d'ecriture : " << args.output << "\n";
        }
        return 1;
    }

    std::cout << "export-replay : rejeu ecrit : " << args.output << "\n";
    return 0;
}

}  // namespace aisolver::cli
