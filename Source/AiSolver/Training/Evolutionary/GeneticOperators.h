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
 * @brief Croisement par moyenne : le réseau enfant a, pour chaque poids, la moyenne des poids
 * correspondants des deux parents (moyenne élément par élément des `Tensor` de poids).
 *
 * Déterministe une fois les deux parents choisis : @p rng n'intervient jamais dans le calcul de la
 * moyenne, seulement pour matérialiser le réseau enfant via `buildNetwork` (poids immédiatement
 * écrasés ci-dessous) — conservé en paramètre pour une éventuelle variante future (hors périmètre
 * ici), et déviation pragmatique du signature de l'épic (`topology` ajouté : `nn::Network` n'est
 * pas introspectable après construction, la topologie doit être fournie explicitement).
 */
[[nodiscard]] Individual crossover(const Individual& parentA, const Individual& parentB,
                                   const NetworkTopology& topology, Rng& rng);

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
