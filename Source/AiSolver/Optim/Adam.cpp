// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Optim/Adam.h"

#include <cmath>

#include "AiSolver/Math/TensorOps.h"
#include "AiSolver/Optim/OptimizerUtils.h"

namespace aisolver::optim {

namespace {

// Racine carrée élément par élément. `a` est toujours ici un tenseur fraîchement alloué (résultat
// de divideScalar), jamais une vue : parcourir data() dans l'ordre naturel reste correct (même
// hypothèse que Tensor::clone()).
Tensor<float> sqrtTensor(const Tensor<float>& a) {
    Tensor<float> result(a.shape());
    for (std::size_t i = 0; i < a.size(); ++i) {
        result.data()[i] = std::sqrt(a.data()[i]);
    }
    return result;
}

}  // namespace

Adam::Adam(float learningRate, float beta1, float beta2, float epsilon)
    : _learningRate(learningRate), _beta1(beta1), _beta2(beta2), _epsilon(epsilon) {}

void Adam::step(const std::vector<autodiff::NodePtr>& parameters) {
    // Facteurs de correction de biais, communs a TOUS les parametres du pas : ils ne dependent
    // que du numero de pas. Formule complete et raison d'etre dans Adam.h.
    ++_stepCount;
    const auto stepCount = static_cast<float>(_stepCount);
    const float beta1Correction = 1.0f - std::pow(_beta1, stepCount);
    const float beta2Correction = 1.0f - std::pow(_beta2, stepCount);

    for (const auto& parameter : parameters) {
        // Adresse du Node, pas du tampon de valeur : cf. commentaire équivalent dans Sgd::step.
        const void* key = parameter.get();
        auto it = _moments.find(key);
        if (it == _moments.end()) {
            it = _moments
                     .emplace(key, Moments{Tensor<float>(parameter->value.shape()),
                                           Tensor<float>(parameter->value.shape())})
                     .first;
        }
        Moments& moments = it->second;

        // m <- beta1 * m + (1 - beta1) * g : moyenne mobile du gradient.
        moments.m =
            add(multiplyScalar(moments.m, _beta1), multiplyScalar(parameter->grad, 1.0f - _beta1));
        moments.v = add(multiplyScalar(moments.v, _beta2),
                        multiplyScalar(multiply(parameter->grad, parameter->grad), 1.0f - _beta2));

        // Correction de biais : m et v partent de zero, les premiers pas seraient sinon tres en
        // deca de leur amplitude voulue (Adam.h).
        const Tensor<float> mHat = divideScalar(moments.m, beta1Correction);
        const Tensor<float> vHat = divideScalar(moments.v, beta2Correction);

        // theta <- theta - lr * mChapeau / (racine(vChapeau) + epsilon) : le pas est normalise par
        // l'amplitude recente du gradient, pas par sa valeur brute.
        const Tensor<float> update =
            multiplyScalar(divide(mHat, addScalar(sqrtTensor(vHat), _epsilon)), _learningRate);
        parameter->value = subtract(parameter->value, update);
    }
}

void Adam::zeroGrad(const std::vector<autodiff::NodePtr>& parameters) {
    optim::zeroGrad(parameters);
}

}  // namespace aisolver::optim
