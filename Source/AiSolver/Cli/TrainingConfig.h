// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Training/Dqn/DqnTrainer.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryConfig.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/LevelTrainingSession.h"
#include "AiSolver/Training/PolicyGradientTuning.h"

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
    std::string algorithmId = "evo";
    /// Évolutionniste (`LOT-ANNEXE-10`) : taille de population, mutation, croisement, sélection.
    training::evolutionary::EvolutionaryConfig evolutionary{};
    /// Critère d'arrêt de la session évolutionniste (`LOT-ANNEXE-11`).
    training::StoppingConfig stopping{};
    /// Taille de la couche cachée du réseau de politique (tout algorithme).
    std::size_t hiddenSize = training::evolutionary::DEFAULT_HIDDEN_SIZE;
    /// Facteur d'actualisation (REINFORCE/acteur-critique/DQN).
    float gamma = training::DEFAULT_GAMMA;
    /// Taux d'apprentissage (tout algorithme de gradient).
    ///
    /// Accordé au défaut `adam` ci-dessous : `0,01` est un pas raisonnable pour SGD, beaucoup trop
    /// grand pour un pas déjà normalisé par l'amplitude récente du gradient.
    float learningRate = 0.003f;
    /// Taux d'apprentissage du **critique** (acteur-critique seulement).
    ///
    /// Deux ordres de grandeur au-dessus de celui de la politique, et ce n'est pas un réglage
    /// arbitraire : le critique doit produire une valeur qui couvre l'amplitude des **retours**
    /// (une centaine de points, `RewardConfig::completionBonus`), là où la politique n'a qu'à
    /// déplacer des logits de l'ordre de l'unité. Avec Adam, le déplacement d'un poids par pas est
    /// borné par le taux, et l'observation est une entrée creuse (surtout des zéros) dont la
    /// plupart des poids ne reçoivent aucun gradient : à `0,003`, le critique n'atteint jamais
    /// l'échelle de sa cible. Mesuré sur le niveau de contrôle, politique figée, `80` épisodes :
    /// l'erreur du critique ne baisse pas à `0,003` ni à `0,1`, elle baisse à `0,5`
    /// (`test_actor_critic_trainer.cpp`).
    float criticLearningRate = 0.5f;
    /// `"sgd"` ou `"adam"` (tout algorithme de gradient).
    ///
    /// `adam` par défaut, et non `sgd` : un policy gradient produit des gradients d'amplitude très
    /// inégale d'un paramètre à l'autre (l'essentiel des poids voit une entrée one-hot rarement
    /// active), ce qu'un pas fixe ne rattrape pas. Mesure sur `demo-saut.json`, `600` épisodes :
    /// `388` victoires avec Adam, contre quelques dizaines avec SGD à taux fixe.
    std::string optimizer = "adam";
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
    std::size_t dqnEpsilonDecaySteps = training::DqnConfig{}.epsilonDecaySteps;
    /// Réglages partagés par REINFORCE et acteur-critique (lot d'épisodes, entropie, écrêtage,
    /// répétition d'action) — voir `training::PolicyGradientTuning`.
    training::PolicyGradientTuning tuning{};
    /// Budget de pas d'un épisode ; `0` = dérivé du niveau (`estimateStepBudget`, `StepBudget.h`).
    ///
    /// Exposé parce que c'est la borne qui décide de ce qu'un agent peut seulement *voir* du
    /// niveau : la valeur fixe précédente (`3 000`) était inférieure aux ~`4 000` pas que demande
    /// le tracé de référence de `demo-final.json`, et aucun réglage ne permettait de la relever.
    int maxSteps = 0;
    /// Seuil de blocage d'un épisode ; `0` = dérivé du budget (`stuckThresholdForBudget`).
    int stuckThreshold = 0;
};

/// Surcharges d'arguments individuels de la ligne de commande (priorité la plus haute, voir
/// `loadTrainingConfig`).
struct CommandLineOverrides {
    std::optional<std::size_t> populationSize;
    std::optional<float> mutationRate;
    std::optional<std::size_t> episodes;
    std::optional<float> learningRate;
    std::optional<float> criticLearningRate;
    std::optional<float> gamma;
    std::optional<std::string> optimizer;
    /// Topologie du réseau de politique, commune à tous les algorithmes. Surchargeable au même
    /// titre que le reste : un modèle n'est rechargeable que sur la topologie qui l'a produit
    /// (voir `hiddenSizeForModel`), donc la valeur d'un run doit pouvoir être choisie sans passer
    /// par un fichier de configuration.
    std::optional<std::size_t> hiddenSize;
    /// Évolutionniste (voir `training::evolutionary::EvolutionaryConfig`).
    std::optional<int> tournamentSize;
    std::optional<float> mutationStrength;
    /// Critère d'arrêt évolutionniste (voir `training::StoppingConfig`).
    std::optional<int> maxGenerations;
    std::optional<int> requiredConsecutiveSuccesses;
    /// Hyperparamètres DQN (voir `TrainingConfig`), pertinents uniquement pour `--algo avance`.
    std::optional<std::size_t> dqnReplayCapacity;
    std::optional<std::size_t> dqnBatchSize;
    std::optional<std::size_t> dqnWarmupSize;
    std::optional<std::size_t> dqnUpdatePeriodSteps;
    std::optional<std::size_t> dqnTargetSyncPeriodSteps;
    std::optional<float> dqnEpsilonStart;
    std::optional<float> dqnEpsilonEnd;
    std::optional<std::size_t> dqnEpsilonDecaySteps;
    /// Réglages de policy gradient (voir `training::PolicyGradientTuning`), pertinents pour
    /// `--algo pg`/`ac`, et `actionRepeat` aussi pour `--algo avance`.
    std::optional<std::size_t> batchEpisodes;
    std::optional<float> entropyCoefficient;
    std::optional<float> gradientClipNorm;
    std::optional<int> actionRepeat;
    std::optional<float> explorationFloor;
    /// Probabilité de croisement (évolutionniste).
    std::optional<float> crossoverRate;
    /// Budget de pas et seuil de blocage de l'environnement ; `0` = dérivés du niveau.
    std::optional<int> maxSteps;
    std::optional<int> stuckThreshold;
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

/**
 * @brief Configuration d'environnement (budget de pas, seuil de blocage) avec laquelle @p modelPath
 *        a été entraîné.
 *
 * Même raison d'être que `hiddenSizeForModel` : un rejeu produit sous un budget plus court que
 * celui de l'entraînement serait tronqué avant la fin du niveau, et le modèle déclaré incapable de
 * le résoudre alors qu'il le résout. La valeur est donc relue dans le `config.json` déposé à côté
 * du modèle.
 * @param modelPath Chemin du fichier de poids ; son dossier est celui du run.
 * @return La configuration lue, ou la configuration par défaut (budget dérivé du niveau) si le
 *         `config.json` est absent — seule valeur supposable alors.
 */
[[nodiscard]] EnvironmentConfig environmentConfigForModel(const std::filesystem::path& modelPath);

}  // namespace aisolver::cli
