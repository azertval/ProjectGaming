// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

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

/**
 * @brief Observateur appelé après chaque répétition d'une campagne.
 *
 * Une campagne est une suite de rejeux complets : sur un budget de pas élevé et un grand nombre de
 * répétitions, elle dure assez longtemps pour qu'une interface doive pouvoir en montrer
 * l'avancement et l'annuler. La granularité est la répétition — jamais le pas de simulation, qui
 * ferait payer un appel indirect à chaque image simulée.
 * @param completed Nombre de répétitions terminées (1 après la première).
 * @param total     `BenchmarkConfig::repetitions`, rappelé pour que l'observateur n'ait pas à
 *                  connaître la configuration.
 * @return `false` pour interrompre la campagne : le résultat ne porte alors que les répétitions
 *         déjà jouées (résultat partiel, jamais une exception).
 */
using RepetitionObserver = std::function<bool(int completed, int total)>;

class BenchmarkRunner {
public:
    /**
     * @brief Exécute `config.repetitions` répétitions de @p policy sur @p levelPath, chacune sur un
     * `HeadlessLevelEnvironment` fraîchement réinitialisé (isolation stricte entre répétitions).
     *
     * Équivalent à `runWithNoise(policy, levelPath, config, 0.0f)` (aucun bruit d'observation).
     * @param policy Politique entraînée à évaluer, non modifiée.
     * @param levelPath Niveau sur lequel exécuter les répétitions.
     * @param config Nombre de répétitions, budget de pas et mode de décodage.
     * @param onRepetition Observateur optionnel de progression/interruption, voir
     *        `RepetitionObserver`. `nullptr` (défaut) : comportement inchangé.
     */
    [[nodiscard]] static BenchmarkResult run(TrainedPolicy& policy,
                                             const std::filesystem::path& levelPath,
                                             const BenchmarkConfig& config,
                                             const RepetitionObserver& onRepetition = {});

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
     * @param onRepetition Observateur optionnel de progression/interruption, voir
     *        `RepetitionObserver`. `nullptr` (défaut) : comportement inchangé.
     */
    [[nodiscard]] static BenchmarkResult runWithNoise(TrainedPolicy& policy,
                                                      const std::filesystem::path& levelPath,
                                                      const BenchmarkConfig& config,
                                                      float noiseAmplitude,
                                                      const RepetitionObserver& onRepetition = {});
};

}  // namespace aisolver::eval
