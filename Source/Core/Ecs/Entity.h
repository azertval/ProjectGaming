#pragma once

#include <cstdint>

/**
 * @file Core/Ecs/Entity.h
 * @brief Identifiant d'entité de l'ECS : handle générationnel (index + génération).
 */

namespace core {

/**
 * @brief Handle générationnel identifiant une entité de l'ECS.
 *
 * Une entité n'est qu'un identifiant : elle ne porte aucune donnée (les données
 * vivent dans les composants). Le handle combine un **index** (position dans les
 * tableaux internes) et une **génération** : quand un index est recyclé après
 * destruction, sa génération est incrémentée, ce qui invalide les anciens handles
 * pointant sur le même index (`EX-ARCH-010`). Type léger, copiable et comparable.
 */
struct Entity {
    /// Type des index d'entité.
    using Index = std::uint32_t;
    /// Type des générations.
    using Generation = std::uint32_t;

    /// Valeur d'index réservée pour désigner l'entité invalide.
    static constexpr Index kInvalidIndex = ~Index{0};

    Index index = kInvalidIndex;
    Generation generation = 0;

    /// @return `true` si les deux handles désignent la même entité (index et génération).
    [[nodiscard]] friend bool operator==(const Entity& lhs, const Entity& rhs) {
        return lhs.index == rhs.index && lhs.generation == rhs.generation;
    }

    [[nodiscard]] friend bool operator!=(const Entity& lhs, const Entity& rhs) {
        return !(lhs == rhs);
    }
};

/// Entité invalide conventionnelle : ne désigne jamais une entité vivante.
inline constexpr Entity kInvalidEntity{Entity::kInvalidIndex, 0};

}  // namespace core
