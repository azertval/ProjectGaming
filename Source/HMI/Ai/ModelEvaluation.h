// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <cstdint>
#include <functional>
#include <optional>

/**
 * @file HMI/Ai/ModelEvaluation.h
 * @brief Évaluation d'un modèle entraîné depuis l'onglet Validation & sauvegarde
 * (`LOT-ANNEXE-21`, `EX-IA-022`) : seule fonction libre de `HMI/Ai` en dehors de
 * `TrainingWorker` — même portée d'amendement documentée dans l'epic du lot (`HMI/Ai`
 * uniquement, jamais `HMI/Interface` directement).
 *
 * Délègue à `aisolver::eval::BenchmarkRunner::run` (`LOT-ANNEXE-15`) — même résultat qu'un appel
 * direct dans les mêmes conditions (répétitions, décodage Argmax), critère hérité de
 * `aisolver::cli::runEvaluate` (`LOT-ANNEXE-19`).
 */

namespace hmi {

/// Résultat d'une évaluation, mêmes champs que `aisolver::eval::BenchmarkResult` en types
/// simples (`HMI/Interface` n'a pas besoin de connaître le type `AiSolver` sous-jacent).
struct EvaluationOutcome {
    double successRate = 0.0;
    double meanStepsOnSuccess = 0.0;
    /// Pas moyen sur **toutes** les répétitions, réussites et échecs confondus : un modèle qui
    /// échoue vite et un modèle qui échoue au bout du budget se distinguent ici, jamais dans
    /// `meanStepsOnSuccess`.
    double meanStepsAll = 0.0;
    double stepVariance = 0.0;
    /// Répétitions réellement jouées — inférieur au nombre demandé si l'évaluation a été annulée.
    int repetitionsRun = 0;
};

/// Paramètres d'une évaluation, mêmes champs que `aisolver::eval::BenchmarkConfig` en types
/// simples ; défauts identiques aux siens, pour qu'une évaluation lancée sans rien régler mesure
/// exactement ce que mesure `aisolver-cli evaluate`.
struct EvaluationRequest {
    QString modelPath;
    QString levelPath;
    QString algorithmId;  ///< `"evo"`, `"pg"`, `"ac"` ou `"avance"`.
    int repetitions = 30;
    /// Budget de pas par épisode ; `0` = dérivé du niveau (`eval::BenchmarkConfig`).
    int maxStepsPerEpisode = 0;
    std::uint64_t seed = 0;
    /// `false` : décodage `Argmax` (déterministe, défaut de la sous-commande `evaluate`).
    bool stochasticDecoding = false;
};

/**
 * @brief Évalue le modèle de @p request sur son niveau, selon ses répétitions et son mode de
 * décodage.
 * @param request      Modèle, niveau et paramètres de campagne (voir `EvaluationRequest`).
 * @param onRepetition Observateur optionnel de progression/interruption, transmis tel quel à
 *                     `eval::BenchmarkRunner::run` — renvoyer `false` interrompt la campagne.
 * @return `std::nullopt` si le modèle ne se charge pas (chemin invalide, format incompatible).
 */
[[nodiscard]] std::optional<EvaluationOutcome> evaluateModel(
    const EvaluationRequest& request,
    const std::function<bool(int completed, int total)>& onRepetition = {});

/**
 * @brief Écrit le rapport CSV d'une évaluation dans @p csvPath, au format de
 * `aisolver-cli evaluate --report`.
 *
 * Vit ici plutôt que dans l'écran parce que le format appartient à `eval::BenchmarkReport`, que
 * `HMI/Interface` ne référence pas (amendement de `LOT-ANNEXE-18`, limité à `HMI/Ai`) — et parce
 * qu'un CSV formaté à la main dans l'écran divergerait du rapport de la ligne de commande.
 * @param request  Requête ayant produit @p outcome : nomme l'algorithme et le niveau mesurés.
 * @param outcome  Résultat affiché à l'écran.
 * @param csvPath  Fichier à (re)créer.
 * @return `false` si le fichier ne peut pas être écrit.
 */
[[nodiscard]] bool writeEvaluationReport(const EvaluationRequest& request,
                                         const EvaluationOutcome& outcome, const QString& csvPath);

/// Issue d'un export de rejeu, distinguant les deux échecs que l'écran doit savoir expliquer
/// différemment.
enum class ReplayExportOutcome {
    Exported,   ///< Rejeu écrit.
    NotSolved,  ///< Le modèle ne résout pas ce niveau : rien n'est écrit (voir
                ///< `exportModelReplay`).
    Failed,     ///< Modèle illisible, niveau introuvable, ou écriture impossible.
};

/**
 * @brief Rejoue @p modelPath en mode `Argmax` sur @p levelPath et écrit le rejeu dans
 * @p outputPath — équivalent exact de `aisolver-cli export-replay`.
 *
 * Remplace la simple copie du `replay.json` d'un run : celle-ci ne pouvait rien produire pour un
 * niveau autre que celui du run, ni pour un run dont le fichier manque, alors que le modèle
 * sauvegardé, lui, est toujours rejouable.
 *
 * Un rejeu **publié** ne peut être qu'une réussite validée (décision de cadrage de
 * `LOT-ANNEXE-11`, appliquée aussi par `aisolver-cli`) : un modèle qui ne termine pas le niveau
 * produit `NotSolved` et **aucun** fichier.
 * @param request  Modèle, niveau, algorithme et graine ; `repetitions`/`maxStepsPerEpisode` et le
 *                 mode de décodage sont ignorés (un rejeu est un unique épisode déterministe).
 * @param outputPath Fichier de rejeu à écrire.
 */
[[nodiscard]] ReplayExportOutcome exportModelReplay(const EvaluationRequest& request,
                                                    const QString& outputPath);

}  // namespace hmi
