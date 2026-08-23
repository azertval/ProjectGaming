// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_training_run_path.cpp
 * @brief Tests unitaires de aisolver::makeTrainingRunPath / aisolver::generateRunId
 * (LOT-ANNEXE-09, TACHE-04).
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "AiSolver/Stats/TrainingRunPath.h"

namespace {

/// Répertoire temporaire du test courant, nettoyé à la destruction (RAII), même patron que
/// Source/Test/Unit/AiSolver/Replay/test_replay_file.cpp.
class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() / "aisolver_test_training_run_path") {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
        std::filesystem::create_directories(_path);
    }
    ~TempDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    [[nodiscard]] const std::filesystem::path& path() const {
        return _path;
    }

private:
    std::filesystem::path _path;
};

}  // namespace

/**
 * @brief `makeTrainingRunPath` produit le chemin attendu et crée les dossiers intermédiaires.
 * \castest{<b>Construction du chemin d'un run et création des dossiers intermédiaires.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `makeTrainingRunPath(root, "demo-deplacement", "20260729-143512")`.<br/>
 * \tattendu Le chemin retourné est `root/demo-deplacement/20260729-143512/stats.csv` ; le dossier
 * parent existe réellement sur disque.}
 */
TEST(TrainingRunPathTest, ConstructionCheminEtCreationDossiers) {
    TempDirectory tempDir;
    const std::filesystem::path result =
        aisolver::makeTrainingRunPath(tempDir.path(), "demo-deplacement", "20260729-143512");

    EXPECT_EQ(result, tempDir.path() / "demo-deplacement" / "20260729-143512" / "stats.csv");
    EXPECT_TRUE(std::filesystem::is_directory(result.parent_path()));
}

/**
 * @brief Deux `levelName` différents produisent des chemins disjoints, dossiers compris.
 * \castest{<b>Isolation des chemins par niveau.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. `makeTrainingRunPath` avec `levelName = "niveau-a"` puis `"niveau-b"`, même
 * `runId`.<br/>
 * \tattendu Les deux chemins retournés sont différents et leurs dossiers parents respectifs
 * existent tous les deux, indépendamment.}
 */
TEST(TrainingRunPathTest, IsolationParNiveau) {
    TempDirectory tempDir;
    const std::filesystem::path pathA =
        aisolver::makeTrainingRunPath(tempDir.path(), "niveau-a", "run-1");
    const std::filesystem::path pathB =
        aisolver::makeTrainingRunPath(tempDir.path(), "niveau-b", "run-1");

    EXPECT_NE(pathA, pathB);
    EXPECT_TRUE(std::filesystem::is_directory(pathA.parent_path()));
    EXPECT_TRUE(std::filesystem::is_directory(pathB.parent_path()));
}

/**
 * @brief Deux appels à `generateRunId()` produisent des identifiants distincts.
 * \castest{<b>Absence de collision entre deux appels successifs à `generateRunId`.</b><br/>
 * \tcat Unitaire · AiSolver Stats<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Appeler `generateRunId()` deux fois de suite (potentiellement dans la même
 * seconde).<br/>
 * \tattendu Les deux identifiants retournés sont distincts.}
 */
TEST(TrainingRunPathTest, AbsenceDeCollisionEntreAppelsSuccessifs) {
    const std::string first = aisolver::generateRunId();
    const std::string second = aisolver::generateRunId();
    EXPECT_NE(first, second);
}
