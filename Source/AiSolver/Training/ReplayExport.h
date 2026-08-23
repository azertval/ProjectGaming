// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "AiSolver/Training/DeterministicReplay.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryConfig.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"
#include "AiSolver/Training/TrainingResult.h"

/**
 * @file AiSolver/Training/ReplayExport.h
 * @brief Export d'un rejeu déterministe au format v1 (`LOT-ANNEXE-11`, `EX-IA-012`), et point
 * d'entrée minimal assemblant `LevelTrainingSession` → `replayBestIndividual` → `exportReplay`.
 */

namespace aisolver::training {

/// Raison d'un échec d'export — jamais d'exception (`EX-NFR-040`).
enum class ReplayExportError {
    None,
    /// L'entraînement source n'a pas résolu le niveau (`TrainingResult::solved == false`) : un
    /// fichier de rejeu n'est jamais écrit pour un résultat non résolu (décision de cadrage de
    /// l'épic).
    NotSolved,
    /// `writeReplay` (`LOT-ANNEXE-07`) a échoué (chemin invalide, disque plein…).
    WriteFailed,
};

/// Issue d'un export : succès, ou échec explicite documenté par `ReplayExportError`.
struct ReplayExportResult {
    bool exported = false;
    ReplayExportError error = ReplayExportError::None;
};

/**
 * @brief Écrit `replay` au format de rejeu v1 (`LOT-ANNEXE-07`, `Source/AiSolver/Replay`), sauf si
 * `solved` est faux (aucun fichier écrit dans ce cas — refus explicite, pas une écriture partielle).
 * @param replay        Rejeu produit par `replayBestIndividual` (séquence, statut, récompense).
 * @param solved        `TrainingResult::solved` de l'entraînement ayant produit `replay`.
 * @param levelPath     Chemin du niveau source ; seul le nom de fichier est conservé dans le
 *                      rejeu exporté (`ReplayFile::levelPath`, chemin relatif et portable).
 * @param outputPath    Chemin du fichier de rejeu à (re)créer.
 * @param algorithmName Nom de l'algorithme ayant produit ce rejeu (ex. `"evolutionnaire"`).
 * @param seed          Graine de l'entraînement source.
 * @return Le résultat de l'export, jamais d'exception.
 */
[[nodiscard]] ReplayExportResult exportReplay(const DeterministicReplayResult& replay, bool solved,
                                              const std::filesystem::path& levelPath,
                                              const std::filesystem::path& outputPath,
                                              const std::string& algorithmName, std::uint64_t seed);

/// Issue du point d'entrée minimal : le résultat de l'entraînement et celui de l'export tenté.
struct TrainAndExportOutcome {
    TrainingResult trainingResult;
    ReplayExportResult exportResult;
};

/**
 * @brief Point d'entrée minimal : entraîne `levelPath` (`LevelTrainingSession`), rejoue le meilleur
 * individu final (`replayBestIndividual`), puis exporte la séquence obtenue (`exportReplay`).
 *
 * Volontairement minimal (pas d'options, pas de reprise) : un usage manuel réel de ce lot, l'outil
 * ergonomique (CLI) restant hors périmètre (`LOT-ANNEXE-19`, génération 5).
 * @param levelPath      Chemin du fichier de niveau **unique** à entraîner.
 * @param topology       Topologie du réseau de chaque individu.
 * @param config         Paramètres de l'algorithme évolutionniste.
 * @param stopping       Critère d'arrêt de la session (`LevelTrainingSession::StoppingConfig`).
 * @param seed           Graine explicite de tout l'aléatoire de l'entraînement.
 * @param statsCsvPath   Chemin du CSV de journalisation (`LOT-ANNEXE-09`).
 * @param replayOutputPath Chemin du fichier de rejeu à écrire si l'entraînement résout le niveau.
 * @param environmentConfig Configuration de l'environnement (budget de pas dur, seuil de
 *                     progression) ; valeur par défaut de `LOT-ANNEXE-05` si omise.
 */
[[nodiscard]] TrainAndExportOutcome trainLevelAndExportReplay(
    const std::filesystem::path& levelPath, const evolutionary::NetworkTopology& topology,
    const evolutionary::EvolutionaryConfig& config, const StoppingConfig& stopping,
    std::uint64_t seed, const std::filesystem::path& statsCsvPath,
    const std::filesystem::path& replayOutputPath, EnvironmentConfig environmentConfig = {});

}  // namespace aisolver::training
