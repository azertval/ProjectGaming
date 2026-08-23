// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ActorCritic/ActorCriticTrainer.h"

#include <vector>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Training/ActorCritic/ActorCriticLoss.h"
#include "AiSolver/Training/ActorCritic/AdvantageCalculator.h"
#include "AiSolver/Training/ActorCritic/CriticLoss.h"
#include "AiSolver/Training/PolicyGradient/ReturnCalculator.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

ActorCriticTrainer::ActorCriticTrainer(nn::Network& policy, optim::IOptimizer& policyOptimizer,
                                       CriticNetwork& critic, optim::IOptimizer& criticOptimizer,
                                       HeadlessLevelEnvironment& environment,
                                       std::filesystem::path levelPath, ActorCriticConfig config,
                                       TrainingStatsRecorder& recorder, std::string levelName,
                                       std::optional<std::filesystem::path> criticLossCsvPath)
    : _policy(policy),
      _policyOptimizer(policyOptimizer),
      _critic(critic),
      _criticOptimizer(criticOptimizer),
      _environment(environment),
      _levelPath(std::move(levelPath)),
      _config(config),
      _recorder(recorder),
      _levelName(std::move(levelName)) {
    if (criticLossCsvPath) {
        std::filesystem::create_directories(criticLossCsvPath->parent_path());
        _criticLossCsv.emplace(*criticLossCsvPath, std::ios::binary | std::ios::trunc);
        PROJECTGAMING_ASSERT(_criticLossCsv->is_open(),
                             "ActorCriticTrainer : impossible de creer le CSV de perte du "
                             "critique");
        *_criticLossCsv << "index,criticLoss\n";
        _criticLossCsv->flush();
    }
}

void ActorCriticTrainer::run(std::size_t episodeCount, bool updateCritic) {
    for (std::size_t i = 0; i < episodeCount; ++i) {
        const std::uint64_t seed = _config.seedBase + static_cast<std::uint64_t>(_episodeIndex);
        Rng rng(seed);

        const bool loaded = _environment.reset(_levelPath);
        PROJECTGAMING_ASSERT(loaded, "ActorCriticTrainer::run : le niveau doit se charger");

        const Trajectory trajectory = _collector.collectEpisode(_environment, _policy, rng);
        const std::vector<float> returns = computeReturns(trajectory, _config.gamma);
        const std::vector<float> advantages = computeAdvantages(returns, _critic, trajectory);

        // Deux graphes independants, deux backward() distincts (decision de cadrage de l'epic :
        // jamais combines en un seul noeud scalaire final).
        const autodiff::NodePtr policyLoss = computeActorCriticLoss(_policy, trajectory, advantages);
        const std::vector<autodiff::NodePtr> policyParameters = _policy.parameters();
        autodiff::backward(policyLoss);
        _policyOptimizer.step(policyParameters);
        _policyOptimizer.zeroGrad(policyParameters);

        const autodiff::NodePtr criticLoss = computeCriticLoss(_critic, trajectory, returns);
        const std::vector<autodiff::NodePtr> criticParameters = _critic.parameters();
        if (updateCritic) {
            autodiff::backward(criticLoss);
            _criticOptimizer.step(criticParameters);
            _criticOptimizer.zeroGrad(criticParameters);
        }

        float totalReward = 0.0f;
        for (const TrajectoryStep& step : trajectory.steps) {
            totalReward += step.reward;
        }

        TrainingStatsRow row;
        row.index = _episodeIndex;
        row.bestReward = totalReward;
        row.meanReward = totalReward;
        row.worstReward = totalReward;
        row.rewardStdDev = 0.0f;
        row.bestStepCount = static_cast<int>(trajectory.steps.size());
        row.successRate = trajectory.status == EpisodeStatus::Won ? 1.0f : 0.0f;
        row.seed = seed;
        row.levelName = _levelName;
        _recorder.record(row);

        if (_criticLossCsv) {
            *_criticLossCsv << _episodeIndex << ',' << criticLoss->value.data()[0] << '\n';
            _criticLossCsv->flush();
        }

        ++_episodeIndex;
    }
}

}  // namespace aisolver::training
