// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Ai/TrainingWorker.h"

#include <chrono>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>

#include "AiSolver/Cli/TrainingConfig.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/ActorCriticTrainedPolicy.h"
#include "AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h"
#include "AiSolver/Eval/ReinforceTrainedPolicy.h"
#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/Serialization.h"
#include "AiSolver/Optim/Adam.h"
#include "AiSolver/Optim/Sgd.h"
#include "AiSolver/Replay/LevelFingerprint.h"
#include "AiSolver/Replay/ReplayFile.h"
#include "AiSolver/Stats/TrainingRunPath.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/ActorCritic/ActorCriticTrainer.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/ArgmaxRollout.h"
#include "AiSolver/Training/DeterministicReplay.h"
#include "AiSolver/Training/Dqn/DqnTrainer.h"
#include "AiSolver/Training/Dqn/QNetwork.h"
#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"
#include "AiSolver/Training/PolicyGradient/ReinforceTrainer.h"
#include "AiSolver/Training/ReplayExport.h"

/**
 * @file HMI/Ai/TrainingWorker.cpp
 * @brief Voir `TrainingWorker.h` — dispatch d'algorithme calqué sur `aisolver::cli::runTrain`
 * (`Source/AiSolver/Cli/Commands.cpp`, `LOT-ANNEXE-19`), avec progression/interruption/aperçu.
 */

