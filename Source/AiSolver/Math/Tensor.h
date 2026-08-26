// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

#include "Core/Diagnostics/Assert.h"

/**
 * @file AiSolver/Math/Tensor.h
 * @brief Conteneur tensoriel N-D générique (aisolver::Tensor), header-only.
 */

namespace aisolver {

namespace detail {

/// Produit des dimensions d'une forme (nombre total d'éléments) ; 1 pour une forme vide (rang 0).
[[nodiscard]] inline std::size_t shapeProduct(const std::vector<std::size_t>& shape) {
    std::size_t product = 1;
    for (std::size_t dimension : shape) {
        product *= dimension;
    }
    return product;
}

/// Strides *row-major* (dernier axe contigu) d'un tampon dense pour la forme donnée.
[[nodiscard]] inline std::vector<std::size_t> rowMajorStrides(
    const std::vector<std::size_t>& shape) {
    std::vector<std::size_t> strides(shape.size());
    std::size_t stride = 1;
    for (std::size_t axis = shape.size(); axis-- > 0;) {
        strides[axis] = stride;
        stride *= shape[axis];
    }
    return strides;
}

}  // namespace detail

/**
 * @brief Conteneur numérique N-dimensionnel à forme et *stride* explicites.
 *
 * Stockage contigu *row-major*, tampon partagé par pointeur intelligent : les vues (`view()`)
 * réinterprètent le même tampon sans copie, `clone()` en produit une copie profonde indépendante.
 *
 * @warning La copie implicite **partage** le tampon, elle ne le duplique pas : deux `Tensor`
 * copiés l'un de l'autre écrivent dans la même mémoire. C'est ce qui rend `view()` gratuit, mais
 * cela vaut aussi pour un simple passage par valeur -- `autodiff::variable(observation)` construit
 * ainsi un nœud dont `value` aliase l'observation d'origine. Utiliser `clone()` dès qu'une copie
 * indépendante est attendue.
 *
 * @tparam T Type des éléments (seule `Tensor<float>` est testée et utilisée en pratique).
 */
template <typename T>
class Tensor {
public:
    /**
     * @brief Alloue un tenseur de la forme donnée, éléments initialisés à `T{}`.
     * @param shape Dimensions du tenseur.
     */
    explicit Tensor(std::vector<std::size_t> shape)
        : _buffer(std::make_shared<std::vector<T>>(detail::shapeProduct(shape))),
          _shape(std::move(shape)),
          _strides(detail::rowMajorStrides(_shape)) {}

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

    /**
     * @brief Le tenseur couvre-t-il son tampon dans l'ordre *row-major* naturel, sans trou ?
     *
     * Vrai pour tout tenseur construit par forme, et pour toute vue de même volume : leurs
     * *strides* sont ceux de `detail::rowMajorStrides`. Faux dès qu'une vue permute ou saute des
     * positions.
     *
     * Sert de **chemin rapide** aux parcours élément par élément (`TensorOps`) : quand il est vrai,
     * l'élément de rang `n` est exactement `data()[n]`, et toute l'arithmétique d'indices
     * multi-axes disparaît. Un faux négatif reste correct — il retombe sur le parcours général.
     */
    [[nodiscard]] bool isContiguous() const noexcept {
        std::size_t expected = 1;
        for (std::size_t axis = _shape.size(); axis-- > 0;) {
            if (_strides[axis] != expected) {
                return false;
            }
            expected *= _shape[axis];
        }
        return true;
    }

    /// @return Nombre total d'éléments (produit des dimensions).
    [[nodiscard]] std::size_t size() const {
        return _buffer->size();
    }

    /**
     * @brief Accès à un élément par ses indices (un indice par dimension).
     * @param indices Position dans le tenseur, un indice par dimension.
     * @pre `indices.size() == rank()`, chaque indice strictement inférieur à sa dimension.
     */
    T& at(std::initializer_list<std::size_t> indices) {
        return (*_buffer)[offsetOf(indices)];
    }

    /// @overload
    [[nodiscard]] const T& at(std::initializer_list<std::size_t> indices) const {
        return (*_buffer)[offsetOf(indices)];
    }

    /// @return Pointeur brut sur le début du tampon partagé -- jamais décalé par la vue : une vue
    /// n'est qu'un jeu de dimensions et de pas sur ce même tampon.
    [[nodiscard]] T* data() {
        return _buffer->data();
    }

    /// @overload
    [[nodiscard]] const T* data() const {
        return _buffer->data();
    }

    /**
     * @brief Vue partageant le même tampon, avec une nouvelle forme (même volume).
     *
     * Ne copie jamais les données : écrire dans la vue modifie le tenseur d'origine à la
     * position correspondante, et réciproquement (contrairement à `clone()`).
     *
     * @param newShape Nouvelle forme ; `product(newShape)` doit rester égal à `size()`.
     */
    [[nodiscard]] Tensor<T> view(std::vector<std::size_t> newShape) const {
        PROJECTGAMING_ASSERT(detail::shapeProduct(newShape) == size(),
                             "Tensor::view() : le volume de la nouvelle forme doit rester "
                             "identique au nombre d'elements du tenseur");
        std::vector<std::size_t> newStrides = detail::rowMajorStrides(newShape);
        return Tensor<T>(_buffer, std::move(newShape), std::move(newStrides));
    }

    /**
     * @brief Copie profonde : nouveau tampon indépendant, mêmes valeurs et forme.
     *
     * Contrairement à `view()`, modifier la copie renvoyée n'affecte jamais le tenseur d'origine.
     */
    [[nodiscard]] Tensor<T> clone() const {
        auto buffer = std::make_shared<std::vector<T>>(*_buffer);
        return Tensor<T>(std::move(buffer), _shape, _strides);
    }

private:
    /// Constructeur interne (tampon/forme/strides déjà connus), utilisé par `view()`/`clone()`.
    Tensor(std::shared_ptr<std::vector<T>> buffer, std::vector<std::size_t> shape,
           std::vector<std::size_t> strides)
        : _buffer(std::move(buffer)), _shape(std::move(shape)), _strides(std::move(strides)) {}

    /// Calcule l'offset linéaire (produit scalaire indices·strides) d'une position, avec assertion.
    [[nodiscard]] std::size_t offsetOf(std::initializer_list<std::size_t> indices) const {
        PROJECTGAMING_ASSERT(indices.size() == rank(),
                             "Tensor::at() : nombre d'indices different du rang du tenseur");
        std::size_t offset = 0;
        std::size_t axis = 0;
        for (std::size_t index : indices) {
            PROJECTGAMING_ASSERT(index < _shape[axis], "Tensor::at() : indice hors bornes");
            offset += index * _strides[axis];
            ++axis;
        }
        return offset;
    }

    std::shared_ptr<std::vector<T>> _buffer;
    std::vector<std::size_t> _shape;
    std::vector<std::size_t> _strides;
};

}  // namespace aisolver
