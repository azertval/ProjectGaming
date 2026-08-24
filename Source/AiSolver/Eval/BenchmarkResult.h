// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "Core/Levels/LevelOutcome.h"

/**
 * @file AiSolver/Eval/BenchmarkResult.h
 * @brief Résultat agrégé d'une exécution répétée d'une politique (`LOT-ANNEXE-15`, TACHE-01,
 * `EX-IA-016`).
 */

namespace aisolver::eval {

/// Issue d'une seule répétition : `core::LevelOutcome::Playing` en fin de budget de pas = timeout.
struct EpisodeOutcome {
    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    int stepCount = 0;
};

/**
 * @brief Accumule les issues de `N` répétitions et expose les métriques agrégées de fiabilité.
 *
 * `meanStepsAll` (toutes répétitions, échecs/timeouts inclus) et `meanStepsOnSuccess` (seules les
 * répétitions gagnées) sont volontairement deux méthodes distinctes : les mélanger gonflerait
 * artificiellement l'une ou l'autre selon le taux de réussite (voir `TrainedPolicy`, points
 * d'attention de la tâche).
 */
struct BenchmarkResult {
    std::vector<EpisodeOutcome> episodes;

    /// @return Fraction des répétitions en `core::LevelOutcome::Won`, dans `[0, 1]` ; `0.0` si
    /// `episodes` est vide.
    [[nodiscard]] double successRate() const;

    /// @return Nombre de pas moyen sur toutes les répétitions ; `0.0` si `episodes` est vide.
    [[nodiscard]] double meanStepsAll() const;

    /// @return Nombre de pas moyen sur les seules répétitions gagnées ; `0.0` si aucune répétition
    /// n'a réussi.
    [[nodiscard]] double meanStepsOnSuccess() const;

    /// @return Variance (population, pas échantillon) du nombre de pas sur `meanStepsAll` ; `0.0`
    /// si `episodes` est vide.
    [[nodiscard]] double stepVariance() const;
};

}  // namespace aisolver::eval
