// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_reinforce_loss.cpp
 * @brief Tests unitaires de `computeReinforceLoss` (LOT-ANNEXE-12, TACHE-03) : gradient checking
 * (le test le plus important du lot, TACHE-05) et propriétés de signe/dégénérescence.
 */

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Optim/Sgd.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "AiSolver/Training/PolicyGradient/ReinforceLoss.h"
#include "AiSolver/Training/PolicyGradient/Trajectory.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::computeReinforceLoss;
using aisolver::training::Trajectory;
using aisolver::training::TrajectoryStep;
using aisolver::training::evolutionary::buildNetwork;
using aisolver::training::evolutionary::policyTopology;

namespace {

constexpr std::size_t kInputSize = 3;
constexpr std::size_t kHiddenSize = 4;

std::unique_ptr<aisolver::nn::Network> tinyPolicy(std::uint64_t seed) {
    Rng rng(seed);
    return buildNetwork(policyTopology(kInputSize, kHiddenSize), rng);
}

Tensor<float> observationOf(float a, float b, float c) {
    Tensor<float> observation({kInputSize, 1});
    observation.at({0, 0}) = a;
    observation.at({1, 0}) = b;
    observation.at({2, 0}) = c;
    return observation;
}

TrajectoryStep stepAt(std::size_t actionIndex, const Tensor<float>& observation) {
    TrajectoryStep step;
    step.observation = observation;
    step.actionIndex = actionIndex;
    step.logProbability = 0.0f;  // Non consomme par computeReinforceLoss (rejoue le forward).
    step.reward = 0.0f;          // Non consomme directement : les retours sont passes a part.
    return step;
}

/// Gradient par differences finies centrees sur chaque poids/biais du reseau de politique, compare
/// au gradient analytique de `backward()` -- meme methode que GradientCheck.h (LOT-ANNEXE-02,
/// TACHE-04), adaptee ici : les parametres vivent dans le reseau (Dense), pas dans des Tensor
/// passes en argument d'une fonction de construction de graphe.
float maxAbsoluteGradientError(aisolver::nn::Network& policy, const Trajectory& trajectory,
                               const std::vector<float>& returns, float epsilon = 1e-3f) {
    const std::vector<aisolver::autodiff::NodePtr> parameters = policy.parameters();

    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    const aisolver::autodiff::NodePtr loss = computeReinforceLoss(policy, trajectory, returns);
    aisolver::autodiff::backward(loss);

    std::vector<Tensor<float>> analyticGrads;
    analyticGrads.reserve(parameters.size());
    for (const auto& parameter : parameters) {
        analyticGrads.push_back(parameter->grad.clone());
    }

    float maxError = 0.0f;
    for (std::size_t paramIndex = 0; paramIndex < parameters.size(); ++paramIndex) {
        auto& parameter = parameters[paramIndex];
        for (std::size_t elementIndex = 0; elementIndex < parameter->value.size(); ++elementIndex) {
            const float original = parameter->value.data()[elementIndex];

            parameter->value.data()[elementIndex] = original + epsilon;
            const float lossPlus =
                computeReinforceLoss(policy, trajectory, returns)->value.data()[0];

            parameter->value.data()[elementIndex] = original - epsilon;
            const float lossMinus =
                computeReinforceLoss(policy, trajectory, returns)->value.data()[0];

            parameter->value.data()[elementIndex] = original;

            const float numericGrad = (lossPlus - lossMinus) / (2.0f * epsilon);
            const float analyticGrad = analyticGrads[paramIndex].data()[elementIndex];
            const float error = std::abs(numericGrad - analyticGrad);
            if (error > maxError) {
                maxError = error;
            }
        }
    }
    return maxError;
}

}  // namespace

/**
 * @brief Sur une trajectoire d'un seul pas, le gradient de `backward()` correspond au gradient par
 * différences finies, à la tolérance de LOT-ANNEXE-02.
 * \castest{<b>computeReinforceLoss : gradient checking, trajectoire d'un pas.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Réseau minuscule, trajectoire d'un pas, retour positif.<br/>2. Comparer gradient
 * analytique et numérique.<br/>
 * \tattendu Écart maximal `< 1e-2`.}
 */
TEST(ReinforceLossTest, GradientCheckingTrajectoireDUnPas) {
    auto policy = tinyPolicy(1);
    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(3, observationOf(0.5f, -0.2f, 0.1f)));
    const std::vector<float> returns{2.0f};

    const float maxError = maxAbsoluteGradientError(*policy, trajectory, returns);
    EXPECT_LT(maxError, 1e-2f);
}

