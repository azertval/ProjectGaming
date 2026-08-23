// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <vector>

#include "AiSolver/Math/Rng.h"
#include "AiSolver/Math/Tensor.h"

/**
 * @file AiSolver/Training/Advanced/ReplayBuffer.h
 * @brief Mémoire de rejeu de transitions (`LOT-ANNEXE-14`, TACHE-01, `EX-IA-015`).
 */

namespace aisolver::training {

/// Une transition observée : `(observation, action, récompense, observation_suivante, fin
/// d'épisode)` — donnée pure, aucune logique de calcul (même décision de cadrage que
/// `TrajectoryStep`, `LOT-ANNEXE-12`).
struct Transition {
    Tensor<float> observation{{0}};
    std::size_t actionIndex = 0;
    float reward = 0.0f;
    Tensor<float> nextObservation{{0}};
    /// `true` si `nextObservation` correspond à un état terminal (l'épisode s'est arrêté à ce pas) :
    /// la cible de Bellman (`DqnLoss.h`) ignore alors `max_a Q_cible(nextObservation, a)`.
    bool done = false;
};

/**
 * @brief Mémoire circulaire de capacité fixe : au-delà de sa capacité, chaque `push` évince la
 * transition la plus ancienne (tampon en anneau, jamais de réallocation en cours de run).
 *
 * `sample` tire uniformément dans l'ensemble des transitions actuellement stockées (avec remise,
 * même transition potentiellement tirée plusieurs fois dans un même mini-lot) : aucun biais vers les
 * transitions les plus récemment poussées.
 */
class ReplayBuffer {
public:
    /// @param capacity Nombre maximal de transitions conservées ; `PROJECTGAMING_ASSERT(capacity >
    /// 0)`.
    explicit ReplayBuffer(std::size_t capacity);

    /// @brief Ajoute une transition ; évince la plus ancienne si `size() == capacity()`.
    void push(Transition transition);

    /**
     * @brief Tire un mini-lot uniformément (avec remise) parmi les transitions stockées.
     * @param batchSize Nombre de transitions à tirer.
     * @param rng       Générateur déterministe fourni par l'appelant.
     * @pre `size() > 0`.
     */
    [[nodiscard]] std::vector<Transition> sample(std::size_t batchSize, Rng& rng) const;

    /// @return Nombre de transitions actuellement stockées (`<= capacity()`).
    [[nodiscard]] std::size_t size() const noexcept {
        return _transitions.size();
    }

    /// @return Capacité maximale, fixée à la construction.
    [[nodiscard]] std::size_t capacity() const noexcept {
        return _capacity;
    }

private:
    std::size_t _capacity;
    std::vector<Transition> _transitions;
    /// Position d'écriture du prochain `push` une fois la capacité atteinte (anneau).
    std::size_t _writeIndex = 0;
};

}  // namespace aisolver::training
