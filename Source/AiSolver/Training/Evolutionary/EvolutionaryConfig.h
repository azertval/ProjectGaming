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
///
/// Valeur reprise de l'ordre de grandeur usuel des algorithmes génétiques appliqués à de petits
/// réseaux de neurones (dizaines à basse centaine d'individus, voir
/// `Documentation/Guide-Annexe/guide-annexe-algorithmes-evolutionnistes.md`) — ni chiffrée par un
/// calcul propre au projet (contrairement aux poids de `RewardConfig`, voir `Reward.h`), ni tunée
/// empiriquement ici : c'est un point de départ raisonnable, pas un optimum mesuré.
inline constexpr std::size_t DEFAULT_POPULATION_SIZE = 32;

/// Nombre d'individus tirés (avec remise) par sélection par tournoi : `O(tournamentSize)` par
/// tirage, sans trier toute la population — suffisant pour une population de taille modeste.
///
/// Même origine que `DEFAULT_POPULATION_SIZE` : une valeur typique de la littérature (souvent 2 à
/// 5 pour une population de quelques dizaines d'individus), pas un résultat de tuning propre à ce
/// projet.
inline constexpr int DEFAULT_TOURNAMENT_SIZE = 3;

/// Probabilité, par poids, qu'une mutation gaussienne soit appliquée.
///
/// Même statut que les deux constantes précédentes : un taux de mutation faible (quelques
/// pourcents par poids) est la valeur usuelle pour éviter qu'une mutation gaussienne détruise plus
/// de progrès qu'elle n'en explore ; non chiffré par un calcul spécifique à ce projet.
inline constexpr float DEFAULT_MUTATION_RATE = 0.05f;

/// Écart-type du bruit gaussien ajouté à un poids muté.
///
/// Même statut : ordre de grandeur usuel pour un poids initialisé proche de zéro, non chiffré par
/// un calcul spécifique à ce projet.
inline constexpr float DEFAULT_MUTATION_STRENGTH = 0.1f;

/// Probabilité qu'un enfant soit issu d'un croisement plutôt que d'une copie du premier parent.
///
/// Le croisement était **toujours** appliqué, et c'était une moyenne arithmétique des deux réseaux
/// parents. Or moyenner deux réseaux de neurones ne combine pas ce que chacun a trouvé : deux
/// solutions également bonnes obtenues par des routes différentes donnent une moyenne moins bonne
/// que l'une comme que l'autre (les neurones cachés n'ont aucune raison de se correspondre d'un
/// réseau à l'autre). Appliqué à chaque enfant sauf l'élite, l'opérateur détruisait donc à chaque
/// génération l'essentiel de ce que la précédente avait gagné.
///
/// À `0,7`, trois enfants sur dix descendent d'un seul parent muté — le seul chemin par lequel une
/// trouvaille locale survit intacte d'une génération à l'autre.
inline constexpr float DEFAULT_CROSSOVER_RATE = 0.7f;

/// Regroupe les paramètres de l'algorithme évolutionniste, valeurs par défaut documentées
/// ci-dessus (constantes nommées, jamais de littéral magique répété).
///
/// L'élitisme conserve **exactement un** individu (le meilleur) d'une génération à l'autre, jamais
/// un pourcentage -- voir `EvolutionaryTrainer::runGeneration`.
struct EvolutionaryConfig {
    std::size_t populationSize = DEFAULT_POPULATION_SIZE;
    int tournamentSize = DEFAULT_TOURNAMENT_SIZE;
    float mutationRate = DEFAULT_MUTATION_RATE;
    float mutationStrength = DEFAULT_MUTATION_STRENGTH;
    /// Probabilité de croiser les deux parents ; sinon l'enfant copie le premier
    /// (`DEFAULT_CROSSOVER_RATE`). `1` restitue le comportement d'origine.
    float crossoverRate = DEFAULT_CROSSOVER_RATE;
};

}  // namespace aisolver::training::evolutionary