namespace hmi {

namespace {

using Clock = std::chrono::steady_clock;

// Periode minimale entre deux apercus (LOT-ANNEXE-21) : un rejeu deterministe complet plus une
// ecriture disque a chaque generation/episode ralentirait un niveau rapide pour rien -- la vue
// "Rejeu 3D" n'a besoin que d'un aperçu recent, pas de chaque generation.
constexpr auto kPreviewInterval = std::chrono::milliseconds(1500);

std::unique_ptr<aisolver::optim::IOptimizer> makeOptimizer(const QString& name,
                                                           float learningRate) {
    if (name.toStdString() == "adam") {
        return std::make_unique<aisolver::optim::Adam>(learningRate);
    }
    return std::make_unique<aisolver::optim::Sgd>(learningRate);
}

// Ecrit un apercu du champion courant (LOT-ANNEXE-21), sans passer par aisolver::training::
// exportReplay : ce dernier refuse d'ecrire tant que solved est faux (decision de cadrage de
// LOT-ANNEXE-11, un rejeu EXPORTE ne peut etre qu'une reussite validee) -- l'apercu, lui, montre
// la meilleure TENTATIVE courante, resolue ou non, jamais publie dans Elements/Replays.
bool writePreviewReplay(const aisolver::training::DeterministicReplayResult& replay,
                        const std::filesystem::path& levelPath,
                        const std::filesystem::path& outputPath, const std::string& algorithmName,
                        std::uint64_t seed, const std::string& algorithmId) {
    aisolver::ReplayFile file;
    file.levelPath = levelPath.filename().string();
    std::ifstream levelFile(levelPath, std::ios::binary);
    if (levelFile) {
        std::ostringstream contents;
        contents << levelFile.rdbuf();
        file.levelFingerprint = aisolver::computeLevelFingerprint(contents.str());
    }
    file.steps = replay.steps;
    file.algorithmName = algorithmName;
    file.seed = seed;
    file.finalReward = replay.finalReward;
    file.algorithmId = algorithmId;
    return aisolver::writeReplay(outputPath, file);
}

}  // namespace

TrainingWorker::TrainingWorker(TrainingRequest request, QObject* parent)
    : QObject(parent), _request(std::move(request)) {}

void TrainingWorker::requestStop() {
    _stopRequested.store(true);
}

void TrainingWorker::run() {
    using namespace aisolver;

    const std::filesystem::path levelPath = _request.levelPath.toStdString();
    if (!std::filesystem::exists(levelPath)) {
        emit failed(QStringLiteral("Niveau introuvable : %1").arg(_request.levelPath));
        return;
    }

    const cli::CommandLineOverrides overrides{_request.populationSize, _request.mutationRate,
                                              _request.episodes,       _request.learningRate,
                                              _request.gamma,          std::nullopt};
    cli::TrainingConfig config = cli::loadTrainingConfig(std::nullopt, overrides);
    config.algo = _request.algo.toStdString();
    if (!_request.optimizer.isEmpty()) {
        config.optimizer = _request.optimizer.toStdString();
    }

    const std::string levelName = levelPath.stem().string();
    const std::string runId = generateRunId();
    const std::filesystem::path runsRoot =
        _request.runsRoot.isEmpty() ? kDefaultTrainingRunsRoot
                                    : std::filesystem::path(_request.runsRoot.toStdString());
    const std::filesystem::path statsPath = makeTrainingRunPath(runsRoot, levelName, runId);
    const std::filesystem::path runDir = statsPath.parent_path();
    const std::filesystem::path modelPath = runDir / "model.bin";
    const std::filesystem::path replayPath = runDir / "replay.json";
    const std::filesystem::path configPath = runDir / "config.json";
    const std::filesystem::path previewsDir = runDir / "previews";

    // Un fichier par generation/episode (jamais ecrase) : le seul moyen pour l'IHM de proposer un
    // choix de generation dans "Voir en jeu" plutot que le seul apercu le plus recent.
    const auto previewPathFor = [&previewsDir](int generation) {
        return previewsDir / ("gen_" + std::to_string(generation) + ".json");
    };

    if (!cli::writeTrainingConfigJson(config, configPath)) {
        emit failed(QStringLiteral("Impossible d'ecrire la configuration : %1")
                        .arg(QString::fromStdString(configPath.string())));
        return;
    }

    const std::string algo = _request.algo.toStdString();
    const std::size_t inputSize = ObservationEncoder().inputSize();
    Clock::time_point lastPreview{};

    const auto shouldStop = [this] { return _stopRequested.load(); };

    // Un seul ecrivain par fichier de statistiques. Le chemin evolutionniste passe par
    // `LevelTrainingSession`, qui possede le sien ; les chemins par gradient construisent celui-ci
    // au moment ou ils en ont besoin. En construire un ici pour tout le monde ouvrirait un second
    // flux en troncature sur le meme chemin des que l'algorithme est evolutionniste.
    std::optional<TrainingStatsRecorder> recorder;

    // Aperçu périodique du champion courant (pg/ac/avance) : un rejeu Argmax déterministe complet
    // à CHAQUE épisode ralentirait un niveau rapide pour rien -- rythme borné par kPreviewInterval,
    // même raison que le chemin évolutionniste (`onGenerationChampion` ci-dessous).
    const auto maybeEmitPreview = [&](eval::TrainedPolicy& evalPolicy, const std::string& name,
                                      const std::string& id, int generation) {
        const Clock::time_point now = Clock::now();
        if (now - lastPreview < kPreviewInterval) {
            return;
        }
        lastPreview = now;
        HeadlessLevelEnvironment previewEnvironment;
        const std::optional<training::DeterministicReplayResult> replay =
            training::argmaxRollout(evalPolicy, previewEnvironment, levelPath);
        const std::filesystem::path previewPath = previewPathFor(generation);
        if (replay &&
            writePreviewReplay(*replay, levelPath, previewPath, name, _request.seed, id)) {
            emit previewReady(QString::fromStdString(previewPath.string()),
                              QString::fromStdString(id), _request.levelPath, generation);
        }
    };

    bool solved = false;
    std::string algorithmName = "evolutionnaire";
    std::string algorithmId = "evo";

    if (algo == "evo") {
        algorithmName = "evolutionnaire";
        algorithmId = "evo";
        const training::evolutionary::NetworkTopology topology =
            training::evolutionary::policyTopology(inputSize, config.hiddenSize);
        training::LevelTrainingSession session(levelPath, topology, config.evolutionary,
                                               config.stopping, _request.seed, statsPath);

        session.setOnStatsRow([this](const TrainingStatsRow& row) {
            emit progress(row.index, row.bestReward, row.meanReward, row.successRate);
        });

        int evoGeneration = 0;
        training::TrainingResult result =
            session.run(shouldStop, [&](const training::evolutionary::Individual& champion) {
                ++evoGeneration;
                const Clock::time_point now = Clock::now();
                if (now - lastPreview < kPreviewInterval) {
                    return;
                }
                lastPreview = now;
                HeadlessLevelEnvironment previewEnvironment;
                training::evolutionary::Individual scratch =
                    training::evolutionary::Individual([&] {
                        Rng scratchRng(0);
                        auto network = training::evolutionary::buildNetwork(topology, scratchRng);
                        const std::vector<autodiff::NodePtr> targetParameters =
                            network->parameters();
                        const std::vector<autodiff::NodePtr> sourceParameters =
                            champion.network().parameters();
                        for (std::size_t index = 0; index < targetParameters.size(); ++index) {
                            targetParameters[index]->value = sourceParameters[index]->value.clone();
                        }
                        return network;
                    }());
                const training::DeterministicReplayResult replay =
                    training::replayBestIndividual(scratch, previewEnvironment, levelPath);
                const std::filesystem::path previewPath = previewPathFor(evoGeneration);
                if (writePreviewReplay(replay, levelPath, previewPath, algorithmName, _request.seed,
                                       algorithmId)) {
                    emit previewReady(QString::fromStdString(previewPath.string()),
                                      QString::fromStdString(algorithmId), _request.levelPath,
                                      evoGeneration);
                }
            });
        solved = result.solved;
        if (!nn::saveWeights(result.bestIndividual.network(), modelPath)) {
            emit failed(QStringLiteral("Echec de sauvegarde du modele."));
            return;
        }
        HeadlessLevelEnvironment finalEnvironment;
        const training::DeterministicReplayResult finalReplay =
            training::replayBestIndividual(result.bestIndividual, finalEnvironment, levelPath);
        const training::ReplayExportResult exportResult = training::exportReplay(
            finalReplay, solved, levelPath, replayPath, algorithmName, _request.seed, algorithmId);
        solved = solved && exportResult.exported;
        emit finished(solved, QString::fromStdString(modelPath.string()),
                      QString::fromStdString(statsPath.string()),
                      QString::fromStdString(configPath.string()),
                      QString::fromStdString(replayPath.string()), exportResult.exported);
        return;
    }

    // Familles par gradient (pg/ac/avance) : reseau/optimiseur possedes localement, comme
    // aisolver::cli::runTrain -- meme construction, jamais dupliquee au-dela de ce dispatch.
    const auto topology = training::evolutionary::policyTopology(inputSize, config.hiddenSize);
    HeadlessLevelEnvironment environment;

    if (algo == "pg") {
        algorithmName = "reinforce";
        algorithmId = "pg";
        Rng policyRng(_request.seed);
        const std::unique_ptr<nn::Network> policy =
            training::evolutionary::buildNetwork(topology, policyRng);
        const std::unique_ptr<optim::IOptimizer> optimizer =
            makeOptimizer(_request.optimizer, config.learningRate);
        training::ReinforceConfig reinforceConfig;
        reinforceConfig.gamma = config.gamma;
        reinforceConfig.seedBase = _request.seed;
        // La politique d'evaluation enveloppe *policy PAR REFERENCE : construite avant
        // l'entrainement, elle reste valide tout du long (memes poids, mis a jour en place),
        // et sert donc a la fois a l'apercu periodique (pendant) et au rejeu final (apres).
        eval::ReinforceTrainedPolicy evalPolicy(*policy);
        recorder.emplace(statsPath);
        recorder->setOnRecord([this, &evalPolicy, &maybeEmitPreview, &algorithmName,
                               &algorithmId](const TrainingStatsRow& row) {
            emit progress(row.index, row.bestReward, row.meanReward, row.successRate);
            maybeEmitPreview(evalPolicy, algorithmName, algorithmId, row.index);
        });
        training::ReinforceTrainer trainer(*policy, *optimizer, environment, levelPath,
                                           reinforceConfig, *recorder, levelName);
        trainer.run(config.episodes, shouldStop);
        if (!nn::saveWeights(*policy, modelPath)) {
            emit failed(QStringLiteral("Echec de sauvegarde du modele."));
            return;
        }
        HeadlessLevelEnvironment rolloutEnvironment;
        const std::optional<training::DeterministicReplayResult> replay =
            training::argmaxRollout(evalPolicy, rolloutEnvironment, levelPath);
        if (replay) {
            solved = replay->status == EpisodeStatus::Won;
            const training::ReplayExportResult exportResult = training::exportReplay(
                *replay, solved, levelPath, replayPath, algorithmName, _request.seed, algorithmId);
            solved = solved && exportResult.exported;
        }
    } else if (algo == "ac") {
        algorithmName = "acteur-critique";
        algorithmId = "ac";
        Rng policyRng(_request.seed);
        Rng criticRng(_request.seed + 900);
        const std::unique_ptr<nn::Network> policy =
            training::evolutionary::buildNetwork(topology, policyRng);
        training::CriticNetwork critic(inputSize, config.hiddenSize, criticRng);
        const std::unique_ptr<optim::IOptimizer> policyOptimizer =
            makeOptimizer(_request.optimizer, config.learningRate);
        const std::unique_ptr<optim::IOptimizer> criticOptimizer =
            makeOptimizer(_request.optimizer, config.learningRate);
        training::ActorCriticConfig actorCriticConfig;
        actorCriticConfig.gamma = config.gamma;
        actorCriticConfig.seedBase = _request.seed;
        eval::ActorCriticTrainedPolicy evalPolicy(*policy);
        recorder.emplace(statsPath);
        recorder->setOnRecord([this, &evalPolicy, &maybeEmitPreview, &algorithmName,
                               &algorithmId](const TrainingStatsRow& row) {
            emit progress(row.index, row.bestReward, row.meanReward, row.successRate);
            maybeEmitPreview(evalPolicy, algorithmName, algorithmId, row.index);
        });
        training::ActorCriticTrainer trainer(*policy, *policyOptimizer, critic, *criticOptimizer,
                                             environment, levelPath, actorCriticConfig, *recorder,
                                             levelName);
        trainer.run(config.episodes, true, shouldStop);
        if (!nn::saveWeights(*policy, modelPath)) {
            emit failed(QStringLiteral("Echec de sauvegarde du modele."));
            return;
        }
        HeadlessLevelEnvironment rolloutEnvironment;
        const std::optional<training::DeterministicReplayResult> replay =
            training::argmaxRollout(evalPolicy, rolloutEnvironment, levelPath);
        if (replay) {
            solved = replay->status == EpisodeStatus::Won;
            const training::ReplayExportResult exportResult = training::exportReplay(
                *replay, solved, levelPath, replayPath, algorithmName, _request.seed, algorithmId);
            solved = solved && exportResult.exported;
        }
    } else {
        algorithmName = "dqn";
        algorithmId = "avance";
        Rng mainRng(_request.seed);
        Rng targetRng(_request.seed + 1700);
        training::QNetwork mainNetwork(inputSize, config.hiddenSize, mainRng);
        training::QNetwork targetNetwork(inputSize, config.hiddenSize, targetRng);
        const std::unique_ptr<optim::IOptimizer> optimizer =
            makeOptimizer(_request.optimizer, config.learningRate);
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
        dqnConfig.seedBase = _request.seed;
        eval::AdvancedAlgorithmTrainedPolicy evalPolicy(mainNetwork);
        recorder.emplace(statsPath);
        recorder->setOnRecord([this, &evalPolicy, &maybeEmitPreview, &algorithmName,
                               &algorithmId](const TrainingStatsRow& row) {
            emit progress(row.index, row.bestReward, row.meanReward, row.successRate);
            maybeEmitPreview(evalPolicy, algorithmName, algorithmId, row.index);
        });
        training::DqnTrainer trainer(mainNetwork, targetNetwork, *optimizer, environment, levelPath,
                                     dqnConfig, *recorder, levelName);
        trainer.run(config.episodes, shouldStop);
        if (!nn::saveWeights(mainNetwork.network(), modelPath)) {
            emit failed(QStringLiteral("Echec de sauvegarde du modele."));
            return;
        }
        HeadlessLevelEnvironment rolloutEnvironment;
        const std::optional<training::DeterministicReplayResult> replay =
            training::argmaxRollout(evalPolicy, rolloutEnvironment, levelPath);
        if (replay) {
            solved = replay->status == EpisodeStatus::Won;
            const training::ReplayExportResult exportResult = training::exportReplay(
                *replay, solved, levelPath, replayPath, algorithmName, _request.seed, algorithmId);
            solved = solved && exportResult.exported;
        }
    }

    emit finished(solved, QString::fromStdString(modelPath.string()),
                  QString::fromStdString(statsPath.string()),
                  QString::fromStdString(configPath.string()),
                  QString::fromStdString(replayPath.string()), solved);
}

}  // namespace hmi
