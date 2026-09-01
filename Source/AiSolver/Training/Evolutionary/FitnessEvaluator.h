// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Training/Evolutionary/Individual.h"

/**
 * @file AiSolver/Training/Evolutionary/FitnessEvaluator.h
 * @brief Transforme un individu en un score : un run complet sur `HeadlessLevelEnvironment`
 * (`LOT-ANNEXE-10`, `EX-IA-011`).
 */

namespace aisolver::training::evolutionary {

/**
 * @brief Issue complète d'une évaluation : le fitness (déjà écrit dans `Individual::fitness`), plus
 * le nombre de pas et le statut de fin d'épisode — nécessaires à `EvolutionaryTrainer` pour
 * journaliser une génération (`Stats/TrainingStatsRow`) sans faire porter cet état à `Individual`
 * (qui reste une politique pure, cf. `Individual.h`).
 */
struct FitnessEvaluation {
    float fitness = 0.0f;
    int stepCount = 0;
    EpisodeStatus status = EpisodeStatus::Ongoing;
};

/**
 * @brief Réinitialise `environment` sur `levelPath`, puis joue `individual` jusqu'à fin d'épisode :
 * à chaque pas, encode l'observation courante (`ObservationEncoder`), propage
 * `individual.network()` en avant, décode l'action en `argmax` (jamais d'échantillonnage
 * stochastique — décision de cadrage de l'épic), applique l'action et cumule la récompense
 * (`Reward.h`). Écrit le total dans `individual.fitness`.
 * @param individual  Individu évalué ; `fitness` mis à jour en sortie.
 * @param environment Environnement réutilisé, réinitialisé en tout premier (aucun état ne fuit d'un
 *                    appel au suivant).
 * @param levelPath   Niveau à charger (`HeadlessLevelEnvironment::reset`).
 * @return Le fitness, le nombre de pas et le statut de fin de cet épisode.
 */
[[nodiscard]] FitnessEvaluation evaluateFitness(Individual& individual,
                                                HeadlessLevelEnvironment& environment,
                                                const std::filesystem::path& levelPath);

}  // namespace aisolver::training::evolutionary
