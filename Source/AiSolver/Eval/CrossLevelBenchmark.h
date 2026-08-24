// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "AiSolver/Eval/BenchmarkConfig.h"
#include "AiSolver/Eval/BenchmarkResult.h"
#include "AiSolver/Eval/TrainedPolicy.h"

/**
 * @file AiSolver/Eval/CrossLevelBenchmark.h
 * @brief Exécution d'une politique entraînée sur un niveau différent de celui qui l'a produite
 * (`LOT-ANNEXE-16`, TACHE-01, `EX-IA-017`).
 *
 * Mesure de référence sur la nature de l'apprentissage réalisé (transfert), jamais un objectif
 * d'entraînement : n'introduit aucune nouvelle boucle de simulation, assemble uniquement l'appel
 * existant à `BenchmarkRunner::run` (`LOT-ANNEXE-15`) avec les deux identifiants de niveau.
 * `BenchmarkRunner`/`BenchmarkReport` (`LOT-ANNEXE-15`) restent inchangés par ce lot : le rapport
 * croisé (`writeCrossLevelCsv`) est un fichier neuf plutôt qu'une surcharge de `BenchmarkReport`,
 * pour respecter le critère d'acceptation de l'épic (aucune ligne de `BenchmarkReport.*` modifiée).
 */

namespace aisolver::eval {

/// Résultat d'une exécution croisée : le `BenchmarkResult` (`LOT-ANNEXE-15`) déjà produit par
/// `BenchmarkRunner::run`, assemblé avec les deux identifiants de niveau distincts.
struct CrossLevelBenchmarkResult {
    std::string trainedOnLevel;
    std::string executedOnLevel;
    BenchmarkResult result;
};

/// Une paire à exécuter : une politique déjà entraînée sur `trainedOnLevel`, exécutée sur
/// `executedOnLevelPath` (`executedOnLevel` en est le nom lisible). Le chargement de l'adaptateur
/// `TrainedPolicy` approprié (`LOT-ANNEXE-15`, TACHE-01) reste à la charge de l'appelant.
struct CrossLevelPair {
    TrainedPolicy& policy;
    std::string trainedOnLevel;
    std::filesystem::path executedOnLevelPath;
    std::string executedOnLevel;
};

/**
 * @brief Exécute `BenchmarkRunner::run` (sans modification) pour chaque paire, dans l'ordre fourni.
 * @param pairs Paires (politique, niveau d'entraînement, niveau d'exécution) à exécuter.
 * @param config Paramètres de la campagne (répétitions, budget de pas, mode de décodage), communs
 *        à toutes les paires.
 * @return Un `CrossLevelBenchmarkResult` par paire, dans le même ordre que @p pairs.
 */
[[nodiscard]] std::vector<CrossLevelBenchmarkResult> runCrossLevelCampaign(
    const std::vector<CrossLevelPair>& pairs, const BenchmarkConfig& config);

/**
 * @brief Écrit un rapport CSV distinguant explicitement, colonne par colonne, le niveau
 * d'entraînement du niveau d'exécution (jamais fusionnés) — même échappement que
 * `BenchmarkReport`/`TrainingStatsRecorder` (`AiSolver/Stats/CsvFormat.h`).
 */
void writeCrossLevelCsv(const std::vector<CrossLevelBenchmarkResult>& results,
                        const std::filesystem::path& path);

}  // namespace aisolver::eval
