// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Ai/TrainingOverrides.h"

namespace hmi {

aisolver::cli::CommandLineOverrides overridesFor(const TrainingRequest& request) {
    // Designateurs plutot qu'une liste positionnelle : `CommandLineOverrides` est le contrat
    // partage avec la CLI et gagne des champs a chaque hyperparametre expose -- un ajout au milieu
    // de la structure decalerait silencieusement toutes les valeurs suivantes.
    return aisolver::cli::CommandLineOverrides{
        .populationSize = request.populationSize,
        .mutationRate = request.mutationRate,
        .episodes = request.episodes,
        .learningRate = request.learningRate,
        .criticLearningRate = request.criticLearningRate,
        .gamma = request.gamma,
        .optimizer = std::nullopt,  // applique par l'appelant, l'ecran le fournit en QString
        .hiddenSize = request.hiddenSize,
        .tournamentSize = request.tournamentSize,
        .mutationStrength = request.mutationStrength,
        .maxGenerations = request.maxGenerations,
        .requiredConsecutiveSuccesses = request.requiredConsecutiveSuccesses,
        .dqnReplayCapacity = request.dqnReplayCapacity,
        .dqnBatchSize = request.dqnBatchSize,
        .dqnWarmupSize = request.dqnWarmupSize,
        .dqnUpdatePeriodSteps = request.dqnUpdatePeriodSteps,
        .dqnTargetSyncPeriodSteps = request.dqnTargetSyncPeriodSteps,
        .dqnEpsilonStart = request.dqnEpsilonStart,
        .dqnEpsilonEnd = request.dqnEpsilonEnd,
        .dqnEpsilonDecaySteps = request.dqnEpsilonDecaySteps,
        .batchEpisodes = request.batchEpisodes,
        .entropyCoefficient = request.entropyCoefficient,
        .gradientClipNorm = request.gradientClipNorm,
        .actionRepeat = request.actionRepeat,
        .explorationFloor = request.explorationFloor,
        .crossoverRate = request.crossoverRate,
        .maxSteps = request.maxSteps,
        .stuckThreshold = request.stuckThreshold,
    };
}

}  // namespace hmi