/**
 * @brief Sur une trajectoire de 3 pas à retours de signes mélangés, le gradient de `backward()`
 * correspond au gradient par différences finies.
 * \castest{<b>computeReinforceLoss : gradient checking, trajectoire de 3 pas.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Réseau minuscule, 3 pas, retours `[2, -1, 0.5]`.<br/>2. Comparer gradients.<br/>
 * \tattendu Écart maximal `< 1e-2`.}
 */
TEST(ReinforceLossTest, GradientCheckingTrajectoireDeTroisPas) {
    auto policy = tinyPolicy(2);
    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(0, observationOf(0.1f, 0.2f, 0.3f)));
    trajectory.steps.push_back(stepAt(5, observationOf(-0.4f, 0.0f, 0.2f)));
    trajectory.steps.push_back(stepAt(12, observationOf(0.3f, -0.1f, -0.2f)));
    const std::vector<float> returns{2.0f, -1.0f, 0.5f};

    const float maxError = maxAbsoluteGradientError(*policy, trajectory, returns);
    EXPECT_LT(maxError, 1e-2f);
}

/**
 * @brief Sur une trajectoire de 10 pas, le gradient de `backward()` correspond au gradient par
 * différences finies.
 * \castest{<b>computeReinforceLoss : gradient checking, trajectoire de 10 pas.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Réseau minuscule, 10 pas, retours variés.<br/>2. Comparer gradients.<br/>
 * \tattendu Écart maximal `< 1e-2`.}
 */
TEST(ReinforceLossTest, GradientCheckingTrajectoireDeDixPas) {
    auto policy = tinyPolicy(3);
    Trajectory trajectory;
    const std::vector<float> returns{1.0f, -0.5f, 0.0f,  2.0f, -1.5f,
                                     0.3f, 0.7f,  -0.2f, 1.1f, -0.9f};
    for (std::size_t i = 0; i < returns.size(); ++i) {
        const float a = 0.1f * static_cast<float>(i) - 0.5f;
        const float b = std::sin(static_cast<float>(i));
        const float c = -0.2f + 0.05f * static_cast<float>(i);
        trajectory.steps.push_back(stepAt((i * 3) % 24, observationOf(a, b, c)));
    }

    const float maxError = maxAbsoluteGradientError(*policy, trajectory, returns);
    EXPECT_LT(maxError, 1e-2f);
}

/**
 * @brief Un retour nul en entrée produit un gradient rigoureusement nul (le terme `-log π * G_t`
 * s'annule quand `G_t = 0`, indépendamment de la politique).
 * \castest{<b>computeReinforceLoss : retour nul, gradient nul.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Réseau minuscule, trajectoire d'un pas, retour `0`.<br/>2. `backward()`.<br/>
 * \tattendu Gradient nul sur tous les paramètres.}
 */
TEST(ReinforceLossTest, RetourNulGradientNul) {
    auto policy = tinyPolicy(4);
    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(7, observationOf(0.2f, 0.3f, -0.1f)));
    const std::vector<float> returns{0.0f};

    const std::vector<aisolver::autodiff::NodePtr> parameters = policy->parameters();
    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    const aisolver::autodiff::NodePtr loss = computeReinforceLoss(*policy, trajectory, returns);
    aisolver::autodiff::backward(loss);

    for (const auto& parameter : parameters) {
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            EXPECT_FLOAT_EQ(parameter->grad.data()[i], 0.0f);
        }
    }
}

/**
 * @brief Sur une trajectoire non triviale, la perte calculée est non nulle et son gradient par
 * rapport à au moins un poids est non nul (absence de dégénérescence).
 * \castest{<b>computeReinforceLoss : perte et gradient non dégénérés.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Réseau minuscule, trajectoire de 3 pas, retours non nuls.<br/>2. `backward()`.<br/>
 * \tattendu Perte `!= 0`, au moins un gradient de paramètre `!= 0`.}
 */
