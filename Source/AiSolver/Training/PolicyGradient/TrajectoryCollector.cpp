// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/PolicyGradient/TrajectoryCollector.h"

#include <cmath>
#include <limits>

#include "AiSolver/Env/ActionDecoding.h"
#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Env/Reward.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "Core/Diagnostics/Assert.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver::training {

TrajectoryCollector::TrajectoryCollector(int stuckThreshold) : _stuckThreshold(stuckThreshold) {}

Trajectory TrajectoryCollector::collectEpisode(HeadlessLevelEnvironment& environment,
                                               nn::Network& policy, Rng& rng) const {
    PROJECTGAMING_ASSERT(environment.loaded(),
                         "TrajectoryCollector::collectEpisode : l'environnement doit deja etre "
                         "charge/reinitialise par l'appelant");

    const ObservationEncoder observationEncoder;
    const RewardConfig rewardConfig;

    // Boîte/état de départ : même convention que evolutionary::evaluateFitness (LOT-ANNEXE-10) --
    // HeadlessLevelEnvironment n'expose pas d'observation avant le premier step().
    const core::GridPosition entry = environment.level().entry();
    core::Aabb previousBox = core::Aabb::fromTopLeftSize(
        core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
    core::Player playerState{};
    core::Velocity playerVelocity{};

    Trajectory trajectory;
    EpisodeStatus status = EpisodeStatus::Ongoing;

    while (status == EpisodeStatus::Ongoing && !environment.budgetExhausted()) {
        Tensor<float> observationVector =
            observationEncoder.encode(environment, previousBox, playerState, playerVelocity);
        const autodiff::NodePtr inputNode = autodiff::variable(observationVector);
        const autodiff::NodePtr outputNode = policy.forward(inputNode);
        const Tensor<float> distribution = outputNode->value.view({actionCount()});

        // Toujours echantillonne (jamais argmax) : REINFORCE a besoin d'exploration pour que le
        // gradient de log-probabilite ait un sens (decision de cadrage de l'epic).
        const Action action = decodeStochastic(distribution, 1.0f, rng);
        const std::size_t actionIndex = indexOf(action);
        const float probability = distribution.data()[actionIndex];
        const float logProbability = std::log(probability);

        const StepObservation stepObservation = environment.step(toPlayerInput(action));
        // Reconstruit a chaque pas (LOT-ANNEXE-21) : l'ensemble des cibles change des qu'une porte
        // s'ouvre, la grille de collision (environment.mechanisms()) est la source de verite.
        const GridDistanceField distanceField =
            buildObjectiveDistanceField(environment.level(), environment.mechanisms());
        const float reward = computeReward(rewardConfig, distanceField, previousBox,
                                           stepObservation.playerBox, stepObservation.outcome);

        trajectory.steps.push_back(
            TrajectoryStep{observationVector, actionIndex, logProbability, reward});

        previousBox = stepObservation.playerBox;
        playerState = stepObservation.playerState;
        playerVelocity = stepObservation.playerVelocity;

        status = classifyEpisode(stepObservation.outcome, stepObservation.stepIndex,
                                 environment.stepsSinceProgress(), std::numeric_limits<int>::max(),
                                 _stuckThreshold);
    }
    if (status == EpisodeStatus::Ongoing) {
        // Budget dur atteint sans que classifyEpisode() (hardStepBudget volontairement desactive
        // ci-dessus) ne l'ait detecte -- meme convention que evaluateFitness.
        status = EpisodeStatus::TimedOut;
    }
    trajectory.status = status;
    return trajectory;
}

}  // namespace aisolver::training
