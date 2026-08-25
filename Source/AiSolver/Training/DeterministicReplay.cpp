// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/DeterministicReplay.h"

#include <limits>

#include "AiSolver/Env/ActionDecoding.h"
#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Env/Reward.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "Core/Diagnostics/Assert.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver::training {

DeterministicReplayResult replayBestIndividual(evolutionary::Individual& individual,
                                               HeadlessLevelEnvironment& environment,
                                               const std::filesystem::path& levelPath,
                                               int stuckThreshold) {
    if (!environment.reset(levelPath)) {
        // Voir `evaluateFitness` : l'assertion ne garde rien en Release. Un rejeu vide est refuse
        // par l'appelant, la ou un `step()` sur un monde vide serait indefini.
        PROJECTGAMING_ASSERT(false, "replayBestIndividual : le niveau doit se charger");
        return DeterministicReplayResult{};
    }

    const ObservationEncoder observationEncoder;
    const RewardConfig rewardConfig;

    // Etat de depart identique a FitnessEvaluator::evaluateFitness (LOT-ANNEXE-10) : meme
    // convention de reconstruction de l'etat de spawn avant le premier step().
    const core::GridPosition entry = environment.level().entry();
    core::Aabb previousBox = core::Aabb::fromTopLeftSize(
        core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
    core::Player playerState{};
    core::Velocity playerVelocity{};

    DeterministicReplayResult result;
    EpisodeStatus status = EpisodeStatus::Ongoing;

    ObjectiveDistanceFieldCache distanceFieldCache;

    while (status == EpisodeStatus::Ongoing && !environment.budgetExhausted()) {
        const Tensor<float> observationVector =
            observationEncoder.encode(environment, previousBox, playerState, playerVelocity);
        const autodiff::NodePtr inputNode = autodiff::variable(observationVector);
        const autodiff::NodePtr outputNode = individual.network().forward(inputNode);
        const Tensor<float> distribution = outputNode->value.view({actionCount()});
        const Action action = decodeArgmax(distribution);
        const core::PlayerInput input = toPlayerInput(action);
        result.steps.push_back(input);

        const StepObservation stepObservation = environment.step(input);
        // Le champ ne change qu'a l'ouverture ou la fermeture d'une porte : le cache
        // le reconstruit alors, et le rend tel quel sinon.
        const GridDistanceField& distanceField =
            distanceFieldCache.field(environment.level(), environment.mechanisms());
        result.finalReward += computeReward(rewardConfig, distanceField, previousBox,
                                            stepObservation.playerBox, stepObservation.outcome);

        previousBox = stepObservation.playerBox;
        playerState = stepObservation.playerState;
        playerVelocity = stepObservation.playerVelocity;

        status = classifyEpisode(stepObservation.outcome, stepObservation.stepIndex,
                                 environment.stepsSinceProgress(), std::numeric_limits<int>::max(),
                                 stuckThreshold);
    }
    if (status == EpisodeStatus::Ongoing) {
        // Budget dur atteint sans que classifyEpisode() (hardStepBudget desactive ci-dessus) ne
        // l'ait detecte -- meme convention que FitnessEvaluator::evaluateFitness.
        status = EpisodeStatus::TimedOut;
    }
    result.status = status;
    return result;
}

}  // namespace aisolver::training