TEST(ReinforceLossTest, PerteEtGradientNonDegeneres) {
    auto policy = tinyPolicy(5);
    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(1, observationOf(0.1f, 0.2f, 0.3f)));
    trajectory.steps.push_back(stepAt(2, observationOf(0.4f, -0.1f, 0.0f)));
    trajectory.steps.push_back(stepAt(3, observationOf(-0.2f, 0.3f, 0.1f)));
    const std::vector<float> returns{1.5f, -0.5f, 2.0f};

    const std::vector<aisolver::autodiff::NodePtr> parameters = policy->parameters();
    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    const aisolver::autodiff::NodePtr loss = computeReinforceLoss(*policy, trajectory, returns);
    EXPECT_NE(loss->value.data()[0], 0.0f);

    aisolver::autodiff::backward(loss);
    bool anyNonZeroGradient = false;
    for (const auto& parameter : parameters) {
        for (std::size_t i = 0; i < parameter->grad.size(); ++i) {
            if (parameter->grad.data()[i] != 0.0f) {
                anyNonZeroGradient = true;
            }
        }
    }
    EXPECT_TRUE(anyNonZeroGradient);
}

/**
 * @brief Deux trajectoires synthétiques de longueurs différentes mais de retours par pas
 * comparables produisent des pertes d'ordre de grandeur comparable (la moyenne, pas la somme, rend
 * la perte indépendante de la longueur de l'épisode).
 * \castest{<b>computeReinforceLoss : indépendance à la longueur d'épisode.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Même politique, deux trajectoires de 2 et 8 pas, retour constant par pas.<br/>2.
 * Comparer les pertes.<br/>
 * \tattendu Rapport des deux pertes proche de `1` (à un facteur `3` près, marge large).}
 */
TEST(ReinforceLossTest, IndependanceALaLongueurDEpisode) {
    auto policy = tinyPolicy(6);

    Trajectory shortTrajectory;
    for (int i = 0; i < 2; ++i) {
        shortTrajectory.steps.push_back(stepAt(4, observationOf(0.1f, 0.1f, 0.1f)));
    }
    const std::vector<float> shortReturns(2, 1.0f);

    Trajectory longTrajectory;
    for (int i = 0; i < 8; ++i) {
        longTrajectory.steps.push_back(stepAt(4, observationOf(0.1f, 0.1f, 0.1f)));
    }
    const std::vector<float> longReturns(8, 1.0f);

    const float shortLoss =
        computeReinforceLoss(*policy, shortTrajectory, shortReturns)->value.data()[0];
    const float longLoss =
        computeReinforceLoss(*policy, longTrajectory, longReturns)->value.data()[0];

    // Chaque pas des deux trajectoires est rigoureusement identique (meme observation, meme
    // action, meme retour) : la moyenne (et non la somme) rend donc les deux pertes egales a la
    // precision flottante pres, quelle que soit la difference de longueur (2 pas contre 8).
    EXPECT_NEAR(shortLoss, longLoss, 1e-4f);
}

/**
 * @brief Une action à log-probabilité basse associée à un retour positif élevé produit, après un
 * pas d'optimiseur, une augmentation mesurable de la probabilité de cette action sur la même
 * observation (vérifie le sens de la mise à jour, pas seulement sa magnitude).
 * \castest{<b>computeReinforceLoss : signe du gradient (augmente la probabilité
 * renforcée).</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Réseau minuscule, observation fixe, retour positif élevé.<br/>2. `backward()` + un
 * pas de SGD.<br/>3. Repasser en avant sur la même observation.<br/>
 * \tattendu Probabilité de l'action renforcée strictement supérieure après la mise à jour.}
 */
TEST(ReinforceLossTest, SigneDuGradientAugmenteLaProbabiliteRenforcee) {
    auto policy = tinyPolicy(7);
    const Tensor<float> observation = observationOf(0.2f, -0.3f, 0.1f);
    constexpr std::size_t kActionIndex = 9;

    const auto probabilityOf = [&](std::size_t actionIndex) {
        const aisolver::autodiff::NodePtr inputNode = aisolver::autodiff::variable(observation);
        const aisolver::autodiff::NodePtr outputNode = policy->forward(inputNode);
        return outputNode->value.data()[actionIndex];
    };

    const float probabilityBefore = probabilityOf(kActionIndex);

    Trajectory trajectory;
    trajectory.steps.push_back(stepAt(kActionIndex, observation));
    const std::vector<float> returns{5.0f};

    const std::vector<aisolver::autodiff::NodePtr> parameters = policy->parameters();
    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
    const aisolver::autodiff::NodePtr loss = computeReinforceLoss(*policy, trajectory, returns);
    aisolver::autodiff::backward(loss);

    aisolver::optim::Sgd optimizer(0.05f);
    optimizer.step(parameters);

    const float probabilityAfter = probabilityOf(kActionIndex);
    EXPECT_GT(probabilityAfter, probabilityBefore);
}
