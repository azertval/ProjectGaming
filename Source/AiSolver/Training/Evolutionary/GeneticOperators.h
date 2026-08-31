// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/Evolutionary/EvolutionaryConfig.h"
#include "AiSolver/Training/Evolutionary/Individual.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/Evolutionary/Population.h"

/**
 * @file AiSolver/Training/Evolutionary/GeneticOperators.h
 * @brief Sélection, croisement, mutation et élitisme (`LOT-ANNEXE-10`, `EX-IA-011`).
 *
 * Toute la stochasticité de ces opérateurs passe par le `Rng` transmis en paramètre, jamais une
 * source globale — condition nécessaire à la reproductibilité stricte exigée par ce lot.
 */

namespace aisolver::training::evolutionary {

/**
 * @brief Sélection par tournoi : tire `config.tournamentSize` individus au hasard (avec remise) via
 * @p rng, retient celui de fitness maximal du groupe.
 * @return Référence dans @p population (ne construit jamais de copie).
 */
[[nodiscard]] const Individual& selectParent(const Population& population,
                                             const EvolutionaryConfig& config, Rng& rng);

/**
 * @brief Croisement **uniforme** : chaque poids de l'enfant est repris, à pile ou face, de l'un ou
 * l'autre parent.
 *
 * Pas une moyenne. Moyenner deux réseaux ne combine pas ce qu'ils ont trouvé : les neurones cachés
 * n'ont aucune raison de se correspondre d'un réseau à l'autre, et la moyenne de deux solutions
 * également bonnes est ordinairement moins bonne que chacune. Un tirage par poids conserve au
 * moins des valeurs qui ont réellement fonctionné quelque part, au lieu d'en fabriquer une
 * troisième qui n'a fonctionné nulle part.
 *
 * Avec probabilité `1 - config.crossoverRate`, aucun croisement n'a lieu : l'enfant copie
 * @p parentA tel quel, et seule la mutation le fera diverger — c'est le seul chemin par lequel une
 * trouvaille survit intacte d'une génération à l'autre en dehors de l'unique élite.
 *
 * Écart assumé à la signature de l'épic : `topology` est ajouté en paramètre parce qu'un
 * `nn::Network` n'est pas introspectable après construction — la topologie doit être fournie
 * explicitement.
 */
[[nodiscard]] Individual crossover(const Individual& parentA, const Individual& parentB,
                                   const NetworkTopology& topology,
                                   const EvolutionaryConfig& config, Rng& rng);

/**
 * @brief Mutation en place : pour chaque poids du réseau, avec probabilité
 * `config.mutationRate`, ajoute un bruit gaussien `Rng::nextGaussian(0, config.mutationStrength)`.
 */
void mutate(Individual& individual, const EvolutionaryConfig& config, Rng& rng);

/**
 * @brief Élitisme : l'individu de fitness maximal de @p population.
 * @pre `population.size() > 0`.
 */
[[nodiscard]] const Individual& bestIndividual(const Population& population);

}  // namespace aisolver::training::evolutionary
