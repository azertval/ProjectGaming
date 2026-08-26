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
 * @note Complexité O(m·k·n), triple boucle directe, sans découpage en blocs ni parallélisation.
 * L'adressage, lui, est sorti des boucles : `Tensor::at()` reconstruirait une `initializer_list`,
 * déréférencerait le tampon partagé et relirait deux `std::vector` du tas à **chaque**
 * multiplication-addition, alors que les pas sont invariants. L'ordre `i, j, p` est **conservé** :
 * c'est lui qui fixe l'ordre des sommations flottantes, donc le résultat au bit près — le dépot
 * éprouve la reproductibilité intégrale de ses entraînements.
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

    const T* aData = a.data();
    const T* bData = b.data();
    const std::size_t aRowStride = a.strides()[0];
    const std::size_t aColStride = a.strides()[1];
    const std::size_t bRowStride = b.strides()[0];
    const std::size_t bColStride = b.strides()[1];

    Tensor<T> result({m, n});
    T* out = result.data();
    const std::size_t outRowStride = result.strides()[0];
    const std::size_t outColStride = result.strides()[1];

    for (std::size_t i = 0; i < m; ++i) {
        const T* aRow = aData + (i * aRowStride);
        T* outRow = out + (i * outRowStride);
        for (std::size_t j = 0; j < n; ++j) {
            T total{};
            for (std::size_t p = 0; p < k; ++p) {
                total = static_cast<T>(total + aRow[p * aColStride] *
                                                   bData[(p * bRowStride) + (j * bColStride)]);
            }
            outRow[j * outColStride] = total;
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

    const T* aData = a.data();
    const std::size_t aRowStride = a.strides()[0];
    const std::size_t aColStride = a.strides()[1];

    Tensor<T> result({cols, rows});
    T* out = result.data();
    const std::size_t outRowStride = result.strides()[0];
    const std::size_t outColStride = result.strides()[1];

    for (std::size_t i = 0; i < rows; ++i) {
        const T* aRow = aData + (i * aRowStride);
        for (std::size_t j = 0; j < cols; ++j) {
            out[(j * outRowStride) + (i * outColStride)] = aRow[j * aColStride];
        }
    }
    return result;
}

}  // namespace aisolver
