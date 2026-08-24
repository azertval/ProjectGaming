// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"

#include <limits>

#include "AiSolver/Env/ActionDecoding.h"
#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Env/Reward.h"
#include "AiSolver/Math/Autodiff/Node.h"
#include "Core/Diagnostics/Assert.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver::training::evolutionary {

FitnessEvaluation evaluateFitness(Individual& individual, HeadlessLevelEnvironment& environment,
                                  const std::filesystem::path& levelPath, int stuckThreshold) {
    [[maybe_unused]] const bool loaded = environment.reset(levelPath);
    PROJECTGAMING_ASSERT(loaded, "evaluateFitness : le niveau doit se charger");

    const ObservationEncoder observationEncoder;
    const RewardConfig rewardConfig;

    // Boîte/état de départ : même convention que test_recompense_demo_niveaux.cpp (LOT-ANNEXE-08)
    // -- HeadlessLevelEnvironment n'expose pas d'observation avant le premier step(), le premier
    // choix d'action se fait donc sur l'état de spawn reconstruit, un `core::Player` par defaut.
    const core::GridPosition entry = environment.level().entry();
    core::Aabb previousBox = core::Aabb::fromTopLeftSize(
        core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
    core::Player playerState{};
    core::Velocity playerVelocity{};

    float cumulativeReward = 0.0f;
    EpisodeStatus status = EpisodeStatus::Ongoing;
    int stepCount = 0;

    while (status == EpisodeStatus::Ongoing && !environment.budgetExhausted()) {
        const Tensor<float> observationVector =
            observationEncoder.encode(environment, previousBox, playerState, playerVelocity);
        const autodiff::NodePtr inputNode = autodiff::variable(observationVector);
        const autodiff::NodePtr outputNode = individual.network().forward(inputNode);
        const Tensor<float> distribution = outputNode->value.view({actionCount()});
        const Action action = decodeArgmax(distribution);

        const StepObservation stepObservation = environment.step(toPlayerInput(action));
        // Reconstruit a chaque pas (LOT-ANNEXE-21) : voir TrajectoryCollector::collectEpisode.
        const GridDistanceField distanceField =
            buildObjectiveDistanceField(environment.level(), environment.mechanisms());
        cumulativeReward += computeReward(rewardConfig, distanceField, previousBox,
                                          stepObservation.playerBox, stepObservation.outcome);

        previousBox = stepObservation.playerBox;
        playerState = stepObservation.playerState;
        playerVelocity = stepObservation.playerVelocity;
        ++stepCount;

        status = classifyEpisode(stepObservation.outcome, stepObservation.stepIndex,
                                 environment.stepsSinceProgress(), std::numeric_limits<int>::max(),
                                 stuckThreshold);
    }
    if (status == EpisodeStatus::Ongoing) {
        // Budget dur atteint sans que classifyEpisode() (hardStepBudget volontairement desactive
        // ci-dessus) ne l'ait detecte -- HeadlessLevelEnvironment::budgetExhausted() reste la
        // seule source de verite sur EnvironmentConfig::maxSteps, non expose autrement ici.
        status = EpisodeStatus::TimedOut;
    }

    individual.fitness = cumulativeReward;
    return FitnessEvaluation{cumulativeReward, stepCount, status};
}

}  // namespace aisolver::training::evolutionary
