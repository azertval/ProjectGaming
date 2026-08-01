#pragma once

#include "AiSolver/Math/Tensor.h"

/**
 * @file AiSolver/Math/Matmul.h
 * @brief Produit matriciel et transposition, restreints aux tenseurs de rang 2 (header-only).
 *
 * @note Squelette (LOT-ANNEXE-01, TACHE-04) : signatures et câblage du module `AiSolver` en
 * place ; corps à implémenter par la tâche elle-même, voir
 * `Documentation/Lot-Annexe/LOT-ANNEXE-01-bibliotheque-tensorielle-rng/tache-04-produit-matriciel-transposition.md`.
 */

namespace aisolver {

/**
 * @brief Produit matriciel standard entre deux matrices (rang 2) `[m,k] * [k,n] -> [m,n]`.
 * @note Complexité O(m·k·n) une fois implémenté, sans optimisation par blocs (cf. exclusions de
 * l'épic LOT-ANNEXE-01 : pas d'objectif de performance).
 */
template <typename T>
[[nodiscard]] Tensor<T> matmul(const Tensor<T>& a, const Tensor<T>& b) {
    (void)b;
    return a.clone();
}

/// @brief Transposition d'une matrice (rang 2) : renvoie une copie de forme `[n, m]`.
template <typename T>
[[nodiscard]] Tensor<T> transpose(const Tensor<T>& a) {
    return a.clone();
}

}  // namespace aisolver
