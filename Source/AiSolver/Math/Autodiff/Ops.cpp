#include "AiSolver/Math/Autodiff/Ops.h"

#include <cmath>

#include "AiSolver/Math/Matmul.h"
#include "AiSolver/Math/TensorOps.h"

namespace aisolver::autodiff {

NodePtr add(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b, [](const Tensor<float>& valueA, const Tensor<float>& valueB) { return aisolver::add(valueA, valueB); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) { return outputGrad; },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>&) { return outputGrad; });
}

NodePtr multiply(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b,
        [](const Tensor<float>& valueA, const Tensor<float>& valueB) { return aisolver::multiply(valueA, valueB); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>& valueB) { return aisolver::multiply(outputGrad, valueB); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& valueA,
           const Tensor<float>&) { return aisolver::multiply(outputGrad, valueA); });
}

NodePtr matmul(const NodePtr& a, const NodePtr& b) {
    return binaryOp(
        a, b, [](const Tensor<float>& valueA, const Tensor<float>& valueB) { return aisolver::matmul(valueA, valueB); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>&,
           const Tensor<float>& valueB) { return aisolver::matmul(outputGrad, aisolver::transpose(valueB)); },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& valueA,
           const Tensor<float>&) { return aisolver::matmul(aisolver::transpose(valueA), outputGrad); });
}

NodePtr relu(const NodePtr& a) {
    return unaryOp(
        a,
        [](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value, [](float x) { return x > 0.0f ? x : 0.0f; });
        },
        [](const Tensor<float>&, const Tensor<float>& outputGrad, const Tensor<float>& inputValue) {
            return aisolver::detail::elementwise(outputGrad, inputValue,
                                                  [](float g, float x) { return x > 0.0f ? g : 0.0f; });
        });
}

NodePtr tanhOp(const NodePtr& a) {
    return unaryOp(
        a,
        [](const Tensor<float>& value) {
            return aisolver::detail::elementwiseUnary(value, [](float x) { return std::tanh(x); });
        },
        [](const Tensor<float>& outputValue, const Tensor<float>& outputGrad, const Tensor<float>&) {
            return aisolver::detail::elementwise(
                outputGrad, outputValue, [](float g, float t) { return g * (1.0f - t * t); });
        });
}

}  // namespace aisolver::autodiff
