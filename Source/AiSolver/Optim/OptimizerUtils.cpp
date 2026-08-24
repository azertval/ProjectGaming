// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Optim/OptimizerUtils.h"

namespace aisolver::optim {

void zeroGrad(const std::vector<autodiff::NodePtr>& parameters) {
    for (const auto& parameter : parameters) {
        parameter->zeroGrad();
    }
}

}  // namespace aisolver::optim
