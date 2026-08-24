// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "AiSolver/Math/Autodiff/Node.h"

/**
 * @file AiSolver/Nn/Activations.h
 * @brief Fonctions d'activation différentiables manquantes après `LOT-ANNEXE-02`
 * (`autodiff::relu`, `autodiff::tanhOp`) : `sigmoid`, `softmax` (LOT-ANNEXE-03).
 */

namespace aisolver::nn {

/// @return Nœud résultat de `1 / (1 + exp(-x))` élément par élément, sortie bornée `]0,1[`.
/// Gradient vers `a` : gradient de sortie × `outputValue × (1 − outputValue)`.
[[nodiscard]] autodiff::NodePtr sigmoid(const autodiff::NodePtr& a);

/**
 * @brief Distribution de probabilité sur un vecteur colonne `[n, 1]` de scores (*logits*).
 *
 * Stabilisée numériquement : soustrait le maximum du vecteur avant l'exponentielle
 * (`softmax(x) == softmax(x − max(x))`, évite un débordement de `exp` sur de grands *logits*).
 * Gradient vers `a` : produit jacobien-vecteur direct (`∂softmax_i/∂x_j = softmax_i×(δ_ij −
 * softmax_j)`), sans matérialiser la jacobienne `n×n`.
 */
[[nodiscard]] autodiff::NodePtr softmax(const autodiff::NodePtr& a);

}  // namespace aisolver::nn
