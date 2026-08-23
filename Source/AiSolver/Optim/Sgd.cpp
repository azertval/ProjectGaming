// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Optim/Sgd.h"

#include "AiSolver/Math/TensorOps.h"
#include "AiSolver/Optim/OptimizerUtils.h"

namespace aisolver::optim {

Sgd::Sgd(float learningRate, float momentum) : _learningRate(learningRate), _momentum(momentum) {}

void Sgd::step(const std::vector<autodiff::NodePtr>& parameters) {
    for (const auto& parameter : parameters) {
        if (_momentum == 0.0f) {
            parameter->value =
                subtract(parameter->value, multiplyScalar(parameter->grad, _learningRate));
            continue;
        }

        // Adresse du Node : identité stable du paramètre pour toute la durée de vie d'un
        // nn::Dense (cf. décision de cadrage de l'épic). L'adresse du tampon de *valeur*, elle,
        // change à chaque step() : `parameter->value` est réassignée à un nouveau Tensor (résultat
        // de add()/subtract() ci-dessous), pas mise à jour en place.
        const void* key = parameter.get();
        auto [entry, inserted] = _velocity.try_emplace(key, parameter->value.shape());
        Tensor<float>& velocity = entry->second;

        velocity = subtract(multiplyScalar(velocity, _momentum),
                            multiplyScalar(parameter->grad, _learningRate));
        parameter->value = add(parameter->value, velocity);
    }
}

void Sgd::zeroGrad(const std::vector<autodiff::NodePtr>& parameters) {
    optim::zeroGrad(parameters);
}

}  // namespace aisolver::optim
