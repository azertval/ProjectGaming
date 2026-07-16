#pragma once

/**
 * @file Core/Levels/TileType.h
 * @brief Types de tuiles d'un niveau et utilitaires associés.
 */

namespace core {

/**
 * @brief Type d'une tuile de la grille d'un niveau (`EX-GP-001`).
 *
 * `Empty` est la case traversable par défaut ; `Solid` bloque le déplacement ; `Danger`
 * provoque l'échec au contact ; `Entry`/`Exit` sont l'apparition et la sortie ; `Switch` et
 * `Door` sont les mécanismes de puzzle (leur **comportement** relève d'un lot ultérieur, ce
 * modèle ne fait que les représenter).
 */
enum class TileType {
    Empty,
    Solid,
    Danger,
    Entry,
    Exit,
    Switch,
    Door,
};

/**
 * @brief Indique si un type de tuile bloque le déplacement de manière **statique**.
 * @param type Type de tuile.
 * @return true pour `Solid`. La solidité d'une porte dépend de son **état** (ouverte/fermée) et
 *         sera gérée par la simulation ; elle n'est donc pas statiquement solide ici.
 */
[[nodiscard]] constexpr bool isSolid(TileType type) noexcept {
    return type == TileType::Solid;
}

}  // namespace core
