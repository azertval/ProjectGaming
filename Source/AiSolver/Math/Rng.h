#pragma once

/**
 * @file Source/AiSolver/Math/Rng.h
 * @brief Générateur de nombres pseudo-aléatoires (PRNG) pour
 * l'IA
 */

namespace aisolver {
    /**
     * @brief Générateur de nombres pseudo-aléatoires (PRNG) pour l'IA
     */
    class Rng {

    public:
        /**
         * @brief Constructeur du générateur de nombres pseudo-aléatoires
         * @param seed Graine pour le générateur
         */
        explicit Rng(std::uint64_t seed);
        /**
         * @brief Génère un nombre flottant aléatoire entre 0 et 1
         * pas de constructeur par défaut (une graine doit toujours être fournie explicitement, jamais dérivée de l'horloge).
         * @return Nombre flottant aléatoire
         */
        float nextFloat();
        /**
         * @brief Génère un nombre flottant aléatoire entre min et max
         * @param min Valeur minimale
         * @param max Valeur maximale
         * @return Nombre flottant aléatoire
         */
        float nextFloat(float min, float max);
        /**
         * @brief Génère un nombre flottant aléatoire selon une distribution gaussienne
         * @param mean Valeur moyenne
         * @param stddev Écart-type
         * @return Nombre flottant aléatoire
         */
        float nextGaussian(float mean = 0.0f, float stddev = 1.0f);
        /**
        * @brief Génère un nombre entier aléatoire entre min et max
        * @param min Valeur minimale
        * @param max Valeur maximale
        * @return Nombre entier aléatoire
        */
        int nextInt(int min, int max);
    private:

    }

}  // namespace aisolver
