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
 * provoque l'échec au contact ; `Entry`/`Exit` sont l'apparition et la sortie ; `Switch`,
 * `PressurePlate` et `Door` sont les mécanismes de puzzle, résolus chaque pas fixe par
 * `core::MechanismController` (ce modèle ne fait que les représenter) : `Switch` bascule au
 * contact (front), `PressurePlate` (`EX-GP-025`) reste active tant qu'un poids suffisant y
 * repose — les deux partagent la même infrastructure de liaison à une `Door`. `Block` (`EX-GP-022`)
 * est un **bloc poussable** : sa position initiale est celle du fichier, mais `core::BlockController`
 * la fait évoluer chaque pas fixe (poussée par le personnage, chute si non soutenu) — comme pour
 * les mécanismes, ce modèle ne fait que représenter sa position de **départ**.
 */
enum class TileType {
    Empty,
    Solid,
    Danger,
    Entry,
    Exit,
    Switch,
    Door,
    PressurePlate,
    Block,
};

/**
 * @brief Indique si un type de tuile bloque le déplacement de manière **statique**.
 * @param type Type de tuile.
 * @return true pour `Solid` et `Block` (un bloc non encore déplacé bloque comme un mur). La
 *         solidité d'une porte dépend de son **état** (ouverte/fermée) et la position d'un bloc
 *         évolue en jeu : toutes deux sont gérées par la simulation, pas par ce test statique.
 */
[[nodiscard]] constexpr bool isSolid(TileType type) noexcept {
    return type == TileType::Solid || type == TileType::Block;
}

}  // namespace core
