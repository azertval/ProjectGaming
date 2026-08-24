#include "AiSolver/Math/Autodiff/Ops.h"

#include <cmath>

#include "AiSolver/Math/Matmul.h"
#include "AiSolver/Math/TensorOps.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::autodiff {

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
                PROJECTGAMING_ASSERT(x > 0.0f,
                                     "logOp() : tous les elements d'entree doivent etre > 0");
                return std::log(x);
            });
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& inputValue) {
            return aisolver::divide(outputGrad, inputValue);
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
