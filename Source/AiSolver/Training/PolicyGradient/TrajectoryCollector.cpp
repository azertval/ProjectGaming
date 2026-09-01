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

TrajectoryCollector::TrajectoryCollector(int actionRepeat, float explorationFloor)
    : _actionRepeat(actionRepeat), _explorationFloor(explorationFloor) {
    PROJECTGAMING_ASSERT(actionRepeat >= 1,
                         "TrajectoryCollector : actionRepeat doit valoir au moins 1");
    PROJECTGAMING_ASSERT(explorationFloor >= 0.0f && explorationFloor < 1.0f,
                         "TrajectoryCollector : explorationFloor doit etre dans [0, 1[");
}

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
        //
        // Plancher d'exploration : une part `epsilon` d'uniforme melangee a la distribution, sans
        // quoi une politique saturee cesse definitivement d'essayer autre chose -- mesure a l'appui
        // (`PolicyGradientTuning.h`). Le gradient reste celui de la politique, jamais celui de la
        // distribution melangee.
        Tensor<float> samplingDistribution = distribution.clone();
        if (_explorationFloor > 0.0f) {
            const float uniformShare = _explorationFloor / static_cast<float>(actionCount());
            for (std::size_t index = 0; index < actionCount(); ++index) {
                samplingDistribution.data()[index] =
                    (1.0f - _explorationFloor) * distribution.data()[index] + uniformShare;
            }
        }
        const Action action = decodeStochastic(samplingDistribution, 1.0f, rng);
        const std::size_t actionIndex = indexOf(action);
        const float probability = distribution.data()[actionIndex];
        const float logProbability = std::log(probability);

        // L'action decidee est maintenue `_actionRepeat` images ; la recompense du pas de
        // trajectoire est la somme de celles des images qu'il recouvre, et l'episode peut se
        // terminer au milieu d'une repetition.
        float reward = 0.0f;
        for (int frame = 0; frame < _actionRepeat && status == EpisodeStatus::Ongoing &&
                            !environment.budgetExhausted();
             ++frame) {
            const StepObservation stepObservation = environment.step(toPlayerInput(action, frame));
            // Champ de l'environnement : une seule instance par episode, deja reconstruite par
            // `step()` quand elle a cesse d'etre valide -- et celle-la meme dont la detection de
            // blocage se sert.
            reward += computeReward(rewardConfig, environment.objectiveField(), previousBox,
                                    stepObservation.playerBox, stepObservation.outcome);

            previousBox = stepObservation.playerBox;
            playerState = stepObservation.playerState;
            playerVelocity = stepObservation.playerVelocity;

            status = classifyEpisode(stepObservation.outcome, stepObservation.stepIndex,
                                     environment.stepsSinceProgress(),
                                     std::numeric_limits<int>::max(), environment.stuckThreshold());
        }

        trajectory.steps.push_back(
            TrajectoryStep{observationVector, actionIndex, logProbability, reward});
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
