// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/ArgmaxRollout.h"

#include <limits>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Env/Reward.h"
#include "AiSolver/Eval/ActionDecodingMode.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver::training {

std::optional<DeterministicReplayResult> argmaxRollout(eval::TrainedPolicy& policy,
                                                       HeadlessLevelEnvironment& environment,
                                                       const std::filesystem::path& levelPath) {
    if (!environment.reset(levelPath)) {
        return std::nullopt;
    }

    const ObservationEncoder observationEncoder;
    const RewardConfig rewardConfig;
    Rng rng(0);  // Argmax ne consomme jamais rng ; instance jetable pour satisfaire la signature.

    const core::GridPosition entry = environment.level().entry();
    core::Aabb previousBox = core::Aabb::fromTopLeftSize(
        core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
    core::Player playerState{};
    core::Velocity playerVelocity{};

    DeterministicReplayResult result;
    EpisodeStatus status = EpisodeStatus::Ongoing;

    while (status == EpisodeStatus::Ongoing && !environment.budgetExhausted()) {
        const Tensor<float> observation =
            observationEncoder.encode(environment, previousBox, playerState, playerVelocity);
        const std::optional<core::PlayerInput> input =
            policy.selectAction(observation, eval::ActionDecodingMode::Argmax, rng);
        if (!input.has_value()) {
            // Politique n'acceptant pas Argmax : ne devrait jamais survenir (les quatre adaptateurs
            // de LOT-ANNEXE-15 acceptent tous Argmax), garde défensive plutôt qu'un plantage.
            return std::nullopt;
        }
        result.steps.push_back(*input);

        const StepObservation stepObservation = environment.step(*input);
        // Champ de l'environnement : une seule instance par episode, deja reconstruite par
        // `step()` quand elle a cesse d'etre valide -- et celle-la meme dont la detection de
        // blocage se sert.
        result.finalReward += computeReward(rewardConfig, environment.objectiveField(), previousBox,
                                            stepObservation.playerBox, stepObservation.outcome);

        previousBox = stepObservation.playerBox;
        playerState = stepObservation.playerState;
        playerVelocity = stepObservation.playerVelocity;

        status = classifyEpisode(stepObservation.outcome, stepObservation.stepIndex,
                                 environment.stepsSinceProgress(), std::numeric_limits<int>::max(),
                                 environment.stuckThreshold());
    }
    if (status == EpisodeStatus::Ongoing) {
        status = EpisodeStatus::TimedOut;
    }
    result.status = status;
    return result;
}

}  // namespace aisolver::training
