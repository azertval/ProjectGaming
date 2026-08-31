// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ActorCritic/ActorCriticTrainer.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Optim/OptimizerUtils.h"
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
      _levelName(std::move(levelName)),
      _collector(config.tuning.actionRepeat, config.tuning.explorationFloor) {
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

void ActorCriticTrainer::run(std::size_t episodeCount, bool updateCritic,
                             const std::function<bool()>& shouldStop) {
    const std::size_t batchSize = (std::max)(std::size_t{1}, _config.tuning.batchEpisodes);
    std::size_t played = 0;

    while (played < episodeCount) {
        // 1. Collecte du lot -- meme structure que `ReinforceTrainer::run`, a ceci pres que le
        //    critique fournit la ligne de base par etat AVANT le centrage-reduction : l'avantage
        //    est ce qui est normalise, jamais le retour brut, dont le critique a besoin intact.
        const std::size_t target = (std::min)(batchSize, episodeCount - played);
        std::vector<Trajectory> batch;
        std::vector<std::vector<float>> returnsPerEpisode;
        std::vector<std::vector<float>> advantages;
        std::vector<std::uint64_t> seeds;
        batch.reserve(target);
        returnsPerEpisode.reserve(target);
        advantages.reserve(target);
        seeds.reserve(target);

        bool stopped = false;
        for (std::size_t index = 0; index < target; ++index) {
            if (shouldStop && shouldStop()) {
                stopped = true;
                break;
            }
            const std::uint64_t seed = _config.seedBase +
                                       static_cast<std::uint64_t>(_episodeIndex) +
                                       static_cast<std::uint64_t>(index);
            Rng rng(seed);

            if (!_environment.reset(_levelPath)) {
                // Voir `evaluateFitness` : l'assertion ne garde rien en Release.
                PROJECTGAMING_ASSERT(false, "ActorCriticTrainer::run : le niveau doit se charger");
                return;
            }
            Trajectory trajectory = _collector.collectEpisode(_environment, _policy, rng);
            if (trajectory.steps.empty()) {
                continue;
            }
            std::vector<float> returns = computeReturns(trajectory, _config.gamma);
            advantages.push_back(computeAdvantages(returns, _critic, trajectory));
            returnsPerEpisode.push_back(std::move(returns));
            batch.push_back(std::move(trajectory));
            seeds.push_back(seed);
        }
        if (batch.empty()) {
            return;
        }

        const WeightStatistics statistics = weightStatistics(advantages);
        for (std::vector<float>& episodeAdvantages : advantages) {
            normalizeWeights(episodeAdvantages, statistics);
        }

        // 2. Deux graphes independants, deux backward() distincts (decision de cadrage de l'epic :
        //    jamais combines en un seul noeud scalaire final), chacun accumule sur tout le lot
        //    avant son unique pas d'optimiseur.
        const std::vector<autodiff::NodePtr> policyParameters = _policy.parameters();
        const std::vector<autodiff::NodePtr> criticParameters = _critic.parameters();
        const float batchScale = 1.0f / static_cast<float>(batch.size());

        std::vector<float> criticLosses;
        criticLosses.reserve(batch.size());
        for (std::size_t index = 0; index < batch.size(); ++index) {
            const autodiff::NodePtr policyLoss = autodiff::multiplyScalar(
                computeActorCriticLoss(_policy, batch[index], advantages[index],
                                       _config.tuning.entropyCoefficient),
                batchScale);
            autodiff::backward(policyLoss);

            const autodiff::NodePtr criticLoss =
                computeCriticLoss(_critic, batch[index], returnsPerEpisode[index]);
            criticLosses.push_back(criticLoss->value.data()[0]);
            if (updateCritic) {
                autodiff::backward(autodiff::multiplyScalar(criticLoss, batchScale));
            }
        }
        static_cast<void>(
            optim::clipGradientNorm(policyParameters, _config.tuning.gradientClipNorm));
        _policyOptimizer.step(policyParameters);
        _policyOptimizer.zeroGrad(policyParameters);
        if (updateCritic) {
            static_cast<void>(
                optim::clipGradientNorm(criticParameters, _config.tuning.gradientClipNorm));
            _criticOptimizer.step(criticParameters);
            _criticOptimizer.zeroGrad(criticParameters);
        }

        for (std::size_t index = 0; index < batch.size(); ++index) {
            float totalReward = 0.0f;
            for (const TrajectoryStep& step : batch[index].steps) {
                totalReward += step.reward;
            }

            TrainingStatsRow row;
            row.index = _episodeIndex;
            row.bestReward = totalReward;
            row.meanReward = totalReward;
            row.worstReward = totalReward;
            row.rewardStdDev = 0.0f;
            row.bestStepCount = static_cast<int>(batch[index].steps.size());
            row.successRate = batch[index].status == EpisodeStatus::Won ? 1.0f : 0.0f;
            row.seed = seeds[index];
            row.levelName = _levelName;
            _recorder.record(row);

            if (_criticLossCsv) {
                *_criticLossCsv << _episodeIndex << ',' << criticLosses[index] << '\n';
                _criticLossCsv->flush();
            }

            ++_episodeIndex;
        }
        played += batch.size();
        if (stopped) {
            return;
        }
    }
}

}  // namespace aisolver::training
