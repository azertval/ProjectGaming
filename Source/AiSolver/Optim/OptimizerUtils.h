// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "AiSolver/Math/Autodiff/Node.h"

/**
 * @file AiSolver/Optim/OptimizerUtils.h
 * @brief Utilitaires partagés entre optimiseurs (LOT-ANNEXE-04) : remise à zéro des gradients.
 */

namespace aisolver::optim {

/**
 * @brief Remet à zéro le gradient de chaque paramètre fourni (`Node::zeroGrad()`).
 *
 * Fonction libre partagée par `Sgd`/`Adam` (`IOptimizer::zeroGrad`) : `LOT-ANNEXE-02` accumule
 * (`+=`) plutôt que d'écraser, donc un appelant doit remettre les gradients à zéro explicitement
 * entre deux passes qui ne doivent pas s'additionner.
 * @param parameters Paramètres dont le gradient est remis à zéro ; les autres paramètres d'un
 *                    même réseau ne sont pas affectés.
 */
void zeroGrad(const std::vector<autodiff::NodePtr>& parameters);

}  // namespace aisolver::optim
