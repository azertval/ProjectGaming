// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

/**
 * @file AiSolver/Training/Evolutionary/EvolutionaryConfig.h
 * @brief Constantes et configuration de l'algorithme évolutionniste (`LOT-ANNEXE-10`, `EX-IA-011`).
 */

namespace aisolver::training::evolutionary {

/// Taille de population par défaut : compromis diversité (plus d'individus explorent plus de
/// directions par génération) / coût d'évaluation (chaque individu joue un niveau complet).
inline constexpr std::size_t DEFAULT_POPULATION_SIZE = 32;

/// Nombre d'individus tirés (avec remise) par sélection par tournoi : `O(tournamentSize)` par
/// tirage, sans trier toute la population — suffisant pour une population de taille modeste.
inline constexpr int DEFAULT_TOURNAMENT_SIZE = 3;

/// Probabilité, par poids, qu'une mutation gaussienne soit appliquée.
inline constexpr float DEFAULT_MUTATION_RATE = 0.05f;

/// Écart-type du bruit gaussien ajouté à un poids muté.
inline constexpr float DEFAULT_MUTATION_STRENGTH = 0.1f;

/// Regroupe les paramètres de l'algorithme évolutionniste, valeurs par défaut documentées
/// ci-dessus (constantes nommées, jamais de littéral magique répété).
struct EvolutionaryConfig {
    std::size_t populationSize = DEFAULT_POPULATION_SIZE;
    int tournamentSize = DEFAULT_TOURNAMENT_SIZE;
    float mutationRate = DEFAULT_MUTATION_RATE;
    float mutationStrength = DEFAULT_MUTATION_STRENGTH;
};

}  // namespace aisolver::training::evolutionary
