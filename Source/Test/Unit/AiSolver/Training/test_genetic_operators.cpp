// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_genetic_operators.cpp
 * @brief Tests unitaires de sélection/croisement/mutation/élitisme (LOT-ANNEXE-10, TACHE-03).
 */

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Training/Evolutionary/GeneticOperators.h"

using aisolver::Rng;
using aisolver::training::evolutionary::bestIndividual;
using aisolver::training::evolutionary::crossover;
using aisolver::training::evolutionary::EvolutionaryConfig;
using aisolver::training::evolutionary::Individual;
using aisolver::training::evolutionary::LayerTopology;
using aisolver::training::evolutionary::mutate;
using aisolver::training::evolutionary::NetworkTopology;
using aisolver::training::evolutionary::Population;
using aisolver::training::evolutionary::selectParent;

namespace {

NetworkTopology tinyTopology() {
    return NetworkTopology{
        LayerTopology{2, 3, aisolver::nn::WeightInitScheme::Xavier, nullptr},
    };
}

Population makePopulationWithFitness(const std::vector<float>& fitnesses) {
    Rng rng(4001);
    const NetworkTopology topology = tinyTopology();
    std::vector<Individual> individuals;
    individuals.reserve(fitnesses.size());
    for (const float fitness : fitnesses) {
        Individual individual(aisolver::training::evolutionary::buildNetwork(topology, rng));
        individual.fitness = fitness;
        individuals.push_back(std::move(individual));
    }
    return Population(topology, std::move(individuals));
}

}  // namespace

/**
 * @brief Le tournoi ne perd jamais un meilleur candidat présent dans le tirage.
 * \castest{<b>selectParent : retient toujours le meilleur du tirage.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Population de fitness `{1, 2, 3, 4, 5}`, tournoi de taille 200 (tirage avec remise :
 * la probabilité qu'au moins un tirage touche l'indice de fitness maximal est alors quasi certaine,
 * `1 - (4/5)^200 ≈ 1`).<br/>2. Appeler `selectParent`.<br/>
 * \tattendu L'individu retourné a le fitness maximal (`5`).}
 */
TEST(GeneticOperatorsTest, TournoiRetientLeMeilleurDuTirage) {
    Population population = makePopulationWithFitness({1.0f, 2.0f, 3.0f, 4.0f, 5.0f});
    EvolutionaryConfig config;
    config.tournamentSize = 200;
    Rng rng(4101);

    const Individual& selected = selectParent(population, config, rng);
    EXPECT_FLOAT_EQ(selected.fitness, 5.0f);
}

/**
 * @brief À seed `Rng` fixée, deux appels consécutifs de sélection sur la même population
 * produisent le même individu sélectionné.
 * \castest{<b>selectParent : déterminisme.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux `Rng` de même graine, même population.<br/>2. Appeler `selectParent` une fois
 * chacun.<br/>
 * \tattendu Même fitness sélectionné.}
 */
TEST(GeneticOperatorsTest, SelectionDeterministeASeedFixee) {
    Population population = makePopulationWithFitness({1.0f, 2.0f, 3.0f});
    EvolutionaryConfig config;

    Rng rngA(4201);
    Rng rngB(4201);
    const Individual& selectedA = selectParent(population, config, rngA);
    const Individual& selectedB = selectParent(population, config, rngB);

    EXPECT_FLOAT_EQ(selectedA.fitness, selectedB.fitness);
}

/**
 * @brief Le croisement de deux parents aux poids identiques produit un enfant aux poids
 * identiques.
 * \castest{<b>crossover : cas dégénéré, parents identiques.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Deux individus construits avec le même `Rng` (mêmes poids).<br/>2. Croiser.<br/>
 * \tattendu Chaque poids de l'enfant est identique à celui des parents.}
 */
TEST(GeneticOperatorsTest, CroisementParentsIdentiques) {
    const NetworkTopology topology = tinyTopology();
    Rng rngA(5001);
    Rng rngB(5001);
    Individual parentA(aisolver::training::evolutionary::buildNetwork(topology, rngA));
    Individual parentB(aisolver::training::evolutionary::buildNetwork(topology, rngB));

    Rng crossRng(9999);
    // Croisement force (taux a 1) : ces tests portent sur la COMBINAISON des deux parents,
    // pas sur la probabilite de la declencher.
    EvolutionaryConfig alwaysCross;
    alwaysCross.crossoverRate = 1.0f;
    Individual child = crossover(parentA, parentB, topology, alwaysCross, crossRng);

    auto childParams = child.network().parameters();
    auto parentParams = parentA.network().parameters();
    ASSERT_EQ(childParams.size(), parentParams.size());
    for (std::size_t index = 0; index < childParams.size(); ++index) {
        for (std::size_t element = 0; element < childParams[index]->value.size(); ++element) {
            EXPECT_FLOAT_EQ(childParams[index]->value.data()[element],
                            parentParams[index]->value.data()[element]);
        }
    }
}

