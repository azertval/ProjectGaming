// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <string>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Optim/IOptimizer.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/Advanced/QNetwork.h"
#include "AiSolver/Training/Advanced/ReplayBuffer.h"

/**
 * @file AiSolver/Training/Advanced/DqnTrainer.h
 * @brief Boucle d'entraînement DQN par pas, intégrée au harnais existant (`LOT-ANNEXE-14`,
 * TACHE-01/TACHE-02, `EX-IA-015`).
 */

namespace aisolver::training {

/**
 * @brief Paramètres d'un run DQN -- structure explicite dédiée (pas de constante codée en dur dans
 * le trainer), même esprit que `ReinforceConfig`/`ActorCriticConfig` (`LOT-ANNEXE-12`/`13`).
 */
struct DqnConfig {
    std::size_t hiddenSize = QNetwork::kDefaultHiddenSize;
    /// Capacité de la mémoire de rejeu (`ReplayBuffer`).
    std::size_t replayCapacity = 2000;
    /// Taille du mini-lot échantillonné à chaque mise à jour.
    std::size_t batchSize = 32;
    /// Nombre minimal de transitions stockées avant la première mise à jour (évite d'apprendre sur
    /// un tampon quasi vide, dominé par les toutes premières transitions).
    std::size_t warmupSize = 32;
    /// Période, en pas de simulation, entre deux mises à jour de poids (`1` = une mise à jour par
    /// pas, une fois le tampon suffisamment rempli).
    std::size_t updatePeriodSteps = 1;
    /// Période, en pas de simulation, entre deux synchronisations du réseau cible.
    std::size_t targetSyncPeriodSteps = 200;
    float gamma = 0.99f;
    /// Borne haute d'exploration (probabilité d'action aléatoire) au tout début du run.
    float epsilonStart = 1.0f;
    /// Borne basse d'exploration, atteinte après `epsilonDecaySteps` pas puis conservée.
    float epsilonEnd = 0.05f;
    /// Nombre de pas sur lequel `epsilon` décroît linéairement de `epsilonStart` à `epsilonEnd`.
    std::size_t epsilonDecaySteps = 2000;
    /// Graine explicite du générateur pseudo-aléatoire interne (exploration `epsilon`-greedy et
    /// échantillonnage du `ReplayBuffer`) : un flux **continu** pour tout le run, pas dérivé par
    /// épisode (décision de cadrage -- contrairement à `TrajectoryCollector`, DQN n'a pas de
    /// frontière d'épisode pertinente pour l'aléatoire : l'exploration et l'échantillonnage du
    /// tampon de rejeu traversent librement les limites d'épisode).
    std::uint64_t seedBase = 0;
};

/**
 * @brief Assemble `QNetwork` (principal + cible), `ReplayBuffer`, `computeDqnLoss` et un optimiseur
 * (`optim::IOptimizer`, `LOT-ANNEXE-04`) en une boucle par pas, journalisée par épisode
 * (`TrainingStatsRecorder`, `LOT-ANNEXE-09`) exactement comme
 * `ReinforceTrainer`/`ActorCriticTrainer`.
 *
 * Un seul niveau à la fois (décision transverse du programme), un seul `HeadlessLevelEnvironment`
 * construit pour toute la durée du run : aucune API ne permet d'en substituer un autre en cours de
 * run (même contrat que `ReinforceTrainer`).
 *
 * **Journalisation** : chaque **épisode** de jeu (victoire, échec, timeout ou blocage, `Episode.h`)
 * produit une ligne dans `TrainingStatsRecorder`, avec le même schéma de colonnes que
 * `ReinforceTrainer`/`ActorCriticTrainer` -- les mises à jour de poids surviennent à une fréquence
 * différente (par mini-lot de pas, pas par épisode), mais ceci ne change ni la fréquence ni le
 * schéma de la journalisation partagée (décision de cadrage de l'épic, TACHE-02). Les colonnes
 * spécifiques à DQN (taille du `ReplayBuffer`, `epsilon` courant en fin d'épisode) sont
 * journalisées dans un CSV secondaire optionnel (`index,replayBufferSize,epsilon`), même patron que
 * la perte du critique de `ActorCriticTrainer` -- jamais une colonne ajoutée au format partagé.
 */
class DqnTrainer {
public:
    /**
     * @param mainNetwork    Réseau principal, entraîné en place.
     * @param targetNetwork  Réseau cible, synchronisé périodiquement depuis @p mainNetwork ; doit
     *                        avoir la même topologie (même `inputSize`/`hiddenSize`).
     * @param optimizer      Optimiseur appliqué au seul réseau principal.
     * @param environment    Environnement réutilisé pour tout le run, réinitialisé en début de
     *                        chaque épisode par cette classe.
     * @param levelPath      Chemin du niveau **unique** joué par tout le run.
     * @param config         Hyperparamètres DQN.
     * @param recorder       Enregistreur CSV partagé (même schéma que `ReinforceTrainer`).
     * @param levelName      Nom du niveau, dupliqué dans chaque ligne journalisée.
     * @param dqnStatsCsvPath Chemin optionnel du CSV secondaire `index,replayBufferSize,epsilon`.
     */
    DqnTrainer(QNetwork& mainNetwork, QNetwork& targetNetwork, optim::IOptimizer& optimizer,
               HeadlessLevelEnvironment& environment, std::filesystem::path levelPath,
               DqnConfig config, TrainingStatsRecorder& recorder, std::string levelName,
               std::optional<std::filesystem::path> dqnStatsCsvPath = std::nullopt);

    /// @brief Exécute `episodeCount` épisodes complets (chacun jusqu'à victoire/échec/timeout/
    /// blocage), en mettant à jour le réseau principal à la période configurée.
    /// @param episodeCount Nombre d'épisodes à exécuter à la suite de ceux déjà joués.
    /// @param shouldStop Vérifié au début de chaque épisode (`LOT-ANNEXE-21`) ; si présent et
    ///        renvoie `true`, l'exécution s'arrête avant cet épisode. `nullptr` (défaut) :
    ///        comportement inchangé.
    void run(std::size_t episodeCount, const std::function<bool()>& shouldStop = {});

    /// @return Le nombre d'épisodes déjà exécutés par ce trainer.
    [[nodiscard]] int episodeIndex() const noexcept {
        return _episodeIndex;
    }

    /// @return Le nombre total de pas de simulation déjà exécutés (toutes épisodes confondus).
    [[nodiscard]] std::size_t totalSteps() const noexcept {
        return _totalSteps;
    }

    /// @return `epsilon` courant, calculé à partir de `totalSteps()` et de la configuration.
    [[nodiscard]] float currentEpsilon() const noexcept;

private:
    QNetwork& _mainNetwork;
    QNetwork& _targetNetwork;
    optim::IOptimizer& _optimizer;
    HeadlessLevelEnvironment& _environment;
    std::filesystem::path _levelPath;
    DqnConfig _config;
    TrainingStatsRecorder& _recorder;
    std::string _levelName;
    ReplayBuffer _replayBuffer;
    Rng _rng;
    std::optional<std::ofstream> _dqnStatsCsv;
    int _episodeIndex = 0;
    std::size_t _totalSteps = 0;
};

}  // namespace aisolver::training
