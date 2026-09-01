// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Math/Autodiff/Ops.h"

#include <algorithm>
#include <cmath>

#include "AiSolver/Math/Matmul.h"
#include "AiSolver/Math/TensorOps.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::autodiff {

namespace {

/// Plancher applique a l'entree de `logOp`, a l'aller comme au retour.
///
/// L'appelant reel est la perte de policy gradient (`Training/PolicyGradientLoss.cpp`), dont
/// l'entree est une probabilite issue d'un `softmax`. Une politique qui se specialise finit par
/// produire des probabilites qui **s'annulent en flottant** (deux logits distants de plus de ~88
/// suffisent) : `log(0)` vaut alors `-inf` et sa derivee `1/0` vaut `+inf`, ce qui detruit tous
/// les poids au premier pas d'optimisation. Le plancher borne `log` a `-18.4` et la derivee a
/// `1e8`, ou le gradient amont vaut deja zero -- la valeur reste finie, la formule reste exacte
/// partout ailleurs.
constexpr float LOG_INPUT_FLOOR = 1e-8f;

}  // namespace

NodePtr add(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) {
            return aisolver::add(valueA, valueB);
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) { return outputGrad; },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) { return outputGrad; });
}

NodePtr multiply(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) {
            return aisolver::multiply(valueA, valueB);
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>& valueB) { return aisolver::multiply(outputGrad, valueB); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& valueA,
           const Tensor<float>&) { return aisolver::multiply(outputGrad, valueA); });
}

NodePtr matmul(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) {
            return aisolver::matmul(valueA, valueB);
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>& valueB) {
            return aisolver::matmul(outputGrad, aisolver::transpose(valueB));
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& valueA,
           const Tensor<float>&) {
            return aisolver::matmul(aisolver::transpose(valueA), outputGrad);
        });
}

NodePtr relu(const NodePtr& a) {
    return unaryOp(
        a,
        [](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value,
                                                      [](float x) { return x > 0.0f ? x : 0.0f; });
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& inputValue) {
            return aisolver::detail::elementwise(
                outputGrad, inputValue, [](float g, float x) { return x > 0.0f ? g : 0.0f; });
        });
}

NodePtr tanhOp(const NodePtr& a) {
    return unaryOp(
        a,
        [](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value, [](float x) { return std::tanh(x); });
        },
        [](const Tensor<float>& outputValue, const Tensor<float>& outputGrad,
           const Tensor<float>&) {
            return aisolver::detail::elementwise(
                outputGrad, outputValue, [](float g, float t) { return g * (1.0f - t * t); });
        });
}

// --- Operations complementaires (LOT-ANNEXE-03, TACHE-05) --------------------------------------

NodePtr subtract(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) {
            return aisolver::subtract(valueA, valueB);
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) { return outputGrad; },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) {
            return aisolver::detail::elementwiseUnary(outputGrad, [](float g) { return -g; });
        });
}

NodePtr divide(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) {
            for (std::size_t i = 0; i < valueB.size(); ++i) {
                PROJECTGAMING_ASSERT(valueB.data()[i] != 0.0f, "divide() : diviseur nul");
            }
            return aisolver::divide(valueA, valueB);
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>& valueB) { return aisolver::divide(outputGrad, valueB); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& valueA,
           const Tensor<float>& valueB) {
            // d(a/b)/db = -a / b^2.
            const Tensor<float> valueBSquared = aisolver::multiply(valueB, valueB);
            const Tensor<float> negativeRatio = aisolver::detail::elementwiseUnary(
                aisolver::divide(valueA, valueBSquared), [](float x) { return -x; });
            return aisolver::multiply(outputGrad, negativeRatio);
        });
}

NodePtr addScalar(const NodePtr& a, float scalar) {
    return unaryOp(
        a, [scalar](const Tensor<float>& value) { return aisolver::addScalar(value, scalar); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&) {
            return outputGrad;
        });
}

NodePtr multiplyScalar(const NodePtr& a, float scalar) {
    return unaryOp(
        a, [scalar](const Tensor<float>& value) { return aisolver::multiplyScalar(value, scalar); },
        [scalar](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&) {
            return aisolver::multiplyScalar(outputGrad, scalar);
        });
}

