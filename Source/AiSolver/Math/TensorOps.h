#pragma once

#include <cstddef>
#include <vector>

#include "AiSolver/Math/Tensor.h"
#include "Core/Diagnostics/Assert.h"

/**
 * @file AiSolver/Math/TensorOps.h
 * @brief Opérations élémentaires et réductions sur `Tensor<T>` (header-only).
 */

namespace aisolver {

namespace detail {

/**
 * @brief Parcourt les offsets linéaires d'un tenseur en ordre logique *row-major*.
 *
 * Recalcule l'offset à partir des indices et des *strides* à chaque position (ne suppose jamais
 * que le tampon est dense dans l'ordre naturel) : reste correct si `Tensor` gagne un jour des
 * sous-vues à *strides* non canoniques (hors périmètre de LOT-ANNEXE-01).
 */
template <typename T, typename Fn>
void forEachOffset(const Tensor<T>& a, Fn&& fn) {
    const std::size_t rank = a.rank();
    std::vector<std::size_t> index(rank, 0);
    for (std::size_t linear = 0; linear < a.size(); ++linear) {
        std::size_t offset = 0;
        for (std::size_t axis = 0; axis < rank; ++axis) {
            offset += index[axis] * a.strides()[axis];
        }
        fn(offset);
        for (std::size_t axis = rank; axis-- > 0;) {
            if (++index[axis] < a.shape()[axis]) {
                break;
            }
            index[axis] = 0;
        }
    }
}

/// Applique `op` élément par élément entre deux tenseurs de même forme (voir `forEachOffset`).
template <typename T, typename BinaryOp>
[[nodiscard]] Tensor<T> elementwise(const Tensor<T>& a, const Tensor<T>& b, BinaryOp op) {
    PROJECTGAMING_ASSERT(a.shape() == b.shape(), "Operation elementwise : formes incompatibles");
    Tensor<T> result(a.shape());
    T* out = result.data();
    const std::size_t rank = a.rank();
    std::vector<std::size_t> index(rank, 0);
    for (std::size_t linear = 0; linear < a.size(); ++linear) {
        std::size_t offsetA = 0;
        std::size_t offsetB = 0;
        for (std::size_t axis = 0; axis < rank; ++axis) {
            offsetA += index[axis] * a.strides()[axis];
            offsetB += index[axis] * b.strides()[axis];
        }
        out[linear] = op(a.data()[offsetA], b.data()[offsetB]);
        for (std::size_t axis = rank; axis-- > 0;) {
            if (++index[axis] < a.shape()[axis]) {
                break;
            }
            index[axis] = 0;
        }
    }
    return result;
}

/// Applique `op` à chaque élément d'un tenseur (voir `forEachOffset`).
template <typename T, typename UnaryOp>
[[nodiscard]] Tensor<T> elementwiseUnary(const Tensor<T>& a, UnaryOp op) {
    Tensor<T> result(a.shape());
    T* out = result.data();
    std::size_t linear = 0;
    forEachOffset(a, [&](std::size_t offset) { out[linear++] = op(a.data()[offset]); });
    return result;
}

}  // namespace detail

/// @return Somme élément par élément ; `PROJECTGAMING_ASSERT` si les formes diffèrent.
template <typename T>
[[nodiscard]] Tensor<T> add(const Tensor<T>& a, const Tensor<T>& b) {
    return detail::elementwise(a, b, [](T x, T y) { return static_cast<T>(x + y); });
}

/// @return Différence élément par élément ; `PROJECTGAMING_ASSERT` si les formes diffèrent.
template <typename T>
[[nodiscard]] Tensor<T> subtract(const Tensor<T>& a, const Tensor<T>& b) {
    return detail::elementwise(a, b, [](T x, T y) { return static_cast<T>(x - y); });
}

/// @return Produit élément par élément ; `PROJECTGAMING_ASSERT` si les formes diffèrent.
template <typename T>
[[nodiscard]] Tensor<T> multiply(const Tensor<T>& a, const Tensor<T>& b) {
    return detail::elementwise(a, b, [](T x, T y) { return static_cast<T>(x * y); });
}

/// @return Quotient élément par élément ; `PROJECTGAMING_ASSERT` si les formes diffèrent.
template <typename T>
[[nodiscard]] Tensor<T> divide(const Tensor<T>& a, const Tensor<T>& b) {
    return detail::elementwise(a, b, [](T x, T y) { return static_cast<T>(x / y); });
}

/// @return Chaque élément de `a` additionné au même scalaire (diffusion scalaire).
template <typename T>
[[nodiscard]] Tensor<T> addScalar(const Tensor<T>& a, T scalar) {
    return detail::elementwiseUnary(a, [scalar](T x) { return static_cast<T>(x + scalar); });
}

/// @return Chaque élément de `a` diminué du même scalaire (diffusion scalaire).
template <typename T>
[[nodiscard]] Tensor<T> subtractScalar(const Tensor<T>& a, T scalar) {
    return detail::elementwiseUnary(a, [scalar](T x) { return static_cast<T>(x - scalar); });
}

/// @return Chaque élément de `a` multiplié par le même scalaire (diffusion scalaire).
template <typename T>
[[nodiscard]] Tensor<T> multiplyScalar(const Tensor<T>& a, T scalar) {
    return detail::elementwiseUnary(a, [scalar](T x) { return static_cast<T>(x * scalar); });
}

/// @return Chaque élément de `a` divisé par le même scalaire (diffusion scalaire).
template <typename T>
[[nodiscard]] Tensor<T> divideScalar(const Tensor<T>& a, T scalar) {
    return detail::elementwiseUnary(a, [scalar](T x) { return static_cast<T>(x / scalar); });
}

template <typename T>
[[nodiscard]] Tensor<T> operator+(const Tensor<T>& a, const Tensor<T>& b) {
    return add(a, b);
}

template <typename T>
[[nodiscard]] Tensor<T> operator-(const Tensor<T>& a, const Tensor<T>& b) {
    return subtract(a, b);
}

template <typename T>
[[nodiscard]] Tensor<T> operator*(const Tensor<T>& a, const Tensor<T>& b) {
    return multiply(a, b);
}

template <typename T>
[[nodiscard]] Tensor<T> operator/(const Tensor<T>& a, const Tensor<T>& b) {
    return divide(a, b);
}

template <typename T>
[[nodiscard]] Tensor<T> operator+(const Tensor<T>& a, T scalar) {
    return addScalar(a, scalar);
}

template <typename T>
[[nodiscard]] Tensor<T> operator-(const Tensor<T>& a, T scalar) {
    return subtractScalar(a, scalar);
}

template <typename T>
[[nodiscard]] Tensor<T> operator*(const Tensor<T>& a, T scalar) {
    return multiplyScalar(a, scalar);
}

template <typename T>
[[nodiscard]] Tensor<T> operator/(const Tensor<T>& a, T scalar) {
    return divideScalar(a, scalar);
}

/// @return Somme de tous les éléments (réduction globale).
template <typename T>
[[nodiscard]] T sum(const Tensor<T>& a) {
    T total{};
    detail::forEachOffset(a, [&](std::size_t offset) { total = static_cast<T>(total + a.data()[offset]); });
    return total;
}

/// @return Moyenne de tous les éléments ; `PROJECTGAMING_ASSERT` sur un tenseur vide.
template <typename T>
[[nodiscard]] T mean(const Tensor<T>& a) {
    PROJECTGAMING_ASSERT(a.size() > 0, "mean() : tenseur vide");
    return static_cast<T>(sum(a) / static_cast<T>(a.size()));
}

/// @return Maximum de tous les éléments ; `PROJECTGAMING_ASSERT` sur un tenseur vide.
template <typename T>
[[nodiscard]] T max(const Tensor<T>& a) {
    PROJECTGAMING_ASSERT(a.size() > 0, "max() : tenseur vide");
    bool first = true;
    T best{};
    detail::forEachOffset(a, [&](std::size_t offset) {
        const T value = a.data()[offset];
        if (first || value > best) {
            best = value;
            first = false;
        }
    });
    return best;
}

}  // namespace aisolver
