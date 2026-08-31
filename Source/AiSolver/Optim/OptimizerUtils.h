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

/**
 * @brief Écrête les gradients de @p parameters à une **norme globale** de @p maxNorm.
 *
 * Norme calculée sur tous les paramètres pris ensemble, jamais paramètre par paramètre : c'est la
 * direction de la mise à jour qu'il faut préserver, et une normalisation par tenseur la
 * déformerait. Si la norme dépasse `maxNorm`, tous les gradients sont multipliés par le même
 * facteur `maxNorm / norme` ; sinon rien n'est modifié.
 *
 * Une seule mise à jour de policy gradient suffit à détruire une politique quand un épisode
 * produit un retour aberrant — et un épisode dure désormais plusieurs milliers de pas, ce qui rend
 * un tel retour d'autant plus probable. L'écrêtage borne le déplacement de chaque pas
 * d'optimisation sans changer sa direction.
 * @param parameters Paramètres dont les gradients sont écrêtés en place.
 * @param maxNorm    Norme maximale ; une valeur nulle ou négative désactive l'écrêtage.
 * @return La norme globale **avant** écrêtage (grandeur de diagnostic).
 */
float clipGradientNorm(const std::vector<autodiff::NodePtr>& parameters, float maxNorm);

}  // namespace aisolver::optim
