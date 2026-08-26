// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradient/ReinforceTrainer.h"

#include <vector>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Math/Autodiff/Node.h"
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
      _levelName(std::move(levelName)) {}

void ReinforceTrainer::run(std::size_t episodeCount, const std::function<bool()>& shouldStop) {
    for (std::size_t i = 0; i < episodeCount; ++i) {
        if (shouldStop && shouldStop()) {
            return;
        }
        const std::uint64_t seed = _config.seedBase + static_cast<std::uint64_t>(_episodeIndex);
        Rng rng(seed);

        if (!_environment.reset(_levelPath)) {
            // Voir `evaluateFitness` : l'assertion ne garde rien en Release. L'entrainement
            // s'arrete, plutot que de tourner a vide sur un monde inexistant.
            PROJECTGAMING_ASSERT(false, "ReinforceTrainer::run : le niveau doit se charger");
            return;
        }

        // Collecte (poids figes pendant tout l'episode) -> retours -> perte (memes poids,
        // aucune mise a jour n'intervient entre les deux) -> backward() -> step() -> zeroGrad
        // (decision de cadrage de l'epic : ordre strict, une seule passe d'optimisation).
        const Trajectory trajectory = _collector.collectEpisode(_environment, _policy, rng);
        const std::vector<float> returns = computeReturns(trajectory, _config.gamma);
        const autodiff::NodePtr loss = computeReinforceLoss(_policy, trajectory, returns);

        const std::vector<autodiff::NodePtr> parameters = _policy.parameters();
        autodiff::backward(loss);
        _optimizer.step(parameters);
        _optimizer.zeroGrad(parameters);

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

        ++_episodeIndex;
    }
}

}  // namespace aisolver::training
