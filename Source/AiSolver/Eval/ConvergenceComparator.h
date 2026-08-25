// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <vector>

/**
 * @file AiSolver/Eval/ConvergenceComparator.h
 * @brief Comparaison chiffrée de convergence entre plusieurs runs d'un même algorithme, à partir
 * des CSV `TrainingStatsRecorder` (`LOT-ANNEXE-13`, TACHE-04, `EX-IA-014`).
 *
 * Décision de cadrage : les CSV lus par ce comparateur ont le **même schéma de colonnes** que
 * ceux produits par `ReinforceTrainer` (`LOT-ANNEXE-12`), qu'ils proviennent d'un run REINFORCE ou
 * d'un run `ActorCriticTrainer` (`LOT-ANNEXE-13`, TACHE-03) — la perte du critique, spécifique à
 * l'acteur-critique, est journalisée à part (voir `ActorCriticTrainer.h`) et n'entre pas dans cette
 * comparaison. Les colonnes sont retrouvées par nom dans l'en-tête (pas par position fixe), pour
 * rester robuste à un ajout de colonne futur dans `Stats/CsvFormat.h`.
 *
 * **API de bibliothèque, sans point d'entrée `aisolver-cli`.** L'outil en ligne de commande
 * n'expose que `train`, `evaluate` et `export-replay` : ce module est appelé par ses tests, et
 * reste disponible pour une nouvelle campagne. Ce n'est pas un oubli — la campagne du lot a été
 * exécutée une fois et ses résultats consignés (`Documentation/Lot-Annexe/`).
 */

namespace aisolver::eval {

/// Métriques de convergence d'un unique run (un CSV).
struct RunConvergenceMetrics {
    /// Index du premier épisode (0-based) dont `bestReward >= rewardThreshold` ; absent si jamais
    /// atteint sur ce run (valeur sentinelle explicite plutôt qu'un nombre d'épisodes erroné).
    std::optional<int> episodesToThreshold;
    /// Moyenne de `bestReward` sur les `finalWindowSize` derniers épisodes du run (ou moins si le
    /// run est plus court).
    float finalWindowMeanReward = 0.0f;
};

/**
 * @brief Lit un CSV produit par `TrainingStatsRecorder` et calcule ses métriques de convergence.
 * @param csvPath        Chemin du CSV (en-tête + une ligne par épisode).
 * @param rewardThreshold Seuil de récompense défini une fois pour toute la comparaison.
 * @param finalWindowSize Taille de la fenêtre de fin de run pour `finalWindowMeanReward`.
 * @pre Le CSV contient une colonne `bestReward` (nom retrouvé dans l'en-tête).
 */
[[nodiscard]] RunConvergenceMetrics analyzeRun(const std::filesystem::path& csvPath,
                                               float rewardThreshold, int finalWindowSize = 10);

/// Rapport de convergence agrégé sur plusieurs essais (graines) d'un même algorithme.
struct ConvergenceReport {
    /// Nombre moyen d'épisodes jusqu'au seuil, sur les seuls essais qui l'ont atteint ; absent si
    /// aucun essai n'a atteint le seuil (résultat explicite, pas une moyenne vide silencieuse).
    std::optional<float> meanEpisodesToThreshold;
    /// Nombre d'essais ayant atteint le seuil, sur `totalTrials`.
    std::size_t trialsReachingThreshold = 0;
    /// Nombre total d'essais comparés.
    std::size_t totalTrials = 0;
    /// Écart-type, à travers les essais, de `finalWindowMeanReward` — mesure de stabilité
    /// inter-essais (décision de cadrage : mesurée entre essais, pas entre épisodes d'un seul run).
    float finalRewardStdDev = 0.0f;
};

/**
 * @brief Agrège les métriques de convergence de plusieurs runs (essais répétés d'un même
 * algorithme, graines différentes).
 * @param csvPaths        Un CSV par essai ; au moins un élément.
 * @param rewardThreshold Même seuil que `analyzeRun`, partagé entre les deux algorithmes comparés.
 * @param finalWindowSize Même fenêtre que `analyzeRun`.
 */
[[nodiscard]] ConvergenceReport compareConvergence(
    const std::vector<std::filesystem::path>& csvPaths, float rewardThreshold,
    int finalWindowSize = 10);

}  // namespace aisolver::eval
