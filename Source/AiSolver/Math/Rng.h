#pragma once

#include <cstdint>
#include <random>

/**
 * @file AiSolver/Math/Rng.h
 * @brief Générateur pseudo-aléatoire déterministe du programme d'IA maison (aisolver::Rng).
 */

namespace aisolver {

/**
 * @brief Source d'aléatoire déterministe et reproductible à partir d'une seule graine.
 *
 * @note Squelette (LOT-ANNEXE-01, TACHE-01) : signatures et câblage du module `AiSolver` en
 * place ; corps des méthodes à implémenter par la tâche elle-même, voir
 * `Documentation/Lot-Annexe/LOT-ANNEXE-01-bibliotheque-tensorielle-rng/tache-01-rng-deterministe.md`.
 */
class Rng {
public:
    /**
     * @brief Construit le générateur à partir d'une graine explicite.
     * @param seed Graine ; aucune valeur par défaut (jamais dérivée de l'horloge).
     */
    explicit Rng(std::uint64_t seed);

    /// @return Flottant uniforme dans `[0, 1)`.
    [[nodiscard]] float nextFloat();

    /**
     * @brief Flottant uniforme dans un intervalle donné.
     * @param min Borne inférieure (incluse).
     * @param max Borne supérieure (exclue).
     */
    [[nodiscard]] float nextFloat(float min, float max);

    /**
     * @brief Tirage gaussien (transformation de Box-Muller).
     * @param mean   Moyenne de la loi normale.
     * @param stddev Écart-type de la loi normale.
     */
    [[nodiscard]] float nextGaussian(float mean = 0.0f, float stddev = 1.0f);

    /**
     * @brief Entier uniforme dans un intervalle donné (bornes incluses), par rejet.
     * @param min Borne inférieure (incluse).
     * @param max Borne supérieure (incluse).
     */
    [[nodiscard]] int nextInt(int min, int max);

private:
    std::mt19937_64 _engine;
};

}  // namespace aisolver
