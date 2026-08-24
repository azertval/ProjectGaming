// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Math/Tensor.h"

/**
 * @file AiSolver/Training/PolicyGradient/Trajectory.h
 * @brief Données pures d'un épisode collecté pour REINFORCE (`LOT-ANNEXE-12`, `EX-IA-013`).
 */

namespace aisolver::training {

/**
 * @brief Un pas de trajectoire : ce que REINFORCE doit connaître après coup pour calculer le retour
 * (`ReturnCalculator.h`) et la perte (`ReinforceLoss.h`) — aucune logique de calcul ici
 * (`EX-ARCH-011`).
 *
 * `logProbability` est la valeur du nœud de log-probabilité au moment de l'échantillonnage
 * (`TrajectoryCollector`), un `float` détaché : `ReinforceLoss.h` **rejoue** le passage avant pour
 * obtenir un nœud de graphe vivant, il ne réutilise jamais ce champ pour la rétropropagation.
 */
struct TrajectoryStep {
    /// Vecteur d'observation encodé au pas (`ObservationEncoder`, `LOT-ANNEXE-06`/`LOT-ANNEXE-10`).
    Tensor<float> observation{{0}};
    /// Indice de l'action échantillonnée (`ActionSpace::indexOf`, `LOT-ANNEXE-07`).
    std::size_t actionIndex = 0;
    /// Log-probabilité de `actionIndex` sous la distribution de la politique à ce pas ; toujours
    /// `<= 0`.
    float logProbability = 0.0f;
    /// Récompense immédiate de ce pas (`Reward.h`, `LOT-ANNEXE-08`).
    float reward = 0.0f;
};

/// Trajectoire complète d'un épisode : un pas par appel à `HeadlessLevelEnvironment::step`, plus
/// l'issue de fin d'épisode (`Episode.h`, `LOT-ANNEXE-08`).
struct Trajectory {
    std::vector<TrajectoryStep> steps;
    EpisodeStatus status = EpisodeStatus::Ongoing;
};

}  // namespace aisolver::training