/**
 * @brief Chaque poids de l'enfant est compris entre les poids correspondants des deux parents.
 * \castest{<b>crossover : propriété de borne.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Deux parents de poids différents.<br/>2. Croiser.<br/>
 * \tattendu Chaque poids enfant est entre le min et le max des poids parents correspondants.}
 */
TEST(GeneticOperatorsTest, CroisementProprieteDeBorne) {
    const NetworkTopology topology = tinyTopology();
    Rng rngA(6001);
    Rng rngB(6002);
    Individual parentA(aisolver::training::evolutionary::buildNetwork(topology, rngA));
    Individual parentB(aisolver::training::evolutionary::buildNetwork(topology, rngB));

    Rng crossRng(9998);
    // Croisement force (taux a 1) : ces tests portent sur la COMBINAISON des deux parents,
    // pas sur la probabilite de la declencher.
    EvolutionaryConfig alwaysCross;
    alwaysCross.crossoverRate = 1.0f;
    Individual child = crossover(parentA, parentB, topology, alwaysCross, crossRng);

    auto childParams = child.network().parameters();
    auto paramsA = parentA.network().parameters();
    auto paramsB = parentB.network().parameters();
    for (std::size_t index = 0; index < childParams.size(); ++index) {
        for (std::size_t element = 0; element < childParams[index]->value.size(); ++element) {
            const float a = paramsA[index]->value.data()[element];
            const float b = paramsB[index]->value.data()[element];
            const float childValue = childParams[index]->value.data()[element];
            EXPECT_LE(childValue, std::max(a, b) + 1e-6f);
            EXPECT_GE(childValue, std::min(a, b) - 1e-6f);
        }
    }
}

/**
 * @brief `mutationRate = 0` ne modifie aucun poids.
 * \castest{<b>mutate : taux nul, aucune modification.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Individu construit, poids copiés.<br/>2. `mutate` avec `mutationRate = 0`.<br/>
 * \tattendu Tous les poids restent strictement identiques.}
 */
TEST(GeneticOperatorsTest, MutationTauxNulNeModifieRien) {
    const NetworkTopology topology = tinyTopology();
    Rng buildRng(7001);
    Individual individual(aisolver::training::evolutionary::buildNetwork(topology, buildRng));
    const std::vector<float> before = [&] {
        std::vector<float> values;
        for (const auto& parameter : individual.network().parameters()) {
            for (std::size_t i = 0; i < parameter->value.size(); ++i) {
                values.push_back(parameter->value.data()[i]);
            }
        }
        return values;
    }();

    EvolutionaryConfig config;
    config.mutationRate = 0.0f;
    Rng mutationRng(7002);
    mutate(individual, config, mutationRng);

    std::size_t cursor = 0;
    for (const auto& parameter : individual.network().parameters()) {
        for (std::size_t i = 0; i < parameter->value.size(); ++i) {
            EXPECT_FLOAT_EQ(parameter->value.data()[i], before[cursor++]);
        }
    }
}

/**
 * @brief `mutationRate = 1` modifie tous les poids ; à seed fixée, la perturbation est
 * reproductible.
 * \castest{<b>mutate : taux plein, reproductible.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux individus identiques (même `Rng` de construction).<br/>2. `mutate` chacun avec
 * `mutationRate = 1` et des `Rng` de mutation de même graine.<br/>
 * \tattendu Tous les poids diffèrent de l'original ; les deux individus mutés sont identiques entre
 * eux.}
 */
TEST(GeneticOperatorsTest, MutationTauxPleinReproductible) {
    const NetworkTopology topology = tinyTopology();
    Rng buildRngA(8001);
    Rng buildRngB(8001);
    Individual individualA(aisolver::training::evolutionary::buildNetwork(topology, buildRngA));
    Individual individualB(aisolver::training::evolutionary::buildNetwork(topology, buildRngB));

    EvolutionaryConfig config;
    config.mutationRate = 1.0f;
    Rng mutationRngA(8501);
    Rng mutationRngB(8501);
    mutate(individualA, config, mutationRngA);
    mutate(individualB, config, mutationRngB);

    auto paramsA = individualA.network().parameters();
    auto paramsB = individualB.network().parameters();
    for (std::size_t index = 0; index < paramsA.size(); ++index) {
        for (std::size_t element = 0; element < paramsA[index]->value.size(); ++element) {
            EXPECT_FLOAT_EQ(paramsA[index]->value.data()[element],
                            paramsB[index]->value.data()[element]);
        }
    }
}

/**
 * @brief `bestIndividual` retourne bien l'individu du plus haut fitness.
 * \castest{<b>bestIndividual : plus haut fitness connu.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Population de fitness `{3, 9, 1}`.<br/>2. Appeler `bestIndividual`.<br/>
 * \tattendu Fitness retourné `== 9`.}
 */
TEST(GeneticOperatorsTest, BestIndividualPlusHautFitness) {
    Population population = makePopulationWithFitness({3.0f, 9.0f, 1.0f});
    EXPECT_FLOAT_EQ(bestIndividual(population).fitness, 9.0f);
}
