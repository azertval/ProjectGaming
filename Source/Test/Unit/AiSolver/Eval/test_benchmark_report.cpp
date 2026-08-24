// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_benchmark_report.cpp
 * @brief Tests unitaires de `BenchmarkReport` (LOT-ANNEXE-15, TACHE-02, `EX-IA-016`).
 */

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Eval/BenchmarkReport.h"

using aisolver::eval::BenchmarkReport;
using aisolver::eval::BenchmarkResult;
using aisolver::eval::EpisodeOutcome;

namespace {

std::filesystem::path reportPath(const char* suffix) {
    return std::filesystem::temp_directory_path() /
           (std::string("aisolver_test_benchmark_report_") + suffix + ".csv");
}

std::vector<std::string> readLines(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

BenchmarkResult makeResult(core::LevelOutcome outcome, int stepCount) {
    BenchmarkResult result;
    result.episodes.push_back(EpisodeOutcome{outcome, stepCount});
    return result;
}

}  // namespace

/**
 * \castest{Une ligne CSV par couple (algorithme, niveau) accumule.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. addResult trois fois (trois couples distincts).<br/>2. writeCsv.<br/>
 * \tattendu rowCount() == 3 ; le fichier ecrit contient 4 lignes (en-tete + 3).}
 */
TEST(BenchmarkReportTest, UneLigneParModeleAccumule) {
    BenchmarkReport report;
    report.addResult("Evolutionniste", "TrivialAI", makeResult(core::LevelOutcome::Won, 5));
    report.addResult("REINFORCE", "TrivialAI", makeResult(core::LevelOutcome::Won, 8));
    report.addResult("DQN", "TrivialAI", makeResult(core::LevelOutcome::Lost, 12));
    EXPECT_EQ(report.rowCount(), 3u);

    const std::filesystem::path path = reportPath("three_rows");
    report.writeCsv(path);
    const std::vector<std::string> lines = readLines(path);
    // En-tete + 3 lignes de donnees.
    EXPECT_EQ(lines.size(), 4u);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

/**
 * \castest{BenchmarkReport reutilise l'echappement CSV partage (escapeCsvField).<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Moyen<br/>
 * \tetapes 1. addResult avec un levelName contenant une virgule.<br/>2. writeCsv.<br/>
 * \tattendu Le champ est entoure de guillemets dans le fichier ecrit, meme convention que
 * TrainingStatsRecorder.}
 */
TEST(BenchmarkReportTest, ReutiliseLEchappementCsvPartage) {
    BenchmarkReport report;
    report.addResult("Evolutionniste", "Niveau, avec virgule",
                     makeResult(core::LevelOutcome::Won, 3));

    const std::filesystem::path path = reportPath("escaping");
    report.writeCsv(path);
    const std::vector<std::string> lines = readLines(path);
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_NE(lines[1].find(R"("Niveau, avec virgule")"), std::string::npos);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

/**
 * \castest{L'en-tete CSV est stable quel que soit le nombre de modeles accumules.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Ecrire un rapport a 1 ligne et un rapport a 4 lignes.<br/>
 * \tattendu Les deux fichiers ont exactement le meme en-tete.}
 */
TEST(BenchmarkReportTest, EnTeteStableQuelQueSoitLeNombreDeModeles) {
    BenchmarkReport smallReport;
    smallReport.addResult("Evolutionniste", "TrivialAI", makeResult(core::LevelOutcome::Won, 4));

    BenchmarkReport largeReport;
    largeReport.addResult("Evolutionniste", "TrivialAI", makeResult(core::LevelOutcome::Won, 4));
    largeReport.addResult("REINFORCE", "TrivialAI", makeResult(core::LevelOutcome::Won, 6));
    largeReport.addResult("ActorCritic", "TrivialAI", makeResult(core::LevelOutcome::Lost, 9));
    largeReport.addResult("DQN", "TrivialAI", makeResult(core::LevelOutcome::Won, 5));

    const std::filesystem::path smallPath = reportPath("small");
    const std::filesystem::path largePath = reportPath("large");
    smallReport.writeCsv(smallPath);
    largeReport.writeCsv(largePath);

    const std::vector<std::string> smallLines = readLines(smallPath);
    const std::vector<std::string> largeLines = readLines(largePath);
    ASSERT_FALSE(smallLines.empty());
    ASSERT_FALSE(largeLines.empty());
    EXPECT_EQ(smallLines.front(), largeLines.front());

    std::error_code ignored;
    std::filesystem::remove(smallPath, ignored);
    std::filesystem::remove(largePath, ignored);
}
