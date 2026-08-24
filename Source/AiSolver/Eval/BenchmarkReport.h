// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "AiSolver/Eval/BenchmarkResult.h"

/**
 * @file AiSolver/Eval/BenchmarkReport.h
 * @brief Rapport comparatif CSV par niveau × algorithme (`LOT-ANNEXE-15`, TACHE-02, `EX-IA-016`).
 */

namespace aisolver::eval {

/// Une ligne du rapport : un couple (algorithme, niveau) exécuté dans la campagne.
struct BenchmarkReportRow {
    std::string algorithmName;
    std::string levelName;
    double successRate = 0.0;
    double meanStepCount = 0.0;
    double stepCountStdDev = 0.0;
};

/**
 * @brief Accumule des `BenchmarkResult` (sortie de `BenchmarkRunner`, un par modèle exécuté) au fil
 * d'une campagne, et écrit un rapport CSV descriptif — aucune agrégation ni classement au-delà
 * d'une ligne par couple exécuté (décision de cadrage de l'épic : pas de « meilleur algorithme »
 * encodé dans le format).
 *
 * Réutilise `aisolver::escapeCsvField` (`Stats/CsvFormat.h`, `LOT-ANNEXE-09`) : mêmes conventions
 * d'échappement que `TrainingStatsRecorder`, pas une seconde implémentation indépendante.
 */
class BenchmarkReport {
public:
    /// @brief Ajoute une ligne (un couple algorithme/niveau) à partir d'un `BenchmarkResult` déjà
    /// calculé (`meanStepCount` = `meanStepsAll()`, `stepCountStdDev` = racine de `stepVariance()`).
    void addResult(std::string algorithmName, std::string levelName, const BenchmarkResult& result);

    /// @return Nombre de lignes accumulées jusqu'ici.
    [[nodiscard]] std::size_t rowCount() const noexcept {
        return _rows.size();
    }

    /// @brief Écrit le rapport complet (en-tête puis une ligne par couple accumulé) dans @p path.
    void writeCsv(const std::filesystem::path& path) const;

private:
    std::vector<BenchmarkReportRow> _rows;
};

}  // namespace aisolver::eval
