// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Eval/BenchmarkConfig.h"
#include "AiSolver/Eval/BenchmarkResult.h"
#include "AiSolver/Eval/TrainedPolicy.h"

/**
 * @file AiSolver/Eval/BenchmarkRunner.h
 * @brief Exécution automatisée répétée d'une politique entraînée sur son niveau d'origine
 * (`LOT-ANNEXE-15`, TACHE-01/TACHE-03, `EX-IA-016`).
 */

namespace aisolver::eval {

/**
 * @brief Dérive une graine déterministe et distincte pour la répétition @p repetitionIndex à
 * partir d'une graine de base.
 *
 * Même convention que `ReinforceTrainer`/`ActorCriticTrainer` (`seedBase + index`) : simple,
 * déterministe, jamais l'horloge ni `std::rand`.
 */
[[nodiscard]] std::uint64_t deriveSeed(std::uint64_t base, int repetitionIndex);

class BenchmarkRunner {
public:
    /**
     * @brief Exécute `config.repetitions` répétitions de @p policy sur @p levelPath, chacune sur un
     * `HeadlessLevelEnvironment` fraîchement réinitialisé (isolation stricte entre répétitions).
     *
     * Équivalent à `runWithNoise(policy, levelPath, config, 0.0f)` (aucun bruit d'observation).
     */
    [[nodiscard]] static BenchmarkResult run(TrainedPolicy& policy,
                                             const std::filesystem::path& levelPath,
                                             const BenchmarkConfig& config);

    /**
     * @brief Comme `run`, mais l'observation transmise à @p policy est perturbée par un bruit
     * gaussien d'amplitude @p noiseAmplitude (`NoisyObservationWrapper`, TACHE-03) — l'état réel
     * simulé par `HeadlessLevelEnvironment` (et donc l'issue jugée par `core::evaluateOutcome`)
     * n'est jamais affecté par ce bruit, seule la perception de la politique change.
     *
     * Toute la stochasticité d'une répétition (bruit **et** décodage `Stochastic` éventuel) passe
     * par une seule instance de `Rng`, dérivée de `deriveSeed(config.rngSeedBase, i)` (décision de
     * cadrage de l'épic) : deux appels avec la même configuration produisent des résultats
     * identiques.
     * @param policy Politique entraînée à évaluer, non modifiée.
     * @param levelPath Niveau d'origine sur lequel exécuter les répétitions.
     * @param config Nombre de répétitions, budget de pas et mode de décodage.
     * @param noiseAmplitude `<= 0.0f` désactive le bruit (voir `NoisyObservationWrapper`).
     */
    [[nodiscard]] static BenchmarkResult runWithNoise(TrainedPolicy& policy,
                                                       const std::filesystem::path& levelPath,
                                                       const BenchmarkConfig& config,
                                                       float noiseAmplitude);
};

}  // namespace aisolver::eval
