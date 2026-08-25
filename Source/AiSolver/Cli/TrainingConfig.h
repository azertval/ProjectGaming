// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

#include "AiSolver/Training/Evolutionary/EvolutionaryConfig.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"

/**
 * @file AiSolver/Cli/TrainingConfig.h
 * @brief Hyperparamètres résolus d'un run `aisolver-cli train`, traçables de bout en bout
 * (`LOT-ANNEXE-19`, TACHE-02, `EX-IA-020`).
 */

namespace aisolver::cli {

/**
 * @brief Hyperparamètres agrégés de toutes les familles d'algorithmes exposées par `train`, chaque
 * champ portant une valeur par défaut documentée (reprise des structures déjà cadrées par les lots
 * amont, jamais réinventée ici) ; seuls les champs pertinents pour `--algo` choisi sont
 * effectivement utilisés.
 */
struct TrainingConfig {
    /// Identifiant court de l'algorithme du run (`evo`/`pg`/`ac`/`avance`).
    ///
    /// Ne vient pas d'un fichier de configuration mais de la ligne de commande (`--algo`) ou de
    /// l'écran Mode IA ; il est journalisé ici parce qu'un run n'est **pas relisible sans lui** :
    /// le modèle sauvegardé ne porte que des poids, et c'est l'algorithme qui dit sur quelle
    /// topologie les recharger.
    std::string algo = "evo";
    /// Évolutionniste (`LOT-ANNEXE-10`) : taille de population, mutation, sélection.
    training::evolutionary::EvolutionaryConfig evolutionary{};
    /// Critère d'arrêt de la session évolutionniste (`LOT-ANNEXE-11`).
    training::StoppingConfig stopping{};
    /// Taille de la couche cachée du réseau de politique (tout algorithme).
    std::size_t hiddenSize = training::evolutionary::DEFAULT_HIDDEN_SIZE;
    /// Facteur d'actualisation (REINFORCE/acteur-critique/DQN).
    float gamma = 0.99f;
    /// Taux d'apprentissage (tout algorithme de gradient).
    float learningRate = 0.01f;
    /// `"sgd"` ou `"adam"` (tout algorithme de gradient).
    std::string optimizer = "sgd";
    /// Budget d'épisodes (REINFORCE/acteur-critique/DQN — pas de session à critère de résolution
    /// dédiée pour ces algorithmes, contrairement à l'évolutionniste).
    std::size_t episodes = 300;
    /// Capacité de la mémoire de rejeu (DQN).
    std::size_t dqnReplayCapacity = 2000;
    /// Taille du mini-lot échantillonné (DQN).
    std::size_t dqnBatchSize = 32;
    /// Transitions minimales avant la première mise à jour (DQN).
    std::size_t dqnWarmupSize = 32;
    /// Période, en pas, entre deux mises à jour de poids (DQN).
    std::size_t dqnUpdatePeriodSteps = 1;
    /// Période, en pas, entre deux synchronisations du réseau cible (DQN).
    std::size_t dqnTargetSyncPeriodSteps = 200;
    /// Borne haute d'exploration en début de run (DQN).
    float dqnEpsilonStart = 1.0f;
    /// Borne basse d'exploration (DQN).
    float dqnEpsilonEnd = 0.05f;
    /// Nombre de pas de décroissance de l'exploration (DQN).
    std::size_t dqnEpsilonDecaySteps = 2000;
};

/// Surcharges d'arguments individuels de la ligne de commande (priorité la plus haute, voir
/// `loadTrainingConfig`).
struct CommandLineOverrides {
    std::optional<std::size_t> populationSize;
    std::optional<float> mutationRate;
    std::optional<std::size_t> episodes;
    std::optional<float> learningRate;
    std::optional<float> gamma;
    std::optional<std::string> optimizer;
};

/**
 * @brief Résout la configuration effective d'un run : défauts documentés → fichier `--config`
 * (s'il est fourni) → arguments individuels (@p overrides, priorité la plus haute).
 * @param configFile Chemin d'un fichier JSON optionnel de configuration ; `std::nullopt` si absent
 *        (`--config` non fourni).
 * @param overrides Surcharges d'arguments individuels, appliquées après le fichier.
 * @return La configuration entièrement résolue, jamais partielle — condition de traçabilité
 *         (écrite dans les métadonnées d'un run par `writeTrainingConfigJson`).
 */
[[nodiscard]] TrainingConfig loadTrainingConfig(
    const std::optional<std::filesystem::path>& configFile, const CommandLineOverrides& overrides);

/**
 * @brief Écrit la configuration résolue @p config dans @p path (`config.json` du dossier de run),
 * pour qu'un run passé reste reproductible sans connaître les valeurs par défaut du code de
 * l'époque.
 * @param config Configuration résolue (défaut → fichier → arguments) à journaliser.
 * @param path Chemin du fichier à (re)créer ; les dossiers parents manquants sont créés.
 * @return `false` si le fichier ne peut pas être ouvert en écriture, `true` sinon (jamais
 *         d'exception).
 */
[[nodiscard]] bool writeTrainingConfigJson(const TrainingConfig& config,
                                           const std::filesystem::path& path);

/**
 * @brief Taille de couche cachée avec laquelle @p modelPath a été entraîné.
 *
 * Un modèle n'est rechargeable que sur la topologie exacte qui l'a produit : `nn::loadWeights`
 * compare les formes et refuse le moindre écart. Or l'entraînement respecte `hiddenSize`, qu'un run
 * peut avoir changé : supposer la valeur par défaut à la relecture rend tout modèle entraîné
 * autrement définitivement illisible. La taille est donc relue dans le `config.json` déposé à côté
 * du modèle — c'est ce à quoi sert ce fichier.
 * @param modelPath Chemin du fichier de poids ; son dossier est celui du run.
 * @return La taille lue, ou `training::evolutionary::DEFAULT_HIDDEN_SIZE` si le `config.json` est
 *         absent (modèle déplacé hors de son dossier de run) — seule valeur supposable alors.
 */
[[nodiscard]] std::size_t hiddenSizeForModel(const std::filesystem::path& modelPath);

}  // namespace aisolver::cli
