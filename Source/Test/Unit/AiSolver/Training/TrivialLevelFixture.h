// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

/**
 * @file TrivialLevelFixture.h
 * @brief Niveau minimal (entrée/sortie adjacentes) utilisé par les tests de LOT-ANNEXE-11
 * (`LevelTrainingSession`, `DeterministicReplay`, `ReplayExport`) : résoluble par un simple
 * déplacement d'une case vers la droite, pour une convergence évolutionniste rapide et fiable en
 * test, indépendante de la difficulté réelle d'un niveau `demo-*.json`.
 */

namespace aisolver_test {

/// Corridor a deux cases, sol et murs lateraux : la seule action utile est un deplacement vers la
/// droite (aucun saut, aucun obstacle).
inline constexpr const char* TRIVIAL_LEVEL_JSON = R"({
  "name": "TrivialAI",
  "width": 4,
  "height": 3,
  "tiles": [
    {"x": 0, "y": 1, "type": "solid"},
    {"x": 1, "y": 1, "type": "entry"},
    {"x": 2, "y": 1, "type": "exit"},
    {"x": 3, "y": 1, "type": "solid"},
    {"x": 0, "y": 2, "type": "solid"},
    {"x": 1, "y": 2, "type": "solid"},
    {"x": 2, "y": 2, "type": "solid"},
    {"x": 3, "y": 2, "type": "solid"}
  ]
})";

/// Répertoire temporaire du test courant contenant `TRIVIAL_LEVEL_JSON` sur disque (requis par
/// `HeadlessLevelEnvironment::reset`, qui charge par chemin de fichier), nettoyé à la destruction
/// (RAII) — même patron que `Source/Test/Unit/AiSolver/Stats/test_training_stats_recorder.cpp`.
class TrivialLevelDirectory {
public:
    explicit TrivialLevelDirectory(const char* suffix)
        : _path(std::filesystem::temp_directory_path() /
                (std::string("aisolver_test_trivial_level_") + suffix)) {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
        std::filesystem::create_directories(_path);
        std::ofstream file(levelPath(), std::ios::binary | std::ios::trunc);
        file << TRIVIAL_LEVEL_JSON;
    }
    ~TrivialLevelDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }

    [[nodiscard]] std::filesystem::path levelPath() const {
        return _path / "trivial.json";
    }

    [[nodiscard]] std::filesystem::path file(const char* name) const {
        return _path / name;
    }

private:
    std::filesystem::path _path;
};

}  // namespace aisolver_test
