#pragma once

#include <cstddef>
#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Autodiff/Ops.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"

/**
 * @file ToyProblems.h
 * @brief Problèmes jouets à solution connue, communs à `test_sgd.cpp`/`test_adam.cpp`/
 * `test_convergence.cpp` (LOT-ANNEXE-04) — header-only, pas d'entrée dans
 * `Source/AiSolver/CMakeLists.txt`. Volontairement triviaux (convexes, solution connue en forme
 * close) : vérifient l'exactitude mécanique de la règle de mise à jour d'un optimiseur, pas la
 * qualité d'un apprentissage par renforcement réel.
 */

namespace aisolver {

/// Quadratique convexe à minimum connu : `f(x) = (x - target)^2`, gradient `2(x - target)`.
struct QuadraticToyProblem {
    static constexpr float kTarget = 5.0f;

    /// @return Nœud de perte `(x->value - kTarget)^2`, relié au graphe d'autodiff.
    [[nodiscard]] static autodiff::NodePtr loss(const autodiff::NodePtr& x) {
        const autodiff::NodePtr diff = autodiff::addScalar(x, -kTarget);
        return autodiff::multiply(diff, diff);
    }
};

/// Régression polynomiale à coefficients connus (`y = kCoeffA*x^2 + kCoeffB*x + kCoeffC`), ajustée
/// par un vecteur de poids de forme `[1, 3]` (colonnes `[x^2, x, 1]`) minimisant l'erreur
/// quadratique moyenne sur un jeu de points synthétiques.
struct PolynomialToyProblem {
    static constexpr float kCoeffA = 2.0f;
    static constexpr float kCoeffB = -3.0f;
    static constexpr float kCoeffC = 1.0f;
    static constexpr std::size_t kSampleCount = 20;

    struct Sample {
        float x;
        float y;
    };

    /// @return `kSampleCount` points `(x, y)` générés à partir de `rng`, `x` uniforme dans
    /// `[-3, 3]`, `y` calculé exactement (aucun bruit ajouté).
    [[nodiscard]] static std::vector<Sample> generateSamples(Rng& rng) {
        std::vector<Sample> samples;
        samples.reserve(kSampleCount);
        for (std::size_t i = 0; i < kSampleCount; ++i) {
            const float x = rng.nextFloat(-3.0f, 3.0f);
            const float y = kCoeffA * x * x + kCoeffB * x + kCoeffC;
            samples.push_back(Sample{x, y});
        }
        return samples;
    }

    /// @return Nœud de perte MSE de `weights` (forme `[1, 3]`) sur `samples`, relié au graphe
    /// d'autodiff : reconstruit une passe avant complète (un `matmul` par échantillon), comme
    /// l'exige le moteur d'autodiff *eager* de `LOT-ANNEXE-02`.
    [[nodiscard]] static autodiff::NodePtr loss(const autodiff::NodePtr& weights,
                                                const std::vector<Sample>& samples) {
        autodiff::NodePtr totalSquaredError;
        for (const Sample& sample : samples) {
            Tensor<float> featureData({3, 1});
            featureData.at({0, 0}) = sample.x * sample.x;
            featureData.at({1, 0}) = sample.x;
            featureData.at({2, 0}) = 1.0f;
            const autodiff::NodePtr features = autodiff::variable(featureData);

            const autodiff::NodePtr prediction = autodiff::matmul(weights, features);
            const autodiff::NodePtr error = autodiff::addScalar(prediction, -sample.y);
            const autodiff::NodePtr squaredError = autodiff::multiply(error, error);

            totalSquaredError =
                totalSquaredError ? autodiff::add(totalSquaredError, squaredError) : squaredError;
        }
        return autodiff::multiplyScalar(totalSquaredError,
                                        1.0f / static_cast<float>(samples.size()));
    }
};

}  // namespace aisolver
