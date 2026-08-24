// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "AiSolver/Stats/TrainingRunPath.h"

/**
 * @file AiSolver/Cli/Commands.h
 * @brief Sous-commandes `train`/`evaluate`/`export-replay` de `aisolver-cli` (`LOT-ANNEXE-19`,
 * TACHE-01, `EX-IA-020`) — minces habillages autour des types déjà cadrés par les lots amont,
 * aucune logique d'entraînement/évaluation/export réimplémentée ici.
 *
 * `--algo` accepte `evo` (évolutionniste, `LOT-ANNEXE-10`), `pg` (REINFORCE, `LOT-ANNEXE-12`),
 * `ac` (acteur-critique, `LOT-ANNEXE-13`) ou `avance` (DQN, `LOT-ANNEXE-14`).
 *
 * Écart documenté par rapport au texte de l'épic : `evaluate`/`export-replay` exposent un `--algo`
 * requis (absent du texte de l'épic), nécessaire pour reconstruire la topologie du réseau et choisir
 * le bon adaptateur `TrainedPolicy` (`LOT-ANNEXE-15`) avant de recharger des poids déjà entraînés —
 * `Nn/Serialization.h` (`LOT-ANNEXE-03`) ne stocke que des formes, jamais l'activation d'une couche,
 * qui distingue pourtant DQN (sortie non bornée) des trois autres familles (sortie `softmax`).
 */

namespace aisolver::cli {

/// Arguments de `train`.
struct TrainArgs {
    std::filesystem::path level;
    std::string algo;
    std::uint64_t seed = 0;
    std::optional<std::filesystem::path> configFile;
    std::filesystem::path runsRoot = kDefaultTrainingRunsRoot;
    std::optional<std::size_t> populationSize;
    std::optional<float> mutationRate;
    std::optional<std::size_t> episodes;
    std::optional<float> learningRate;
    std::optional<float> gamma;
    std::optional<std::string> optimizer;
};

/// Arguments de `evaluate`.
struct EvaluateArgs {
    std::filesystem::path model;
    std::string algo;
    std::filesystem::path level;
    int repetitions = 30;
    std::optional<std::filesystem::path> report;
};

/// Arguments de `export-replay`.
struct ExportReplayArgs {
    std::filesystem::path model;
    std::string algo;
    std::filesystem::path level;
    std::filesystem::path output;
    std::uint64_t seed = 0;
};

/**
 * @brief Analyse les arguments de `train` (hors nom de sous-commande).
 * @param args Arguments bruts (ex. `argv[2..]`).
 * @param error Rempli d'un message explicite si l'analyse échoue.
 * @return Les arguments analysés, ou `std::nullopt` si `--level`/`--algo` manquent ou si `--algo`
 *         n'est pas l'une des quatre valeurs acceptées.
 */
[[nodiscard]] std::optional<TrainArgs> parseTrainArgs(const std::vector<std::string>& args,
                                                       std::string& error);

/// @copydoc parseTrainArgs
[[nodiscard]] std::optional<EvaluateArgs> parseEvaluateArgs(const std::vector<std::string>& args,
                                                             std::string& error);

/// @copydoc parseTrainArgs
[[nodiscard]] std::optional<ExportReplayArgs> parseExportReplayArgs(
    const std::vector<std::string>& args, std::string& error);

/// @return `true` si `algo` est l'une des quatre valeurs acceptées (`evo`/`pg`/`ac`/`avance`).
[[nodiscard]] bool isKnownAlgorithm(const std::string& algo);

/**
 * @brief Exécute `train` : construit et lance un entraînement complet sur `args.level`, journalise
 * (`TrainingStatsRecorder`), sauvegarde les poids du modèle final et exporte un rejeu si résolu.
 * @return Code de sortie (`0` si succès, non nul sinon) ; message d'erreur sur `stderr`.
 */
[[nodiscard]] int runTrain(const TrainArgs& args);

/**
 * @brief Exécute `evaluate` : charge le modèle, l'exécute via `BenchmarkRunner::run`, affiche le
 * résultat sur `stdout` et écrit optionnellement un rapport CSV.
 * @return Code de sortie (`0` si succès, non nul sinon) ; message d'erreur sur `stderr`.
 */
[[nodiscard]] int runEvaluate(const EvaluateArgs& args);

/**
 * @brief Exécute `export-replay` : charge le modèle, le rejoue en mode `Argmax` et écrit un fichier
 * de rejeu.
 * @return Code de sortie (`0` si succès, non nul sinon) ; message d'erreur sur `stderr`.
 */
[[nodiscard]] int runExportReplay(const ExportReplayArgs& args);

}  // namespace aisolver::cli
