// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <cstddef>
#include <cstdint>
#include <optional>

/**
 * @file HMI/Ai/TrainingRequest.h
 * @brief Paramètres d'un run d'entraînement saisis dans l'onglet Entraînement (`LOT-ANNEXE-21`).
 *
 * Séparé de `HMI/Ai/TrainingWorker.h` par le `LOT-73` : le worker est un `QObject` à signaux, donc
 * soumis à `moc` et impossible à compiler dans `UnitTests` ; la requête, elle, n'est que de la
 * **donnée**. Cette séparation est ce qui rend testable la traduction requête -> surcharges
 * (`HMI/Ai/TrainingOverrides.h`) — la traduction précisément où neuf réglages se perdaient.
 */

namespace hmi {

/// Paramètres d'un run, saisis dans l'onglet Entraînement — mêmes champs que
/// `aisolver::cli::TrainArgs`/`CommandLineOverrides` (`LOT-ANNEXE-19`), en `QString`/valeurs
/// simples pour rester utilisables sans dépendre de `AiSolver/Cli` depuis un en-tête Qt.
struct TrainingRequest {
    QString levelPath;
    QString algorithmId;  ///< `"evo"`, `"pg"`, `"ac"` ou `"avance"`.
    std::uint64_t seed = 0;
    QString runsRoot;  ///< Vide : défaut `aisolver::training::DEFAULT_TRAINING_RUNS_ROOT`.
    std::optional<std::size_t> populationSize;
    std::optional<float> mutationRate;
    std::optional<std::size_t> episodes;
    std::optional<float> learningRate;
    std::optional<float> gamma;
    QString optimizer;  ///< Vide : défaut (`"sgd"`).
    /// Topologie du réseau de politique, commune à tous les algorithmes.
    std::optional<std::size_t> hiddenSize;
    /// Évolutionniste : reste de `EvolutionaryConfig` et critère d'arrêt `StoppingConfig`.
    /// `maxGenerations` est le plafond de générations — distinct d'`episodes`, qui ne concerne
    /// que les algorithmes par gradient.
    std::optional<int> tournamentSize;
    std::optional<float> mutationStrength;
    std::optional<int> maxGenerations;
    std::optional<int> requiredConsecutiveSuccesses;
    /// Hyperparamètres DQN (voir `aisolver::cli::TrainingConfig`), pertinents uniquement pour
    /// `algorithmId == "avance"` — groupe dédié de l'onglet Entraînement, masqué pour les autres
    /// algorithmes (`LOT-ANNEXE-21`).
    std::optional<std::size_t> dqnReplayCapacity;
    std::optional<std::size_t> dqnBatchSize;
    std::optional<std::size_t> dqnWarmupSize;
    std::optional<std::size_t> dqnUpdatePeriodSteps;
    std::optional<std::size_t> dqnTargetSyncPeriodSteps;
    std::optional<float> dqnEpsilonStart;
    std::optional<float> dqnEpsilonEnd;
    std::optional<std::size_t> dqnEpsilonDecaySteps;
    /// Réglages de policy gradient (`aisolver::training::PolicyGradientTuning`), pertinents pour
    /// `"pg"`/`"ac"` — et `actionRepeat` aussi pour `"avance"`.
    std::optional<float> criticLearningRate;
    std::optional<std::size_t> batchEpisodes;
    std::optional<float> entropyCoefficient;
    std::optional<float> gradientClipNorm;
    std::optional<int> actionRepeat;
    std::optional<float> explorationFloor;
    /// Probabilité de croisement (évolutionniste).
    std::optional<float> crossoverRate;
    /// Budget de pas et seuil de blocage de l'environnement ; `0` = dérivés du niveau.
    std::optional<int> maxSteps;
    std::optional<int> stuckThreshold;
};

}  // namespace hmi
