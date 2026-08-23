// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <deque>

/**
 * @file AiSolver/Stats/MovingAverage.h
 * @brief Moyenne mobile sur fenêtre glissante, utilisée pour détecter un plateau d'entraînement
 * (`LOT-ANNEXE-09`, `EX-IA-010`).
 */

namespace aisolver {

/**
 * @brief Moyenne mobile sur les `windowSize` dernières valeurs poussées.
 *
 * Avant que la fenêtre soit pleine, la moyenne porte sur les valeurs disponibles (pas de valeur
 * sentinelle) : une fenêtre de taille `20` retourne, après seulement `3` appels à `push`, la
 * moyenne de ces `3` valeurs, pas une moyenne biaisée par des zéros implicites.
 */
class MovingAverageTracker {
public:
    /// @param windowSize Taille de la fenêtre glissante (`N`), un paramètre documenté et non une
    /// constante magique : l'appelant choisit une largeur adaptée au bruit attendu de sa séquence
    /// (génération d'une population évolutionniste, ou épisode d'un algorithme par gradient).
    explicit MovingAverageTracker(int windowSize);

    /// @brief Ajoute `value` à la fenêtre et retourne la nouvelle moyenne mobile.
    [[nodiscard]] float push(float value);

private:
    int windowSize_;
    std::deque<float> window_;
    float sum_ = 0.0f;
};

}  // namespace aisolver
