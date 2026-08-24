// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

/**
 * @file AiSolver/Training/PolicyGradient/TrajectoryCollector.h
 * @brief Rejoue un épisode complet en échantillonnant les actions selon la politique
 * (`LOT-ANNEXE-12`, TACHE-01, `EX-IA-013`).
 */

namespace aisolver::training {

/**
 * @brief Collecte une trajectoire complète sur un environnement déjà chargé/réinitialisé.
 *
 * Contrairement à `evolutionary::evaluateFitness` (`LOT-ANNEXE-10`), ne réinitialise jamais
 * `environment` elle-même : l'appelant (`ReinforceTrainer`, `LOT-ANNEXE-12` TACHE-04) est seul
 * responsable de `HeadlessLevelEnvironment::reset` avant chaque appel, pour garder ce collecteur
 * agnostique du chemin de niveau et testable sur un environnement construit à la main.
 */
class TrajectoryCollector {
public:
    /// @param stuckThreshold Seuil de blocage transmis à `classifyEpisode` (`Episode.h`), même
    /// défaut que `evolutionary::evaluateFitness` pour rester cohérent entre familles
    /// d'algorithmes.
    explicit TrajectoryCollector(int stuckThreshold = evolutionary::DEFAULT_STUCK_THRESHOLD);

    /**
     * @brief Rejoue `policy` sur `environment` jusqu'à fin d'épisode, en échantillonnant chaque
     * action selon la distribution produite par le réseau (jamais `argmax`, cf. décision de
     * cadrage de l'épic).
     * @param environment Environnement déjà chargé et réinitialisé sur le niveau visé.
     * @param policy      Réseau de politique (sortie `softmax` de taille `actionCount()`).
     * @param rng         Générateur déterministe fourni par l'appelant, seule source d'aléatoire.
     * @return La trajectoire complète de l'épisode.
     */
    [[nodiscard]] Trajectory collectEpisode(HeadlessLevelEnvironment& environment,
                                            nn::Network& policy, Rng& rng) const;

private:
    int _stuckThreshold;
};

}  // namespace aisolver::training
