// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "AiSolver/Eval/ConvergenceComparator.h"

/**
 * @file AiSolver/Eval/GenerationComparator.h
 * @brief Comparaison chiffrée de convergence entre *N* approches (`LOT-ANNEXE-14`, TACHE-03,
 * `EX-IA-015`) -- généralise `ConvergenceComparator` (`LOT-ANNEXE-13`, deux séries) sans dupliquer
 * sa logique de lecture/calcul, réutilisée telle quelle par série.
 *
 * **API de bibliothèque, sans point d'entrée `aisolver-cli`.** L'outil en ligne de commande
 * n'expose que `train`, `evaluate` et `export-replay` : ce module est appelé par ses tests, et
 * reste disponible pour une nouvelle campagne. Ce n'est pas un oubli — la campagne du lot a été
 * exécutée une fois et ses résultats consignés (`Documentation/Lot-Annexe/`).
 */

namespace aisolver::eval {

/// Une série nommée : plusieurs essais (graines) du même algorithme, un CSV `TrainingStatsRecorder`
/// par essai (même schéma de colonnes communes que `ConvergenceComparator`).
struct NamedSeries {
    std::string name;
    std::vector<std::filesystem::path> csvPaths;
};

/// Résultat d'une série : absent (`std::nullopt`) si @ref NamedSeries::csvPaths était vide --
/// signalé explicitement plutôt qu'un résultat silencieusement faussé (critère d'acceptation de
/// TACHE-03).
struct GenerationComparisonResult {
    std::string name;
    std::optional<ConvergenceReport> report;
};

/**
 * @brief Applique `compareConvergence` (`LOT-ANNEXE-13`) à chaque série, dans l'ordre fourni.
 * @param series          Une entrée par approche comparée (évolutionniste, REINFORCE,
 * acteur-critique, algorithme avancé, ...) ; chaque série lue indépendamment des autres.
 * @param rewardThreshold Même seuil, partagé par toutes les séries comparées.
 * @param finalWindowSize Même fenêtre de fin de run, partagée par toutes les séries.
 */
[[nodiscard]] std::vector<GenerationComparisonResult> compareGenerations(
    const std::vector<NamedSeries>& series, float rewardThreshold, int finalWindowSize = 10);

/**
 * @brief Convertit un budget évolutionniste (générations × population) en nombre équivalent
 * d'épisodes de jeu (`LOT-ANNEXE-10` : chaque individu évalué joue un épisode complet), pour que la
 * comparaison porte sur une quantité d'expérience de jeu comparable entre familles d'algorithmes
 * (décision de cadrage de l'épic, TACHE-03) plutôt que sur un nombre d'itérations d'API différent.
 * @param generationCount Nombre de générations exécutées.
 * @param populationSize  Taille de la population (`EvolutionaryConfig::populationSize`).
 * @return `generationCount * populationSize` (un épisode par individu évalué, par génération).
 */
[[nodiscard]] std::size_t evolutionaryEpisodeBudget(std::size_t generationCount,
                                                    std::size_t populationSize);

}  // namespace aisolver::eval
