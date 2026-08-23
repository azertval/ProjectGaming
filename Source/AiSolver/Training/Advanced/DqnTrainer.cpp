// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Advanced/DqnTrainer.h"

#include <algorithm>
#include <limits>

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Env/Reward.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Training/Advanced/DqnLoss.h"
#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "Core/Diagnostics/Assert.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver::training {

namespace {

std::size_t argmaxIndex(const Tensor<float>& qValues) {
    const float* data = qValues.data();
    std::size_t best = 0;
    for (std::size_t index = 1; index < qValues.size(); ++index) {
        if (data[index] > data[best]) {
            best = index;
        }
    }
    return best;
}

}  // namespace

DqnTrainer::DqnTrainer(QNetwork& mainNetwork, QNetwork& targetNetwork, optim::IOptimizer& optimizer,
                       HeadlessLevelEnvironment& environment, std::filesystem::path levelPath,
                       DqnConfig config, TrainingStatsRecorder& recorder, std::string levelName,
                       std::optional<std::filesystem::path> dqnStatsCsvPath)
    : _mainNetwork(mainNetwork),
      _targetNetwork(targetNetwork),
      _optimizer(optimizer),
      _environment(environment),
      _levelPath(std::move(levelPath)),
      _config(config),
      _recorder(recorder),
      _levelName(std::move(levelName)),
      _replayBuffer(config.replayCapacity),
      _rng(config.seedBase) {
    if (dqnStatsCsvPath) {
        std::filesystem::create_directories(dqnStatsCsvPath->parent_path());
        _dqnStatsCsv.emplace(*dqnStatsCsvPath, std::ios::binary | std::ios::trunc);
        PROJECTGAMING_ASSERT(_dqnStatsCsv->is_open(),
                             "DqnTrainer : impossible de creer le CSV de statistiques DQN");
        *_dqnStatsCsv << "index,replayBufferSize,epsilon\n";
        _dqnStatsCsv->flush();
    }
}

float DqnTrainer::currentEpsilon() const noexcept {
    if (_totalSteps >= _config.epsilonDecaySteps) {
        return _config.epsilonEnd;
    }
    const float progress =
        static_cast<float>(_totalSteps) / static_cast<float>(_config.epsilonDecaySteps);
    return _config.epsilonStart + progress * (_config.epsilonEnd - _config.epsilonStart);
}

void DqnTrainer::run(std::size_t episodeCount) {
    const ObservationEncoder observationEncoder;
    const RewardConfig rewardConfig;

    for (std::size_t i = 0; i < episodeCount; ++i) {
        const bool loaded = _environment.reset(_levelPath);
        PROJECTGAMING_ASSERT(loaded, "DqnTrainer::run : le niveau doit se charger");

        // Boite/etat de depart : meme convention que TrajectoryCollector (LOT-ANNEXE-12) --
        // HeadlessLevelEnvironment n'expose pas d'observation avant le premier step().
        const core::GridPosition entry = _environment.level().entry();
        core::Aabb previousBox = core::Aabb::fromTopLeftSize(
            core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
        core::Player playerState{};
        core::Velocity playerVelocity{};

        EpisodeStatus status = EpisodeStatus::Ongoing;
        float totalReward = 0.0f;
        int stepCount = 0;

        while (status == EpisodeStatus::Ongoing && !_environment.budgetExhausted()) {
            Tensor<float> observationVector =
                observationEncoder.encode(_environment, previousBox, playerState, playerVelocity);

            // Exploration epsilon-greedy : remplace l'echantillonnage stochastique d'une
            // distribution de politique utilise par REINFORCE/acteur-critique (decision de cadrage
            // de l'epic, TACHE-01).
            const float epsilon = currentEpsilon();
            std::size_t actionIndex;
            if (_rng.nextFloat() < epsilon) {
                actionIndex = static_cast<std::size_t>(
                    _rng.nextInt(0, static_cast<int>(actionCount()) - 1));
            } else {
                const autodiff::NodePtr qValues = _mainNetwork.forward(observationVector);
                actionIndex = argmaxIndex(qValues->value);
            }
            const Action action = actionAt(actionIndex);

            const StepObservation stepObservation = _environment.step(toPlayerInput(action));
            const float reward = computeReward(rewardConfig, previousBox, stepObservation.playerBox,
                                               _environment.level().exit(), stepObservation.outcome);
            totalReward += reward;
            ++stepCount;

            previousBox = stepObservation.playerBox;
            playerState = stepObservation.playerState;
            playerVelocity = stepObservation.playerVelocity;

            status = classifyEpisode(stepObservation.outcome, stepObservation.stepIndex,
                                     _environment.stepsSinceProgress(),
                                     std::numeric_limits<int>::max(),
                                     evolutionary::DEFAULT_STUCK_THRESHOLD);

            Tensor<float> nextObservationVector =
                observationEncoder.encode(_environment, previousBox, playerState, playerVelocity);

            Transition transition;
            transition.observation = std::move(observationVector);
            transition.actionIndex = actionIndex;
            transition.reward = reward;
            transition.nextObservation = std::move(nextObservationVector);
            transition.done = status != EpisodeStatus::Ongoing;
            _replayBuffer.push(std::move(transition));

            ++_totalSteps;

            if (_replayBuffer.size() >= _config.warmupSize &&
                _totalSteps % _config.updatePeriodSteps == 0) {
                const std::vector<Transition> batch =
                    _replayBuffer.sample(_config.batchSize, _rng);
                const autodiff::NodePtr loss =
                    computeDqnLoss(_mainNetwork, _targetNetwork, batch, _config.gamma);
                const std::vector<autodiff::NodePtr> parameters = _mainNetwork.parameters();
                autodiff::backward(loss);
                _optimizer.step(parameters);
                _optimizer.zeroGrad(parameters);
            }

            if (_totalSteps % _config.targetSyncPeriodSteps == 0) {
                _targetNetwork.copyWeightsFrom(_mainNetwork);
            }
        }
        if (status == EpisodeStatus::Ongoing) {
            // Budget dur atteint sans que classifyEpisode() ne l'ait detecte -- meme convention
            // que TrajectoryCollector.
            status = EpisodeStatus::TimedOut;
        }

        TrainingStatsRow row;
        row.index = _episodeIndex;
        row.bestReward = totalReward;
        row.meanReward = totalReward;
        row.worstReward = totalReward;
        row.rewardStdDev = 0.0f;
        row.bestStepCount = stepCount;
        row.successRate = status == EpisodeStatus::Won ? 1.0f : 0.0f;
        row.seed = _config.seedBase;
        row.levelName = _levelName;
        _recorder.record(row);

        if (_dqnStatsCsv) {
            *_dqnStatsCsv << _episodeIndex << ',' << _replayBuffer.size() << ','
                          << currentEpsilon() << '\n';
            _dqnStatsCsv->flush();
        }

        ++_episodeIndex;
    }
}

}  // namespace aisolver::training
