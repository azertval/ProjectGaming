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
#include "Core/Diagnostics/ScopedLogLevel.h"
#include "HMI/Ai/TrainingOverrides.h"

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
constexpr auto PREVIEW_INTERVAL = std::chrono::milliseconds(1500);

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

// Recopie integrale de la ligne journalisee : les champs propres a un algorithme (epsilon,
// stabilite) restent absents ici et sont renseignes par la branche qui les connait.
TrainingProgress toProgress(const aisolver::TrainingStatsRow& row,
                            const aisolver::TrainingStatsDerived& derived) {
    TrainingProgress step;
    step.index = row.index;
    step.movingAverageReward = derived.movingAverageReward;
    step.rewardDelta = derived.rewardDelta;
    step.bestReward = row.bestReward;
    step.meanReward = row.meanReward;
    step.worstReward = row.worstReward;
    step.rewardStdDev = row.rewardStdDev;
    step.bestStepCount = row.bestStepCount;
    step.successRate = row.successRate;
    step.seed = row.seed;
    return step;
}

}  // namespace

TrainingWorker::TrainingWorker(TrainingRequest request, QObject* parent)
    : QObject(parent), _request(std::move(request)) {
    qRegisterMetaType<TrainingProgress>();
}

void TrainingWorker::requestStop() {
    _stopRequested.store(true);
}

