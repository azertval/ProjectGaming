#pragma once

#include "AiSolver/Math/Tensor.h"

/**
 * @file AiSolver/Math/TensorOps.h
 * @brief Opérations élémentaires et réductions sur `Tensor<T>` (header-only).
 *
 * @note Squelette (LOT-ANNEXE-01, TACHE-03) : signatures et câblage du module `AiSolver` en
 * place ; corps à implémenter par la tâche elle-même, voir
 * `Documentation/Lot-Annexe/LOT-ANNEXE-01-bibliotheque-tensorielle-rng/tache-03-operations-reductions.md`.
 */

namespace aisolver {

template <typename T>
[[nodiscard]] Tensor<T> add(const Tensor<T>& a, const Tensor<T>& b) {
    (void)b;
    return a.clone();
}

template <typename T>
[[nodiscard]] Tensor<T> subtract(const Tensor<T>& a, const Tensor<T>& b) {
    (void)b;
    return a.clone();
}

template <typename T>
[[nodiscard]] Tensor<T> multiply(const Tensor<T>& a, const Tensor<T>& b) {
    (void)b;
    return a.clone();
}

template <typename T>
[[nodiscard]] Tensor<T> divide(const Tensor<T>& a, const Tensor<T>& b) {
    (void)b;
    return a.clone();
}

template <typename T>
[[nodiscard]] Tensor<T> addScalar(const Tensor<T>& a, T scalar) {
    (void)scalar;
    return a.clone();
}

template <typename T>
[[nodiscard]] Tensor<T> subtractScalar(const Tensor<T>& a, T scalar) {
    (void)scalar;
    return a.clone();
}

template <typename T>
[[nodiscard]] Tensor<T> multiplyScalar(const Tensor<T>& a, T scalar) {
    (void)scalar;
    return a.clone();
}

template <typename T>
[[nodiscard]] Tensor<T> divideScalar(const Tensor<T>& a, T scalar) {
    (void)scalar;
    return a.clone();
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
    (void)a;
    return T{};
}

/// @return Moyenne de tous les éléments.
template <typename T>
[[nodiscard]] T mean(const Tensor<T>& a) {
    (void)a;
    return T{};
}

/// @return Maximum de tous les éléments.
template <typename T>
[[nodiscard]] T max(const Tensor<T>& a) {
    (void)a;
    return T{};
}

}  // namespace aisolver
