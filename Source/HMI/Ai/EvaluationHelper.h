// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <optional>

/**
 * @file HMI/Ai/EvaluationHelper.h
 * @brief Évaluation d'un modèle entraîné depuis l'onglet Validation & sauvegarde
 * (`LOT-ANNEXE-21`, `EX-IA-022`) : seule fonction libre de `HMI/Ai` en dehors de
 * `TrainingWorker` — même portée d'amendement documentée dans l'epic du lot (`HMI/Ai`
 * uniquement, jamais `HMI/Interface` directement).
 *
 * Délègue à `aisolver::eval::BenchmarkRunner::run` (`LOT-ANNEXE-15`) — même résultat qu'un appel
 * direct dans les mêmes conditions (répétitions, décodage Argmax), critère hérité de
 * `aisolver::cli::runEvaluate` (`LOT-ANNEXE-19`).
 */

namespace hmi {

/// Résultat d'une évaluation, mêmes champs que `aisolver::eval::BenchmarkResult` en types
/// simples (`HMI/Interface` n'a pas besoin de connaître le type `AiSolver` sous-jacent).
struct EvaluationOutcome {
    double successRate = 0.0;
    double meanStepsOnSuccess = 0.0;
    double stepVariance = 0.0;
};

/**
 * @brief Évalue le modèle `modelPath` (topologie de `algo`) sur `levelPath`, `repetitions` fois,
 * en mode Argmax.
 * @param modelPath   Chemin des poids sauvegardés (`nn::saveWeights`, format `LOT-ANNEXE-03`).
 * @param levelPath   Niveau sur lequel évaluer.
 * @param algo        `"evo"`, `"pg"`, `"ac"` ou `"avance"` — choisit l'adaptateur
 *                    `eval::TrainedPolicy` et la topologie réseau adaptée.
 * @param repetitions Nombre d'épisodes rejoués pour la mesure.
 * @return `std::nullopt` si le modèle ne se charge pas (chemin invalide, format incompatible).
 */
[[nodiscard]] std::optional<EvaluationOutcome> evaluateModel(const QString& modelPath,
                                                             const QString& levelPath,
                                                             const QString& algo, int repetitions);

}  // namespace hmi
