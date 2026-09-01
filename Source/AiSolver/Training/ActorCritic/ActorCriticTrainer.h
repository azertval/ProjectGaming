// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <string>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Optim/IOptimizer.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/ActorCritic/CriticNetwork.h"
#include "AiSolver/Training/PolicyGradient/TrajectoryCollector.h"
#include "AiSolver/Training/PolicyGradientTuning.h"

/**
 * @file AiSolver/Training/ActorCritic/ActorCriticTrainer.h
 * @brief Boucle d'entraînement acteur-critique par épisodes (`LOT-ANNEXE-13`, TACHE-03,
 * `EX-IA-014`) — même ossature de boucle que `ReinforceTrainer` (`LOT-ANNEXE-12`), sans lien de
 * code avec lui, augmentée d'un second réseau et d'un second optimiseur.
 */

namespace aisolver::training {

/// Paramètres d'un run acteur-critique : mêmes défauts que `ReinforceConfig` (`LOT-ANNEXE-12`),
/// pour rester le niveau de contrôle comparable prescrit par TACHE-04.
struct ActorCriticConfig {
    float gamma = DEFAULT_GAMMA;
    std::uint64_t seedBase = 0;
    PolicyGradientTuning tuning{};
};

/**
 * @brief Assemble collecte de trajectoire (`TrajectoryCollector`, `LOT-ANNEXE-12`, inchangée),
 * calcul de retour (`computeReturns`, `LOT-ANNEXE-12`, inchangé), avantage (`computeAdvantages`),
 * perte de politique (`computeActorCriticLoss`) et perte du critique (`computeCriticLoss`) en une
 * boucle par épisode optimisant les **deux** réseaux, chacun via son propre `optim::IOptimizer`.
 *
 * Un seul niveau à la fois (décision transverse du programme), même convention de propriété que
 * `ReinforceTrainer` : ne possède ni les réseaux ni les optimiseurs (références), seul propriétaire
 * de la boucle elle-même.
 *
 * **Journalisation de la perte du critique** : plutôt que d'ajouter une colonne au format CSV
 * partagé (`Stats/TrainingStatsRecorder.h`, volontairement agnostique de tout algorithme, cf. son
 * en-tête), la perte du critique est journalisée dans un fichier CSV **séparé**, minimal
 * (`index,criticLoss`), optionnel. `TrainingStatsRecorder` continue de recevoir des lignes de
 * schéma strictement identique à `ReinforceTrainer` : `eval::ConvergenceComparator` (TACHE-04) peut
 * donc lire les runs des deux algorithmes sans aucun traitement spécial de colonnes.
 */
class ActorCriticTrainer {
public:
    /**
     * @param policy           Réseau de politique, entraîné en place.
     * @param policyOptimizer  Optimiseur de la politique (indépendant de celui du critique).
     * @param critic           Réseau critique, entraîné en place.
     * @param criticOptimizer  Optimiseur du critique (indépendant de celui de la politique).
     * @param environment      Environnement réutilisé à chaque épisode, réinitialisé par cette
     *                          classe en tout début d'épisode.
     * @param levelPath        Chemin du niveau **unique** joué par tout le run.
     * @param config           Facteur d'actualisation et graine de base.
     * @param recorder         Enregistreur CSV partagé (même schéma que `ReinforceTrainer`).
     * @param levelName        Nom du niveau, dupliqué dans chaque ligne journalisée.
     * @param criticLossCsvPath Chemin optionnel d'un CSV secondaire `index,criticLoss` ; absent
     *                          (`std::nullopt`) si l'appelant ne souhaite pas cette journalisation.
     */
    ActorCriticTrainer(nn::Network& policy, optim::IOptimizer& policyOptimizer,
                       CriticNetwork& critic, optim::IOptimizer& criticOptimizer,
                       HeadlessLevelEnvironment& environment, std::filesystem::path levelPath,
                       ActorCriticConfig config, TrainingStatsRecorder& recorder,
                       std::string levelName,
                       std::optional<std::filesystem::path> criticLossCsvPath = std::nullopt);

    /**
     * @brief Exécute `episodeCount` épisodes.
     *
     * Ordre des deux mises à jour non significatif (documenté dans l'épic) : l'avantage utilisé par
     * la perte de politique est une valeur détachée du graphe du critique, aucune dépendance entre
     * les deux passages arrière.
     * @param episodeCount Nombre d'épisodes à exécuter à la suite de ceux déjà joués.
     * @param updateCritic Si `false`, le critique continue de produire des avantages (`forward`)
     *                      mais son optimiseur n'est jamais appelé (`step`) : sert à vérifier que
     *                      les deux optimisations sont bien indépendantes (test dédié,
     *                      `LOT-ANNEXE-13` TACHE-03).
     * @param shouldStop Vérifié au début de chaque épisode (`LOT-ANNEXE-21`) ; si présent et
     *        renvoie `true`, l'exécution s'arrête avant cet épisode. `nullptr` (défaut) :
     *        comportement inchangé.
     */
    void run(std::size_t episodeCount, bool updateCritic = true,
             const std::function<bool()>& shouldStop = {});

    /// @return Le nombre d'épisodes déjà exécutés par ce trainer.
    [[nodiscard]] int episodeIndex() const noexcept {
        return _episodeIndex;
    }

private:
    nn::Network& _policy;
    optim::IOptimizer& _policyOptimizer;
    CriticNetwork& _critic;
    optim::IOptimizer& _criticOptimizer;
    HeadlessLevelEnvironment& _environment;
    std::filesystem::path _levelPath;
    ActorCriticConfig _config;
    TrainingStatsRecorder& _recorder;
    std::string _levelName;
    TrajectoryCollector _collector;
    std::optional<std::ofstream> _criticLossCsv;
    int _episodeIndex = 0;
};

}  // namespace aisolver::training
