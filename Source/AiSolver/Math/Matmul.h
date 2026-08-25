// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

#include "AiSolver/Math/Tensor.h"
#include "Core/Diagnostics/Assert.h"

/**
 * @file AiSolver/Math/Matmul.h
 * @brief Produit matriciel et transposition, restreints aux tenseurs de rang 2 (header-only).
 */

namespace aisolver {

/**
 * @brief Produit matriciel standard entre deux matrices (rang 2) `[m,k] * [k,n] -> [m,n]`.
 * @note Complexité O(m·k·n), triple boucle directe, sans optimisation par blocs — cohérent avec
 * l'exclusion de performance de l'épic LOT-ANNEXE-01 (correction visée, pas la vitesse).
 * @pre `a.rank() == 2 && b.rank() == 2 && a.shape()[1] == b.shape()[0]` (`PROJECTGAMING_ASSERT`).
 */
template <typename T>
[[nodiscard]] Tensor<T> matmul(const Tensor<T>& a, const Tensor<T>& b) {
    PROJECTGAMING_ASSERT(a.rank() == 2 && b.rank() == 2,
                         "matmul() : les deux tenseurs doivent etre de rang 2");
    PROJECTGAMING_ASSERT(
        a.shape()[1] == b.shape()[0],
        "matmul() : dimension interne incompatible (colonnes de a != lignes de b)");

    const std::size_t m = a.shape()[0];
    const std::size_t k = a.shape()[1];
    const std::size_t n = b.shape()[1];

    Tensor<T> result({m, n});
    for (std::size_t i = 0; i < m; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            T total{};
            for (std::size_t p = 0; p < k; ++p) {
                total = static_cast<T>(total + a.at({i, p}) * b.at({p, j}));
            }
            result.at({i, j}) = total;
        }
    }
    return result;
}

/**
 * @brief Transposition d'une matrice (rang 2) : renvoie une **copie** de forme `[n, m]`.
 *
 * Une copie plutôt qu'une vue à *strides* permutés : plus simple à raisonner pour ce lot, et
 * évite qu'une opération élémentaire (`TensorOps`) suppose à tort un parcours row-major strict
 * sur le résultat.
 * @pre `a.rank() == 2` (`PROJECTGAMING_ASSERT`).
 */
template <typename T>
[[nodiscard]] Tensor<T> transpose(const Tensor<T>& a) {
    PROJECTGAMING_ASSERT(a.rank() == 2, "transpose() : le tenseur doit etre de rang 2");

    const std::size_t rows = a.shape()[0];
    const std::size_t cols = a.shape()[1];

    Tensor<T> result({cols, rows});
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            result.at({j, i}) = a.at({i, j});
        }
    }
    return result;
}

}  // namespace aisolver
