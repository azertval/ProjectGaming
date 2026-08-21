// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

/**
 * @file Core/Math/DeterministicRandom.h
 * @brief Générateur pseudo-aléatoire déterministe, à graine explicite (jamais l'horloge,
 *        `EX-NFR-002`).
 */

namespace core {

/**
 * @brief Mélange (SplitMix64) un entier 64 bits en une valeur bien distribuée.
 *
 * Fonction pure et sans état : sert à combiner plusieurs valeurs reproductibles (graine de base,
 * numéro de pas, identifiant d'entité) en une graine unique par tirage, sans jamais lire
 * l'horloge système ni un générateur par défaut (`EX-NFR-002`).
 * @param value Valeur à mélanger.
 * @return La valeur mélangée.
 */
[[nodiscard]] constexpr std::uint64_t splitMix64(std::uint64_t value) noexcept {
    value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31);
}

/**
 * @brief Combine une graine de base, un numéro de pas et un identifiant reproductible en une
 *        graine unique, propre à ce triplet (`LOT-53` TACHE-01).
 *
 * Reproductible quel que soit l'ordre ou le nombre d'appels : deux tirages faits pour le même
 * triplet (graine, pas, identifiant) produisent toujours la même graine dérivée, jamais
 * dépendante d'un compteur mutable partagé entre appels.
 * @param baseSeed Graine de base, explicite.
 * @param step     Numéro de pas de simulation courant.
 * @param entityId Identifiant reproductible (`core::Entity::index`, par exemple).
 * @return Une graine dérivée, propre à ce triplet.
 */
[[nodiscard]] constexpr std::uint64_t deriveSeed(std::uint64_t baseSeed, std::uint64_t step,
                                                 std::uint64_t entityId) noexcept {
    std::uint64_t seed = splitMix64(baseSeed);
    seed = splitMix64(seed ^ step);
    seed = splitMix64(seed ^ entityId);
    return seed;
}

/**
 * @brief Générateur pseudo-aléatoire léger (SplitMix64), à graine explicite.
 *
 * Aucune dépendance à `<random>` ni à l'horloge système : deux instances construites avec la
 * même graine produisent exactement la même suite de valeurs (`EX-NFR-002`). Utilisé par
 * `core::ParticleSystem` (`LOT-53`), où tout tirage doit rester reproductible d'une exécution à
 * l'autre pour une même séquence d'entrées.
 */
class DeterministicRandom {
public:
    /// @param seed Graine explicite (jamais issue de l'horloge ou d'un générateur par défaut).
    explicit DeterministicRandom(std::uint64_t seed) noexcept : _state(seed) {}

    /// @return Le prochain entier 32 bits de la suite.
    [[nodiscard]] std::uint32_t nextUInt32() noexcept {
        _state += 0x9E3779B97F4A7C15ULL;
        return static_cast<std::uint32_t>(splitMix64(_state) >> 32);
    }

    /// @return Le prochain flottant dans [0, 1[.
    [[nodiscard]] float nextFloat01() noexcept {
        constexpr float UINT32_RANGE = 4294967296.0f;  // 2^32
        return static_cast<float>(nextUInt32()) / UINT32_RANGE;
    }

    /**
     * @brief Prochain flottant dans [min, max].
     * @param min Borne inférieure.
     * @param max Borne supérieure ; si égale à `min`, renvoie exactement `min` (aucun tirage
     *            discriminant, utile pour une durée de vie ou une vitesse fixe).
     * @return La valeur tirée.
     */
    [[nodiscard]] float nextRange(float min, float max) noexcept {
        return min + nextFloat01() * (max - min);
    }

private:
    std::uint64_t _state;
};

}  // namespace core