void TrainingWorker::run() {
    using namespace aisolver;

    // HeadlessLevelEnvironment (chargement de niveau, mecanismes...) journalise a des niveaux
    // Trace/Info concus pour une partie reelle -- un entrainement le rejoue des milliers de fois
    // (une fois par individu/episode), ce qui produit un volume de journalisation sans rapport
    // avec une partie jouee et ralentit l'entrainement pour rien (traces parasites signalees en
    // essai utilisateur, LOT-ANNEXE-21). Releve temporairement le niveau minimal du journaliseur
    // partage de l'application pour toute la duree du run, restaure automatiquement a la sortie
    // (RAII, tous les chemins de retour compris).
    const core::ScopedLogLevel quietDuringTraining(core::defaultLogger(), core::LogLevel::Warning);

    const std::filesystem::path levelPath = _request.levelPath.toStdString();
    if (!std::filesystem::exists(levelPath)) {
        emit failed(QStringLiteral("ai_mode.error_level_missing"), _request.levelPath);
        return;
    }

    const cli::CommandLineOverrides overrides = overridesFor(_request);
    cli::TrainingConfig config = cli::loadTrainingConfig(std::nullopt, overrides);
    config.algorithmId = _request.algorithmId.toStdString();
    if (!_request.optimizer.isEmpty()) {
        config.optimizer = _request.optimizer.toStdString();
    }

    const std::string levelName = levelPath.stem().string();
    const std::string runId = generateRunId();
    const std::filesystem::path runsRoot =
        _request.runsRoot.isEmpty() ? DEFAULT_TRAINING_RUNS_ROOT
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
        emit failed(QStringLiteral("ai_mode.error_config_write"),
                    QString::fromStdString(configPath.string()));
        return;
    }

    // Algorithme DEMANDE par l'ecran. Distinct de `algorithmId` plus bas, qui est l'etiquette
    // ecrite dans les metadonnees du run : l'un choisit la branche, l'autre decrit le resultat.
    const std::string requestedAlgorithm = _request.algorithmId.toStdString();
    const std::size_t inputSize = ObservationEncoder().inputSize();
    // Budget de pas et seuil de blocage de la configuration resolue -- derives du niveau quand ils
    // valent zero. Le meme objet sert a l'entrainement, aux apercus et au rejeu final : un apercu
    // produit sous un budget plus court que celui de l'entrainement serait tronque avant la fin du
    // niveau, et montrerait un echec la ou le modele reussit.
    const EnvironmentConfig environmentConfig{.maxSteps = config.maxSteps,
                                              .stuckThreshold = config.stuckThreshold};
    Clock::time_point lastPreview{};

    const auto shouldStop = [this] { return _stopRequested.load(); };

    // Un seul ecrivain par fichier de statistiques. Le chemin evolutionniste passe par
    // `LevelTrainingSession`, qui possede le sien ; les chemins par gradient construisent celui-ci
    // au moment ou ils en ont besoin. En construire un ici pour tout le monde ouvrirait un second
    // flux en troncature sur le meme chemin des que l'algorithme est evolutionniste.
    std::optional<TrainingStatsRecorder> recorder;

    // Aperçu périodique du champion courant (pg/ac/avance) : un rejeu Argmax déterministe complet
    // à CHAQUE épisode ralentirait un niveau rapide pour rien -- rythme borné par PREVIEW_INTERVAL,
    // même raison que le chemin évolutionniste (`onGenerationChampion` ci-dessous).
    const auto maybeEmitPreview = [&](eval::TrainedPolicy& evalPolicy, const std::string& name,
                                      const std::string& id, int generation) {
        const Clock::time_point now = Clock::now();
        if (now - lastPreview < PREVIEW_INTERVAL) {
            return;
        }
        lastPreview = now;
        HeadlessLevelEnvironment previewEnvironment(environmentConfig);
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
    // Episodes reellement joues par les chemins par gradient : `config.episodes` est le budget
    // DEMANDE, qu'une interruption ne consomme pas. Compte les lignes journalisees, la seule
    // mesure commune aux trois trainers.
    int stepsCompleted = 0;
    std::string algorithmName = "evolutionnaire";
    std::string algorithmId = "evo";

    if (requestedAlgorithm == "evo") {
        algorithmName = "evolutionnaire";
        algorithmId = "evo";
        const training::evolutionary::NetworkTopology topology =
            training::evolutionary::policyTopology(inputSize, config.hiddenSize);
        training::LevelTrainingSession session(levelPath, topology, config.evolutionary,
                                               config.stopping, _request.seed, statsPath,
                                               environmentConfig);

        // Les statistiques et la stabilite d'une meme generation arrivent par deux rappels
        // successifs (`LevelTrainingSession::run` journalise la generation, puis met a jour le
        // compteur). On retient donc la ligne et on n'emet qu'apres le second, plutot que
        // d'envoyer a l'ecran une ligne portant la stabilite de la generation precedente.
        std::optional<TrainingProgress> pendingStep;
        session.setOnStatsRow(
            [&](const TrainingStatsRow& row, const aisolver::TrainingStatsDerived& derived) {
                pendingStep = toProgress(row, derived);
            });
        session.setOnStabilityChanged([&](int consecutive, int required) {
            if (!pendingStep.has_value()) {
                return;
            }
            pendingStep->consecutiveStable = consecutive;
            pendingStep->requiredStable = required;
            emit progress(*pendingStep);
            pendingStep.reset();
        });

        int evoGeneration = 0;
        training::TrainingResult result =
            session.run(shouldStop, [&](const training::evolutionary::Individual& champion) {
                ++evoGeneration;
                const Clock::time_point now = Clock::now();
                if (now - lastPreview < PREVIEW_INTERVAL) {
                    return;
                }
                lastPreview = now;
                HeadlessLevelEnvironment previewEnvironment(environmentConfig);
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
            emit failed(QStringLiteral("ai_mode.error_model_save"),
                        QString::fromStdString(modelPath.string()));
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
                      QString::fromStdString(replayPath.string()), exportResult.exported,
                      static_cast<int>(result.generationsRun));
        return;
    }

    // Familles par gradient (pg/ac/avance) : reseau/optimiseur possedes localement, comme
    // aisolver::cli::runTrain -- meme construction, jamais dupliquee au-dela de ce dispatch.
    const auto topology = training::evolutionary::policyTopology(inputSize, config.hiddenSize);
    HeadlessLevelEnvironment environment(environmentConfig);

    if (requestedAlgorithm == "pg") {
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
        reinforceConfig.tuning = config.tuning;
        // La politique d'evaluation enveloppe *policy PAR REFERENCE : construite avant
        // l'entrainement, elle reste valide tout du long (memes poids, mis a jour en place),
        // et sert donc a la fois a l'apercu periodique (pendant) et au rejeu final (apres).
        eval::ReinforceTrainedPolicy evalPolicy(*policy);
        recorder.emplace(statsPath);
        recorder->setOnRecord(
            [this, &evalPolicy, &maybeEmitPreview, &algorithmName, &stepsCompleted, &algorithmId](
                const TrainingStatsRow& row, const aisolver::TrainingStatsDerived& derived) {
                stepsCompleted = row.index + 1;
                emit progress(toProgress(row, derived));
                maybeEmitPreview(evalPolicy, algorithmName, algorithmId, row.index);
            });
        training::ReinforceTrainer trainer(*policy, *optimizer, environment, levelPath,
                                           reinforceConfig, *recorder, levelName);
        trainer.run(config.episodes, shouldStop);
        if (!nn::saveWeights(*policy, modelPath)) {
            emit failed(QStringLiteral("ai_mode.error_model_save"),
                        QString::fromStdString(modelPath.string()));
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
    } else if (requestedAlgorithm == "ac") {
        algorithmName = "acteur-critique";
        algorithmId = "ac";
        Rng policyRng(_request.seed);
        Rng criticRng(_request.seed + 900);
        const std::unique_ptr<nn::Network> policy =
            training::evolutionary::buildNetwork(topology, policyRng);
        training::CriticNetwork critic(inputSize, config.hiddenSize, criticRng);
        const std::unique_ptr<optim::IOptimizer> policyOptimizer =
            makeOptimizer(_request.optimizer, config.learningRate);
        // Taux propre au critique : sa sortie doit couvrir l'amplitude des retours, pas celle
        // des logits d'une politique (voir `cli::TrainingConfig::criticLearningRate`).
        const std::unique_ptr<optim::IOptimizer> criticOptimizer =
            makeOptimizer(_request.optimizer, config.criticLearningRate);
        training::ActorCriticConfig actorCriticConfig;
        actorCriticConfig.gamma = config.gamma;
        actorCriticConfig.seedBase = _request.seed;
        actorCriticConfig.tuning = config.tuning;
        eval::ActorCriticTrainedPolicy evalPolicy(*policy);
        recorder.emplace(statsPath);
        recorder->setOnRecord(
            [this, &evalPolicy, &maybeEmitPreview, &algorithmName, &stepsCompleted, &algorithmId](
                const TrainingStatsRow& row, const aisolver::TrainingStatsDerived& derived) {
                stepsCompleted = row.index + 1;
                emit progress(toProgress(row, derived));
                maybeEmitPreview(evalPolicy, algorithmName, algorithmId, row.index);
            });
        training::ActorCriticTrainer trainer(*policy, *policyOptimizer, critic, *criticOptimizer,
                                             environment, levelPath, actorCriticConfig, *recorder,
                                             levelName);
        trainer.run(config.episodes, true, shouldStop);
        if (!nn::saveWeights(*policy, modelPath)) {
            emit failed(QStringLiteral("ai_mode.error_model_save"),
                        QString::fromStdString(modelPath.string()));
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
        dqnConfig.actionRepeat = config.tuning.actionRepeat;
        dqnConfig.seedBase = _request.seed;
        eval::AdvancedAlgorithmTrainedPolicy evalPolicy(mainNetwork);
        recorder.emplace(statsPath);
        // L'enregistreur doit etre entierement branche avant de construire le trainer, qui le
        // prend par reference et journalise des son premier episode ; le trainer, lui, n'existe
        // qu'apres. D'ou ce pointeur, renseigne juste apres la construction et lu seulement
        // pendant `run()` -- jamais nul quand le rappel se declenche.
        const training::DqnTrainer* runningTrainer = nullptr;
        recorder->setOnRecord([this, &evalPolicy, &maybeEmitPreview, &algorithmName,
                               &stepsCompleted, &algorithmId,
                               &runningTrainer](const TrainingStatsRow& row,
                                                const aisolver::TrainingStatsDerived& derived) {
            stepsCompleted = row.index + 1;
            TrainingProgress step = toProgress(row, derived);
            if (runningTrainer != nullptr) {
                step.epsilon = runningTrainer->currentEpsilon();
                // Pas de simulation CUMULES depuis le debut du run : c'est le compteur dont
                // depend la decroissance d'epsilon, et donc la seule facon de lire cette
                // decroissance autrement que comme un nombre qui baisse tout seul. Le trainer
                // l'exposait deja, aucun appelant ne le lisait.
                step.totalSteps = runningTrainer->totalSteps();
            }
            emit progress(step);
            maybeEmitPreview(evalPolicy, algorithmName, algorithmId, row.index);
        });
        // CSV secondaire (index,replayBufferSize,epsilon), meme fichier que `aisolver-cli train` :
        // les deux grandeurs qui expliquent la forme d'une courbe DQN n'ont pas de colonne dans le
        // CSV commun a tous les algorithmes.
        training::DqnTrainer trainer(mainNetwork, targetNetwork, *optimizer, environment, levelPath,
                                     dqnConfig, *recorder, levelName, runDir / "dqn_stats.csv");
        runningTrainer = &trainer;
        trainer.run(config.episodes, shouldStop);
        if (!nn::saveWeights(mainNetwork.network(), modelPath)) {
            emit failed(QStringLiteral("ai_mode.error_model_save"),
                        QString::fromStdString(modelPath.string()));
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
                  QString::fromStdString(replayPath.string()), solved, stepsCompleted);
}

}  // namespace hmi
