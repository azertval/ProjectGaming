#pragma once

#include "AiSolver/Math/Autodiff/Node.h"

/**
 * @file AiSolver/Math/Autodiff/Ops.h
 * @brief Opérations différentiables de base (LOT-ANNEXE-02, TACHE-02), construites au-dessus de
 * `unaryOp`/`binaryOp` (TACHE-01).
 */

namespace aisolver::autodiff {

/// @return Nœud résultat de `a->value + b->value` ; gradient de sortie propagé inchangé aux deux
/// parents.
[[nodiscard]] NodePtr add(const NodePtr& a, const NodePtr& b);

/// @return Nœud résultat de `a->value * b->value` (élément par élément) ; règle du produit.
[[nodiscard]] NodePtr multiply(const NodePtr& a, const NodePtr& b);

/// @return Nœud résultat de `matmul(a->value, b->value)` ; dérivation matricielle standard du
/// produit (transposition du parent opposé).
[[nodiscard]] NodePtr matmul(const NodePtr& a, const NodePtr& b);

/// @return Nœud résultat de `max(0, x)` élément par élément ; gradient propagé uniquement où
/// l'entrée était strictement positive.
[[nodiscard]] NodePtr relu(const NodePtr& a);

/// @brief Tangente hyperbolique différentiable (nommée `tanhOp`, pas `tanh`, pour ne pas ombrer
/// `std::tanh`).
/// @return Nœud résultat de `tanh(x)` élément par élément ; règle `1 - tanh(x)^2`.
[[nodiscard]] NodePtr tanhOp(const NodePtr& a);

}  // namespace aisolver::autodiff
