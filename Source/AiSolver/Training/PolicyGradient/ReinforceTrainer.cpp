// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradient/ReinforceTrainer.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Optim/OptimizerUtils.h"
#include "AiSolver/Training/PolicyGradient/ReinforceLoss.h"
#include "AiSolver/Training/PolicyGradient/ReturnCalculator.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

ReinforceTrainer::ReinforceTrainer(nn::Network& policy, optim::IOptimizer& optimizer,
                                   HeadlessLevelEnvironment& environment,
                                   std::filesystem::path levelPath, ReinforceConfig config,
                                   TrainingStatsRecorder& recorder, std::string levelName)
    : _policy(policy),
      _optimizer(optimizer),
      _environment(environment),
      _levelPath(std::move(levelPath)),
      _config(config),
      _recorder(recorder),
      _levelName(std::move(levelName)),
      _collector(config.tuning.actionRepeat, config.tuning.explorationFloor) {}

void ReinforceTrainer::run(std::size_t episodeCount, const std::function<bool()>& shouldStop) {
    const std::size_t batchSize = (std::max)(std::size_t{1}, _config.tuning.batchEpisodes);
    std::size_t played = 0;

    while (played < episodeCount) {
        // 1. Collecte du lot, poids figes d'un bout a l'autre : les `batchEpisodes` trajectoires
        //    sont toutes echantillonnees par la MEME politique, condition pour que leurs retours
        //    soient comparables entre eux (etape 2).
        const std::size_t target = (std::min)(batchSize, episodeCount - played);
        std::vector<Trajectory> batch;
        std::vector<std::vector<float>> weights;
        std::vector<std::uint64_t> seeds;
        batch.reserve(target);
        weights.reserve(target);
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
                // Voir `evaluateFitness` : l'assertion ne garde rien en Release. L'entrainement
                // s'arrete, plutot que de tourner a vide sur un monde inexistant.
                PROJECTGAMING_ASSERT(false, "ReinforceTrainer::run : le niveau doit se charger");
                return;
            }
            Trajectory trajectory = _collector.collectEpisode(_environment, _policy, rng);
            if (trajectory.steps.empty()) {
                continue;  // budget nul : rien a apprendre, et la perte exige un pas au moins.
            }
            weights.push_back(computeReturns(trajectory, _config.gamma));
            batch.push_back(std::move(trajectory));
            seeds.push_back(seed);
        }
        if (batch.empty()) {
            return;
        }

        // 2. Centrage-reduction des retours sur le lot entier : sans elle, un lot dont tous les
        //    retours sont negatifs ne produit que des mises a jour qui decouragent.
        const WeightStatistics statistics = weightStatistics(weights);
        for (std::vector<float>& episodeWeights : weights) {
            normalizeWeights(episodeWeights, statistics);
        }

        // 3. Un backward par episode, gradients ACCUMULES (aucun zeroGrad entre eux), puis un seul
        //    pas d'optimiseur : c'est ce qui fait du lot une seule estimation du gradient. La perte
        //    de chaque episode est divisee par la taille du lot pour que la norme du gradient ne
        //    depende pas d'elle.
        const std::vector<autodiff::NodePtr> parameters = _policy.parameters();
        const float batchScale = 1.0f / static_cast<float>(batch.size());
        for (std::size_t index = 0; index < batch.size(); ++index) {
            const autodiff::NodePtr loss =
                autodiff::multiplyScalar(computeReinforceLoss(_policy, batch[index], weights[index],
                                                              _config.tuning.entropyCoefficient),
                                         batchScale);
            autodiff::backward(loss);
        }
        static_cast<void>(optim::clipGradientNorm(parameters, _config.tuning.gradientClipNorm));
        _optimizer.step(parameters);
        _optimizer.zeroGrad(parameters);

        // 4. Journalisation : une ligne par episode, schema CSV strictement inchange (les
        //    comparateurs de la generation 4 lisent les runs des quatre algorithmes sans cas
        //    particulier).
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

            ++_episodeIndex;
        }
        played += batch.size();
        if (stopped) {
            return;
        }
    }
}

}  // namespace aisolver::training
