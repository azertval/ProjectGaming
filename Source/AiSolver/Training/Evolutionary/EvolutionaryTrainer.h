// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "AiSolver/Env/Episode.h"
#include "AiSolver/Stats/TrainingStatsRecorder.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryConfig.h"
#include "AiSolver/Training/Evolutionary/Population.h"

/**
 * @file AiSolver/Training/Evolutionary/EvolutionaryTrainer.h
 * @brief Boucle de génération de l'algorithme évolutionniste (`LOT-ANNEXE-10`, `EX-IA-011`).
 */

namespace aisolver::training::evolutionary {

/**
 * @brief Assemble population, évaluation de fitness et opérateurs génétiques en une boucle
 * exécutable, journalisée via `TrainingStatsRecorder`.
 *
 * Un run = un niveau, toujours (décision de cadrage de l'épic) : construit à partir d'un
 * `HeadlessLevelEnvironment&` déjà existant et du chemin d'**un seul** niveau, jamais d'une liste.
 * Agnostique de tout critère d'arrêt métier : `runGeneration()` exécute exactement une génération
 * par appel, à l'appelant (`LOT-ANNEXE-11`) de décider combien de fois l'invoquer et pourquoi
 * s'arrêter.
 */
class EvolutionaryTrainer {
public:
    /**
     * @param topology   Topologie partagée par tous les individus de la population.
     * @param config     Paramètres de l'algorithme (taille de population, tournoi, mutation).
     * @param environment Environnement réutilisé à chaque génération, déjà construit pour un
     *                    niveau donné (jamais reconstruit ni changé de niveau par cette classe).
     * @param levelPath  Chemin du niveau joué par toutes les évaluations (`FitnessEvaluator`).
     * @param seed       Graine explicite du `Rng` interne — toute la stochasticité de
     *                   l'entraînement (initialisation, sélection, mutation) en dérive.
     * @param recorder   Enregistreur CSV, une ligne par génération (`Stats/TrainingStatsRecorder`).
     * @param levelName  Nom du niveau, dupliqué dans chaque ligne journalisée (`TrainingStatsRow`).
     */
    EvolutionaryTrainer(NetworkTopology topology, EvolutionaryConfig config,
                        HeadlessLevelEnvironment& environment, std::filesystem::path levelPath,
                        std::uint64_t seed, TrainingStatsRecorder& recorder, std::string levelName);

    /**
     * @brief Exécute exactement une génération : (1) évalue tous les individus, (2) journalise la
     * génération courante, (3) construit la génération suivante dans un double tampon (élite
     * clonée sans réévaluation, reste par sélection + croisement + mutation), (4) échange les
     * tampons.
     */
    void runGeneration();

    /// @return Le meilleur individu connu de la génération courante (fitness maximal).
    [[nodiscard]] const Individual& bestIndividual() const;

    /// @return Le statut de fin d'épisode (`LOT-ANNEXE-08`) du meilleur individu de la dernière
    /// génération exécutée (`runGeneration()`) — donne à l'appelant (`LOT-ANNEXE-11`) le moyen de
    /// distinguer un champion qui progresse d'un champion qui résout réellement le niveau, sans
    /// réévaluer l'individu une seconde fois.
    [[nodiscard]] EpisodeStatus lastChampionStatus() const noexcept {
        return _lastChampionStatus;
    }

    /// @return Le nombre d'appels effectifs à `runGeneration()` déjà réalisés.
    [[nodiscard]] int generationIndex() const noexcept {
        return _generationIndex;
    }

private:
    NetworkTopology _topology;
    EvolutionaryConfig _config;
    HeadlessLevelEnvironment& _environment;
    std::filesystem::path _levelPath;
    std::uint64_t _seed;
    Rng _rng;
    TrainingStatsRecorder& _recorder;
    std::string _levelName;
    Population _population;
    int _generationIndex = 0;
    EpisodeStatus _lastChampionStatus = EpisodeStatus::Ongoing;
};

}  // namespace aisolver::training::evolutionary
