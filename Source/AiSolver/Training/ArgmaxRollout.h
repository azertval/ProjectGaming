// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Eval/TrainedPolicy.h"
#include "AiSolver/Training/DeterministicReplay.h"

/**
 * @file AiSolver/Training/ArgmaxRollout.h
 * @brief Rejeu déterministe d'un `eval::TrainedPolicy` en mode `Argmax` (`LOT-ANNEXE-19`,
 * extrait en `LOT-ANNEXE-21` de `Cli/Commands.cpp` pour être partagé avec l'IHM sans duplication).
 */

namespace aisolver::training {

/**
 * @brief Rejoue @p policy en mode `Argmax` sur `environment`/`levelPath`, jusqu'à fin d'épisode ou
 * budget de pas — même structure que `replayBestIndividual` (`LOT-ANNEXE-11`), généralisée à
 * n'importe quel `eval::TrainedPolicy` (`LOT-ANNEXE-15`) plutôt que dupliquée pour chaque famille
 * d'algorithme : c'est précisément le rôle de cette abstraction uniforme. Utilisée par
 * `aisolver::cli::runEvaluate`/`runExportReplay` (`LOT-ANNEXE-19`) et par `hmi::TrainingWorker`
 * (`LOT-ANNEXE-21`) — un seul rejeu déterministe générique, jamais réimplémenté par appelant.
 * @return `std::nullopt` si `levelPath` ne se charge pas ou si `policy` refuse `Argmax` (erreur
 *         récupérable, jamais de plantage).
 */
[[nodiscard]] std::optional<DeterministicReplayResult> argmaxRollout(
    eval::TrainedPolicy& policy, HeadlessLevelEnvironment& environment,
    const std::filesystem::path& levelPath);

}  // namespace aisolver::training
