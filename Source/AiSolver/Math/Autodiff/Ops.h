#pragma once

#include <cstddef>

#include "AiSolver/Math/Autodiff/Node.h"

/**
 * @file AiSolver/Math/Autodiff/Ops.h
 * @brief Opérations différentiables de base (LOT-ANNEXE-02, TACHE-02) et complémentaires
 * (LOT-ANNEXE-03, TACHE-05), construites au-dessus de `unaryOp`/`binaryOp` (LOT-ANNEXE-02, TACHE-01).
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

// --- Opérations complémentaires (LOT-ANNEXE-03, TACHE-05) --------------------------------------
// Complètent l'ensemble ci-dessus, reporté ici par LOT-ANNEXE-02 faute de consommateur à l'époque
// (le premier étant la perte de policy gradient, LOT-ANNEXE-12). Même méthode : construites via
// unaryOp/binaryOp, sans modifier Node.h/Node.cpp.

/// @return Nœud résultat de `a->value - b->value` ; gradient vers `a` = gradient de sortie inchangé,
/// gradient vers `b` = opposé du gradient de sortie.
[[nodiscard]] NodePtr subtract(const NodePtr& a, const NodePtr& b);

/// @return Nœud résultat de `a->value / b->value` (élément par élément) ; règle du quotient.
/// `PROJECTGAMING_ASSERT` que tous les éléments de `b` sont non nuls.
[[nodiscard]] NodePtr divide(const NodePtr& a, const NodePtr& b);

/// @return Nœud résultat de `a->value + scalar` ; le scalaire est une constante, gradient vers `a`
/// inchangé.
[[nodiscard]] NodePtr addScalar(const NodePtr& a, float scalar);

/// @return Nœud résultat de `a->value * scalar` ; le scalaire est une constante **détachée** du
/// graphe (ex. le retour `G_t` d'une perte de policy gradient), jamais un `NodePtr`.
[[nodiscard]] NodePtr multiplyScalar(const NodePtr& a, float scalar);

/// @brief Logarithme différentiable (nommé `logOp`, pas `log`, même raison que `tanhOp`).
/// @return Nœud résultat de `std::log(x)` élément par élément ; `PROJECTGAMING_ASSERT` que tous les
/// éléments d'entrée sont strictement positifs.
[[nodiscard]] NodePtr logOp(const NodePtr& a);

/// @return Nœud résultat de `std::exp(x)` élément par élément ; gradient vers `a` = gradient de
/// sortie × valeur de sortie (`exp` est sa propre dérivée, valeur déjà calculée en avant).
[[nodiscard]] NodePtr expOp(const NodePtr& a);

/// @return Nœud de forme `[1]` contenant `a->value` à `index` (rang 1 ou forme `[n, 1]`).
/// `PROJECTGAMING_ASSERT(index < a->value.size())`. Gradient vers `a` : nul partout sauf à `index`,
/// où il vaut le gradient de sortie.
[[nodiscard]] NodePtr selectIndex(const NodePtr& a, std::size_t index);

/// @return Minimum élément par élément. Gradient : entièrement transmis à l'opérande minimal pour
/// chaque élément, `0` à l'autre ; en cas d'égalité stricte, transmis à `a` (sous-gradient,
/// convention arbitraire documentée — la fonction n'est pas dérivable au point d'égalité).
[[nodiscard]] NodePtr minimum(const NodePtr& a, const NodePtr& b);

/// @return Chaque élément de `a->value` borné dans `[low, high]`. `PROJECTGAMING_ASSERT(low <=
/// high)`. Gradient vers `a` : transmis inchangé pour les éléments strictement à l'intérieur des
/// bornes, `0` pour ceux qui ont été rognés (leur valeur ne dépend plus de l'entrée).
[[nodiscard]] NodePtr clamp(const NodePtr& a, float low, float high);

}  // namespace aisolver::autodiff