NodePtr logOp(const NodePtr& a) {
    return unaryOp(
        a,
        [](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value, [](float x) {
                PROJECTGAMING_ASSERT(x >= 0.0f,
                                     "logOp() : tous les elements d'entree doivent etre >= 0");
                return std::log((std::max)(x, LOG_INPUT_FLOOR));
            });
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& inputValue) {
            // Meme plancher qu'a l'aller : sans lui, `outputGrad / 0` rend un gradient infini qui
            // contamine tous les poids au premier `step()`.
            const Tensor<float> flooredInput = aisolver::detail::elementwiseUnary(
                inputValue, [](float x) { return (std::max)(x, LOG_INPUT_FLOOR); });
            return aisolver::divide(outputGrad, flooredInput);
        });
}

NodePtr sumAll(const NodePtr& a) {
    return unaryOp(
        a,
        [](const Tensor<float>& value) {
            Tensor<float> result({1});
            float total = 0.0f;
            for (std::size_t index = 0; index < value.size(); ++index) {
                total += value.data()[index];
            }
            result.data()[0] = total;
            return result;
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& inputValue) {
            // Derivee d'une somme par rapport a chacun de ses termes : 1. Le gradient scalaire de
            // sortie est donc diffuse tel quel sur toute la forme d'entree.
            Tensor<float> result(inputValue.shape());
            const float seed = outputGrad.data()[0];
            for (std::size_t index = 0; index < result.size(); ++index) {
                result.data()[index] = seed;
            }
            return result;
        });
}

NodePtr expOp(const NodePtr& a) {
    return unaryOp(
        a,
        [](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value, [](float x) { return std::exp(x); });
        },
        [](const Tensor<float>& outputValue, const Tensor<float>& outputGrad,
           const Tensor<float>&) { return aisolver::multiply(outputGrad, outputValue); });
}

NodePtr selectIndex(const NodePtr& a, std::size_t index) {
    PROJECTGAMING_ASSERT(index < a->value.size(), "selectIndex() : index hors bornes");
    return unaryOp(
        a,
        [index](const Tensor<float>& value) {
            Tensor<float> result({1});
            result.data()[0] = value.data()[index];
            return result;
        },
        [index](const Tensor<float>&, const Tensor<float>& outputGrad,
                const Tensor<float>& inputValue) {
            // Nul partout sauf a `index`, ou il vaut le gradient de sortie (scalaire, un seul
            // element) : Tensor<T> initialise ses elements a T{} (donc 0.0f), il suffit d'ecrire
            // le seul element non nul.
            Tensor<float> grad(inputValue.shape());
            grad.data()[index] = outputGrad.data()[0];
            return grad;
        });
}

NodePtr minimum(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) {
            return aisolver::detail::elementwise(valueA, valueB,
                                                 [](float x, float y) { return x < y ? x : y; });
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& valueA,
           const Tensor<float>& valueB) {
            // Egalite stricte : convention arbitraire documentee, va a `a`.
            return aisolver::detail::elementwise(
                outputGrad,
                aisolver::detail::elementwise(
                    valueA, valueB, [](float x, float y) { return x <= y ? 1.0f : 0.0f; }),
                [](float g, float mask) { return g * mask; });
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& valueA,
           const Tensor<float>& valueB) {
            return aisolver::detail::elementwise(
                outputGrad,
                aisolver::detail::elementwise(valueA, valueB,
                                              [](float x, float y) { return x < y ? 0.0f : 1.0f; }),
                [](float g, float mask) { return g * mask; });
        });
}

NodePtr clamp(const NodePtr& a, float low, float high) {
    PROJECTGAMING_ASSERT(low <= high, "clamp() : low doit etre <= high");
    return unaryOp(
        a,
        [low, high](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value, [low, high](float x) {
                if (x < low) {
                    return low;
                }
                if (x > high) {
                    return high;
                }
                return x;
            });
        },
        [low, high](const Tensor<float>&, const Tensor<float>& outputGrad,
                    const Tensor<float>& inputValue) {
            return aisolver::detail::elementwise(
                outputGrad, inputValue,
                [low, high](float g, float x) { return (x > low && x < high) ? g : 0.0f; });
        });
}

}  // namespace aisolver::autodiff
