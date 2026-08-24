// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fitness_evaluator.cpp
 * @brief Tests unitaires de `evaluateFitness`/`Population::evaluateAll` (LOT-ANNEXE-10, TACHE-02).
 */

#include <filesystem>
#include <memory>

#include <gtest/gtest.h>

#include "AiSolver/Env/ActionSpace.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Nn/Activations.h"
#include "AiSolver/Training/Evolutionary/FitnessEvaluator.h"
#include "AiSolver/Training/Evolutionary/Population.h"

using aisolver::Action;
using aisolver::actionCount;
using aisolver::Direction;
using aisolver::HeadlessLevelEnvironment;
using aisolver::indexOf;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::evaluateFitness;
using aisolver::training::evolutionary::Individual;
using aisolver::training::evolutionary::LayerTopology;
using aisolver::training::evolutionary::NetworkTopology;
using aisolver::training::evolutionary::Population;

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

// Reseau fixture (pas d'entrainement) qui produit toujours la meme action, quelle que soit
// l'observation : poids nuls (l'entree n'influence donc jamais la sortie), biais tres favorable a
// `actionIndex` -- softmax(biais) reste dominee par ce biais quel que soit `x` puisque `W = 0`.
Individual constantActionIndividual(std::size_t inputSize, std::size_t actionIndex) {
    NetworkTopology topology{
        LayerTopology{inputSize, actionCount(), aisolver::nn::WeightInitScheme::Xavier,
                      aisolver::nn::softmax},
    };
    Rng rng(1);
    auto network = buildNetwork(topology, rng);
    auto params = network->parameters();
    float* weights = params[0]->value.data();
    for (std::size_t i = 0; i < params[0]->value.size(); ++i) {
        weights[i] = 0.0f;
    }
    float* bias = params[1]->value.data();
    for (std::size_t i = 0; i < params[1]->value.size(); ++i) {
        bias[i] = (i == actionIndex) ? 10.0f : -10.0f;
    }
    return Individual(std::move(network));
}

std::size_t moveRightActionIndex() {
    return indexOf(Action{Direction::Right, false, false, false});
}

std::size_t doNothingActionIndex() {
    return indexOf(Action{Direction::None, false, false, false});
}

}  // namespace

/**
 * @brief Deux évaluations successives du même individu (poids inchangés) sur le même niveau
 * produisent un fitness strictement identique.
 * \castest{<b>evaluateFitness : déterminisme.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Individu à action constante « avancer à droite ».<br/>2. Évaluer deux fois sur
 * `demo-deplacement.json`.<br/>
 * \tattendu Fitness strictement identique entre les deux évaluations.}
 */
TEST(FitnessEvaluatorTest, DeterminismeDUneEvaluation) {
    const ObservationEncoder encoder;
    Individual individual = constantActionIndividual(encoder.inputSize(), moveRightActionIndex());
    HeadlessLevelEnvironment env;

    const auto first = evaluateFitness(individual, env, levelPath("demo-deplacement.json"));
    const auto second = evaluateFitness(individual, env, levelPath("demo-deplacement.json"));

    EXPECT_FLOAT_EQ(first.fitness, second.fitness);
    EXPECT_EQ(first.stepCount, second.stepCount);
    EXPECT_EQ(first.status, second.status);
}

/**
 * @brief Un individu qui progresse effectivement vers la sortie obtient un fitness supérieur à un
 * individu qui reste immobile.
 * \castest{<b>evaluateFitness : ordre de fitness cohérent.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Deux individus à action constante (« avancer à droite » vs « ne rien faire »).<br/>2.
 * Évaluer chacun sur `demo-deplacement.json`.<br/>
 * \tattendu Fitness de l'individu qui avance strictement supérieur à celui qui reste immobile.}
 */
TEST(FitnessEvaluatorTest, OrdreDeFitnessCoherent) {
    const ObservationEncoder encoder;
    Individual moving = constantActionIndividual(encoder.inputSize(), moveRightActionIndex());
    Individual still = constantActionIndividual(encoder.inputSize(), doNothingActionIndex());

    HeadlessLevelEnvironment env;
    const auto movingEvaluation = evaluateFitness(moving, env, levelPath("demo-deplacement.json"));
    const auto stillEvaluation = evaluateFitness(still, env, levelPath("demo-deplacement.json"));

    EXPECT_GT(movingEvaluation.fitness, stillEvaluation.fitness);
}

/**
 * @brief L'évaluation d'un individu produisant en boucle « ne rien faire » se termine bien avant le
 * plafond de pas dur.
 * \castest{<b>evaluateFitness : terminaison garantie (blocage détecté).</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Individu à action constante « ne rien faire ».<br/>2. Évaluer sur
 * `demo-deplacement.json`.<br/>
 * \tattendu Nombre de pas strictement inférieur au budget dur par défaut (`3000`).}
 */
TEST(FitnessEvaluatorTest, TerminaisonGarantieSurBlocage) {
    const ObservationEncoder encoder;
    Individual still = constantActionIndividual(encoder.inputSize(), doNothingActionIndex());
    HeadlessLevelEnvironment env;

    const auto evaluation = evaluateFitness(still, env, levelPath("demo-deplacement.json"));

    EXPECT_LT(evaluation.stepCount, 3000);
}

/**
 * @brief `Population::evaluateAll` sur deux individus différents utilisant la même instance
 * d'environnement produit, pour chacun, le même fitness que si chacun était évalué isolément.
 * \castest{<b>evaluateAll : aucune fuite d'état entre individus.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Évaluer deux individus isolément (deux environnements distincts).<br/>2. Évaluer les
 * deux mêmes individus (poids identiques, individus neufs) via `Population::evaluateAll` sur une
 * seule instance d'environnement.<br/>
 * \tattendu Les fitness obtenus par `evaluateAll` sont égaux à ceux obtenus isolément.}
 */
TEST(FitnessEvaluatorTest, EvaluateAllSansFuiteDEtat) {
    const ObservationEncoder encoder;
    const std::size_t rightIndex = moveRightActionIndex();
    const std::size_t stillIndex = doNothingActionIndex();

    HeadlessLevelEnvironment isolatedEnv1;
    Individual isolatedMoving = constantActionIndividual(encoder.inputSize(), rightIndex);
    const auto isolatedMovingEvaluation =
        evaluateFitness(isolatedMoving, isolatedEnv1, levelPath("demo-deplacement.json"));

    HeadlessLevelEnvironment isolatedEnv2;
    Individual isolatedStill = constantActionIndividual(encoder.inputSize(), stillIndex);
    const auto isolatedStillEvaluation =
        evaluateFitness(isolatedStill, isolatedEnv2, levelPath("demo-deplacement.json"));

    NetworkTopology topology{
        LayerTopology{encoder.inputSize(), actionCount(), aisolver::nn::WeightInitScheme::Xavier,
                      aisolver::nn::softmax},
    };
    std::vector<Individual> individuals;
    individuals.push_back(constantActionIndividual(encoder.inputSize(), rightIndex));
    individuals.push_back(constantActionIndividual(encoder.inputSize(), stillIndex));
    Population population(topology, std::move(individuals));

    HeadlessLevelEnvironment sharedEnv;
    const auto evaluations = population.evaluateAll(sharedEnv, levelPath("demo-deplacement.json"));

    ASSERT_EQ(evaluations.size(), 2u);
    EXPECT_FLOAT_EQ(evaluations[0].fitness, isolatedMovingEvaluation.fitness);
    EXPECT_FLOAT_EQ(evaluations[1].fitness, isolatedStillEvaluation.fitness);
}
