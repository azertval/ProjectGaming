// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "AiSolver/Training/Evolutionary/Individual.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

/**
 * @file AiSolver/Training/Evolutionary/Population.h
 * @brief Conteneur de taille fixe d'individus (`LOT-ANNEXE-10`, `EX-IA-011`).
 */

namespace aisolver::training::evolutionary {

/**
 * @brief Ensemble de taille fixe `N` d'individus de même topologie, poids indépendants.
 *
 * Taille fixée à la construction (pas de croissance/réduction dynamique) : simplifie la boucle de
 * génération (`EvolutionaryTrainer`), qui construit la génération suivante dans une `Population`
 * distincte (double tampon) plutôt que de muter celle-ci en place.
 */
class Population {
public:
    /**
     * @brief Construit `size` individus indépendants, poids tirés via @p rng.
     * @param topology Topologie partagée par tous les individus (seuls les poids diffèrent).
     * @param size     Nombre d'individus, fixé pour toute la durée de vie de l'objet.
     * @param rng      Source d'aléatoire déterministe, consommée séquentiellement individu par
     *                 individu (même graine ⇒ mêmes poids initiaux, individu par individu).
     */
    Population(NetworkTopology topology, std::size_t size, Rng& rng);

    /**
     * @brief Construit une population à partir d'individus déjà assemblés.
     *
     * Utilisé par `EvolutionaryTrainer` pour le double tampon de génération (élite clonée puis
     * enfants de sélection/croisement/mutation) : évite d'initialiser au hasard des individus
     * immédiatement écrasés.
     */
    Population(NetworkTopology topology, std::vector<Individual> individuals);

    [[nodiscard]] std::size_t size() const noexcept {
        return _individuals.size();
    }

    [[nodiscard]] Individual& individual(std::size_t index) {
        return _individuals.at(index);
    }

    [[nodiscard]] const Individual& individual(std::size_t index) const {
        return _individuals.at(index);
    }

    [[nodiscard]] const NetworkTopology& topology() const noexcept {
        return _topology;
    }

    /**
     * @brief Évalue tous les individus, séquentiellement, sur la même instance d'environnement.
     *
     * Chaque appel à `evaluateFitness` commence par un `reset()` explicite (voir
     * `FitnessEvaluator.h`) : aucun état ne fuit d'un individu au suivant, pas de parallélisation
     * (décision de cadrage de l'épic).
     * @return Une évaluation par individu, dans l'ordre de la population.
     */
    [[nodiscard]] std::vector<FitnessEvaluation> evaluateAll(
        HeadlessLevelEnvironment& environment, const std::filesystem::path& levelPath);

private:
    NetworkTopology _topology;
    std::vector<Individual> _individuals;
};

}  // namespace aisolver::training::evolutionary
