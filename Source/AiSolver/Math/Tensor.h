#pragma once

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

/**
 * @file AiSolver/Math/Tensor.h
 * @brief Conteneur tensoriel N-D générique (aisolver::Tensor), header-only.
 */

namespace aisolver {

/**
 * @brief Conteneur numérique N-dimensionnel à forme et *stride* explicites.
 *
 * @tparam T Type des éléments (seule `Tensor<float>` est testée et utilisée en pratique).
 *
 * @note Squelette (LOT-ANNEXE-01, TACHE-02) : signatures et câblage du module `AiSolver` en
 * place ; corps à implémenter par la tâche elle-même (stockage contigu *row-major*, tampon
 * partagé par `view()`), voir
 * `Documentation/Lot-Annexe/LOT-ANNEXE-01-bibliotheque-tensorielle-rng/tache-02-tensor-nd.md`.
 */
template <typename T>
class Tensor {
public:
    /**
     * @brief Alloue un tenseur de la forme donnée, éléments initialisés à `T{}`.
     * @param shape Dimensions du tenseur.
     */
    explicit Tensor(std::vector<std::size_t> shape) : _shape(std::move(shape)) {}

    /// @return Nombre de dimensions du tenseur.
    [[nodiscard]] std::size_t rank() const {
        return _shape.size();
    }

    /// @return Dimensions du tenseur.
    [[nodiscard]] const std::vector<std::size_t>& shape() const {
        return _shape;
    }

    /// @return *Strides* (pas, en éléments) de chaque dimension.
    [[nodiscard]] const std::vector<std::size_t>& strides() const {
        return _strides;
    }

    /// @return Nombre total d'éléments (produit des dimensions).
    [[nodiscard]] std::size_t size() const {
        return _buffer ? _buffer->size() : 0;
    }

    /**
     * @brief Accès à un élément par ses indices (un indice par dimension).
     * @param indices Position dans le tenseur.
     */
    T& at(std::initializer_list<std::size_t> indices) {
        (void)indices;
        static T placeholder{};
        return placeholder;
    }

    /// @overload
    [[nodiscard]] const T& at(std::initializer_list<std::size_t> indices) const {
        (void)indices;
        static const T placeholder{};
        return placeholder;
    }

    /// @return Pointeur brut sur le tampon (premier élément de la vue courante).
    [[nodiscard]] T* data() {
        return _buffer ? _buffer->data() : nullptr;
    }

    /// @overload
    [[nodiscard]] const T* data() const {
        return _buffer ? _buffer->data() : nullptr;
    }

    /**
     * @brief Vue partageant le même tampon, avec une nouvelle forme (même volume).
     * @param newShape Nouvelle forme (produit des dimensions inchangé).
     */
    [[nodiscard]] Tensor<T> view(std::vector<std::size_t> newShape) const {
        return Tensor<T>(std::move(newShape));
    }

    /// @return Copie profonde (nouveau tampon, mêmes valeurs).
    [[nodiscard]] Tensor<T> clone() const {
        return Tensor<T>(_shape);
    }

private:
    /// Constructeur interne (tampon/forme/strides déjà connus), utilisé par `view()`.
    Tensor(std::shared_ptr<std::vector<T>> buffer, std::vector<std::size_t> shape,
           std::vector<std::size_t> strides)
        : _buffer(std::move(buffer)), _shape(std::move(shape)), _strides(std::move(strides)) {}

    std::shared_ptr<std::vector<T>> _buffer;
    std::vector<std::size_t> _shape;
    std::vector<std::size_t> _strides;
};

}  // namespace aisolver
