#pragma once

#include <cmath>
#include <cstddef>
#include <functional>
#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Math/TensorOps.h"

/**
 * @file GradientCheck.h
 * @brief Vérification de gradient par différences finies centrées (LOT-ANNEXE-02, TACHE-04),
 * utilitaire de test réutilisable — header-only, pas d'entrée dans
 * `Source/AiSolver/CMakeLists.txt`.
 */

namespace aisolver {

/// Résultat d'une vérification de gradient : `passed` est vrai si l'écart maximal observé entre
/// gradient analytique et gradient numérique reste sous la tolérance demandée.
struct GradientCheckResult {
    bool passed;
    float maxAbsoluteError;
};

namespace detail {

/**
 * @brief Réduit un `Node` de forme quelconque à un `Node` scalaire (un seul élément), par somme de
 * ses éléments.
 *
 * `backward()` (LOT-ANNEXE-02, TACHE-03) n'accepte qu'une racine scalaire ; les opérations testées
 * ici (`add`, `multiply`, `matmul`, `relu`, `tanhOp`) produisent en général une sortie non
 * scalaire. Sommer la sortie avant `backward()` équivaut à vérifier chaque élément de sortie avec
 * un gradient de sortie unitaire (`d(sum)/d(sortie_i) = 1`), ce qui recouvre exactement le produit
 * jacobien-vecteur que produirait une différence finie appliquée directement à chaque élément de
 * sortie. Construit via `unaryOp` (TACHE-01), comme toute opération différentiable de ce lot — pas
 * de `sum` dédié dans `autodiff` (`Ops.h`), hors périmètre de l'épic, ce réducteur reste local à
 * cet utilitaire de test.
 */
[[nodiscard]] inline autodiff::NodePtr reduceToScalar(const autodiff::NodePtr& node) {
    return autodiff::unaryOp(
        node,
        [](const Tensor<float>& value) {
            Tensor<float> result({1});
            result.at({0}) = sum(value);
            return result;
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& inputValue) {
            Tensor<float> grad(inputValue.shape());
            const float scalarGrad = outputGrad.at({0});
            float* data = grad.data();
            for (std::size_t i = 0; i < grad.size(); ++i) {
                data[i] = scalarGrad;
            }
            return grad;
        });
}

}  // namespace detail

/**
 * @brief Compare, élément par élément de chaque entrée, le gradient analytique (`backward()`) au
 * gradient numérique (différences finies centrées) d'un graphe construit par `buildGraph`.
 *
 * Pour chaque tenseur d'entrée et chacun de ses éléments : reconstruit le graphe deux fois (une
 * fois avec l'élément perturbé de `+epsilon`, une fois de `-epsilon`, les autres entrées
 * inchangées), en dérive le gradient numérique `(sortiePerturbeePlus - sortiePerturbeeMoins) /
 * (2 * epsilon)`. Le gradient analytique est lu une seule fois, sur un graphe construit avec les
 * entrées non perturbées et un unique appel à `backward()`. `maxAbsoluteError` est l'écart absolu
 * maximal observé sur l'ensemble des éléments de toutes les entrées ; `passed` est vrai si cet
 * écart reste sous `tolerance`.
 *
 * @param buildGraph Construit le nœud de sortie à partir des nœuds d'entrée (dans le même ordre que
 *                    `inputs`) ; reconstruit à chaque perturbation, jamais muté en place.
 * @param inputs     Tenseurs d'entrée, un par paramètre de `buildGraph`.
 * @param epsilon    Amplitude de la perturbation centrée (valeur par défaut choisie empiriquement,
 *                    cf. points d'attention de TACHE-04).
 * @param tolerance  Écart absolu maximal toléré entre gradient analytique et numérique.
 */
[[nodiscard]] inline GradientCheckResult checkGradient(
    const std::function<autodiff::NodePtr(const std::vector<autodiff::NodePtr>&)>& buildGraph,
    const std::vector<Tensor<float>>& inputs, float epsilon = 1e-3f, float tolerance = 1e-2f) {
    std::vector<autodiff::NodePtr> analyticNodes;
    analyticNodes.reserve(inputs.size());
    for (const Tensor<float>& input : inputs) {
        analyticNodes.push_back(autodiff::variable(input.clone()));
    }
    autodiff::backward(detail::reduceToScalar(buildGraph(analyticNodes)));

    float maxAbsoluteError = 0.0f;

    for (std::size_t inputIndex = 0; inputIndex < inputs.size(); ++inputIndex) {
        const Tensor<float>& original = inputs[inputIndex];
        for (std::size_t elementIndex = 0; elementIndex < original.size(); ++elementIndex) {
            Tensor<float> perturbedPlus = original.clone();
            perturbedPlus.data()[elementIndex] += epsilon;
            Tensor<float> perturbedMinus = original.clone();
            perturbedMinus.data()[elementIndex] -= epsilon;

            std::vector<autodiff::NodePtr> nodesPlus;
            std::vector<autodiff::NodePtr> nodesMinus;
            nodesPlus.reserve(inputs.size());
            nodesMinus.reserve(inputs.size());
            for (std::size_t i = 0; i < inputs.size(); ++i) {
                nodesPlus.push_back(
                    autodiff::variable(i == inputIndex ? perturbedPlus : inputs[i].clone()));
                nodesMinus.push_back(
                    autodiff::variable(i == inputIndex ? perturbedMinus : inputs[i].clone()));
            }

            const float outputPlus = detail::reduceToScalar(buildGraph(nodesPlus))->value.at({0});
            const float outputMinus = detail::reduceToScalar(buildGraph(nodesMinus))->value.at({0});
            const float numericGrad = (outputPlus - outputMinus) / (2.0f * epsilon);

            const float analyticGrad = analyticNodes[inputIndex]->grad.data()[elementIndex];

            const float absoluteError = std::abs(numericGrad - analyticGrad);
            if (absoluteError > maxAbsoluteError) {
                maxAbsoluteError = absoluteError;
            }
        }
    }

    return GradientCheckResult{maxAbsoluteError < tolerance, maxAbsoluteError};
}

}  // namespace aisolver
