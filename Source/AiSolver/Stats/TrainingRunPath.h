// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

/**
 * @file AiSolver/Stats/TrainingRunPath.h
 * @brief Construction du chemin de fichier CSV d'un run d'entraînement, sous `/TrainingRuns/`
 * (`LOT-ANNEXE-09`, `EX-IA-010`).
 */

namespace aisolver {

/// Racine par défaut des sorties d'entraînement, relative à la racine du dépôt (déjà exclue de
/// Git par `.gitignore`, `/TrainingRuns/`). Reste un nom relatif : l'appelant (CLI,
/// `LOT-ANNEXE-19`) résout ce chemin en absolu selon son propre répertoire de travail — cette
/// fonction ne fait aucune hypothèse sur le répertoire courant du processus.
inline constexpr const char* kDefaultTrainingRunsRoot = "TrainingRuns";

/**
 * @brief Construit le chemin `trainingRunsRoot / levelName / runId / "stats.csv"` et crée les
 * dossiers intermédiaires manquants.
 * @param trainingRunsRoot Racine des runs d'entraînement (voir `kDefaultTrainingRunsRoot`).
 * @param levelName Nom du niveau entraîné : isole les runs de niveaux différents dans des dossiers
 * disjoints (régime d'entraînement niveau par niveau).
 * @param runId Identifiant du run (voir `generateRunId`) : deux runs successifs sur le même niveau
 * restent chacun consultables séparément.
 * @return Le chemin du fichier `stats.csv` du run, dossiers parents déjà créés sur disque.
 */
[[nodiscard]] std::filesystem::path makeTrainingRunPath(
    const std::filesystem::path& trainingRunsRoot, std::string_view levelName,
    std::string_view runId);

/**
 * @brief Génère un identifiant de run par horodatage ISO 8601 compact (ex. `20260729-143512`).
 *
 * Deux appels dans la même seconde produisent des identifiants distincts : un compteur
 * incrémental est suffixé en repli (`20260729-143512-1`, `20260729-143512-2`, …).
 */
[[nodiscard]] std::string generateRunId();

}  // namespace aisolver
