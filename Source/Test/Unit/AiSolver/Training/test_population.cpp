// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_population.cpp
 * @brief Tests unitaires de `aisolver::training::evolutionary::Population`/`Individual`
 * (LOT-ANNEXE-10, TACHE-01).
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Nn/WeightInit.h"
#include "AiSolver/Training/Evolutionary/Individual.h"
#include "AiSolver/Training/Evolutionary/Population.h"

using aisolver::Rng;
using aisolver::training::evolutionary::Individual;
using aisolver::training::evolutionary::LayerTopology;
using aisolver::training::evolutionary::NetworkTopology;
using aisolver::training::evolutionary::Population;
using aisolver::training::evolutionary::UNEVALUATED_FITNESS;

namespace {

NetworkTopology smallTopology() {
    return NetworkTopology{
        LayerTopology{4, 5, aisolver::nn::WeightInitScheme::Xavier, aisolver::autodiff::tanhOp},
        LayerTopology{5, 3, aisolver::nn::WeightInitScheme::Xavier, nullptr},
    };
}

}  // namespace

/**
 * @brief Une `Population` construite avec `N` individus en contient exactement `N`, chacun avec la
 * topologie attendue.
 * \castest{<b>Population : taille et topologie correctes.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une `Population` de 7 individus.<br/>2. Lire `size()` et
 * `parameters()`/`layerCount()` de chaque réseau.<br/>
 * \tattendu 7 individus, chacun `layerCount() == 2` et `parameters().size() == 4`.}
 */
TEST(PopulationTest, TailleEtTopologieCorrectes) {
    Rng rng(1001);
    Population population(smallTopology(), 7, rng);

    EXPECT_EQ(population.size(), 7u);
    for (std::size_t index = 0; index < population.size(); ++index) {
        EXPECT_EQ(population.individual(index).network().layerCount(), 2u);
        EXPECT_EQ(population.individual(index).network().parameters().size(), 4u);
    }
}

/**
 * @brief Deux individus de la même population n'ont jamais exactement les mêmes poids, et modifier
 * les poids de l'un ne modifie pas l'autre (copie profonde).
 * \castest{<b>Population : poids indépendants entre individus.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une `Population` de 2 individus.<br/>2. Comparer leurs premiers poids,
 * puis modifier ceux du premier individu.<br/>
 * \tattendu Poids initiaux différents ; la modification du premier n'affecte pas le second.}
 */
TEST(PopulationTest, PoidsIndependantsEntreIndividus) {
    Rng rng(1002);
    Population population(smallTopology(), 2, rng);

    auto paramsA = population.individual(0).network().parameters();
    auto paramsB = population.individual(1).network().parameters();
    ASSERT_EQ(paramsA.size(), paramsB.size());

    bool anyDifferent = false;
    for (std::size_t index = 0; index < paramsA.size(); ++index) {
        for (std::size_t element = 0; element < paramsA[index]->value.size(); ++element) {
            if (paramsA[index]->value.data()[element] != paramsB[index]->value.data()[element]) {
                anyDifferent = true;
            }
        }
    }
    EXPECT_TRUE(anyDifferent);

    const float originalB = paramsB[0]->value.data()[0];
    paramsA[0]->value.data()[0] += 1000.0f;
    EXPECT_EQ(paramsB[0]->value.data()[0], originalB);
}

/**
 * @brief À seed `Rng` fixée, deux populations construites avec les mêmes paramètres ont des poids
 * initiaux strictement identiques, individu par individu.
 * \castest{<b>Population : reproductibilité de l'initialisation.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire deux `Population` identiques avec des `Rng` de même graine.<br/>2.
 * Comparer bit-à-bit tous les poids.<br/>
 * \tattendu Poids strictement identiques, individu par individu, poids par poids.}
 */
TEST(PopulationTest, ReproductibiliteInitialisation) {
    Rng rngA(2001);
    Rng rngB(2001);
    Population populationA(smallTopology(), 5, rngA);
    Population populationB(smallTopology(), 5, rngB);

    for (std::size_t individualIndex = 0; individualIndex < populationA.size(); ++individualIndex) {
        auto paramsA = populationA.individual(individualIndex).network().parameters();
        auto paramsB = populationB.individual(individualIndex).network().parameters();
        ASSERT_EQ(paramsA.size(), paramsB.size());
        for (std::size_t index = 0; index < paramsA.size(); ++index) {
            ASSERT_EQ(paramsA[index]->value.size(), paramsB[index]->value.size());
            for (std::size_t element = 0; element < paramsA[index]->value.size(); ++element) {
                EXPECT_EQ(paramsA[index]->value.data()[element],
                          paramsB[index]->value.data()[element]);
            }
        }
    }
}

/**
 * @brief Chaque individu est créé avec un fitness marqué « non évalué ».
 * \castest{<b>Population : fitness initial non évalué.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une `Population`.<br/>2. Lire `fitness` de chaque individu.<br/>
 * \tattendu Chaque `fitness` vaut `UNEVALUATED_FITNESS`.}
 */
TEST(PopulationTest, FitnessInitialNonEvalue) {
    Rng rng(3001);
    Population population(smallTopology(), 4, rng);
    for (std::size_t index = 0; index < population.size(); ++index) {
        EXPECT_EQ(population.individual(index).fitness, UNEVALUATED_FITNESS);
    }
}
