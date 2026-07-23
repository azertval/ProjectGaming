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
 * les mécanismes, ce modèle ne fait que représenter sa position de **départ**. `SlopeUpRight`/
 * `SlopeUpLeft` (`EX-GP-003`) sont des **pentes** à 45° (montée sur toute la largeur d'une case,
 * respectivement vers la droite et vers la gauche) : leur surface est **inclinée**, décrite par
 * `core::slopeSurfaceHeight` (`Core/Physics/SlopeGeometry.h`) et suivie par une passe de
 * résolution dédiée (@ref guide-physique) — **pas** par `isSolid` (voir ci-dessous). `RoundedUpRight`/
 * `RoundedUpLeft` (`EX-GP-004`) sont la variante **courbe** (quart de cercle) des pentes : même
 * orientation, même infrastructure de suivi (`core::slopeSurfaceHeight`, non solides), seule la
 * formule de hauteur diffère (linéaire pour une pente, quart de cercle pour un arrondi).
 * `BlockHalf`/`BlockQuarter` (`EX-GP-005`) sont des **blocs poussables réduits** (facteurs `×0.5`/
 * `×0.25`, `core::BlockController`) : mêmes règles de poussée/chute que `Block` (case par case),
 * mais leur boîte de collision **réelle** (testée contre le personnage) est plus petite que la
 * case et **centrée** dedans — résolue par une routine dédiée boîte-contre-boîte
 * (`core::sweepAabbVsAabb`, @ref guide-physique), pas par la grille classique.
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
    SlopeUpRight,
    SlopeUpLeft,
    RoundedUpRight,
    RoundedUpLeft,
    BlockHalf,
    BlockQuarter,
};

/**
 * @brief Indique si un type de tuile bloque le déplacement de manière **statique**.
 * @param type Type de tuile.
 * @return true pour `Solid` et les trois tailles de bloc (`Block`/`BlockHalf`/`BlockQuarter`, non
 *         encore déplacés bloquent comme un mur — `core::BlockController` gère leur position
 *         réelle et, pour les tailles réduites, leur boîte de collision **plus petite que la
 *         case** via `core::sweepAabbVsAabb`, jamais via ce test statique). `false` pour les
 *         pentes et arrondis (`SlopeUpRight`/`SlopeUpLeft`/`RoundedUpRight`/`RoundedUpLeft`) : une
 *         surface suivable n'est **jamais** solide pour le balayage classique
 *         (`core::sweepAabb`), sous peine de transformer son bord haut en mur invisible — sa
 *         solidité est entièrement gérée par la passe de suivi de surface (voir
 *         `core::slopeSurfaceHeight`). La solidité d'une porte dépend de son **état**
 *         (ouverte/fermée) et la position d'un bloc évolue en jeu : les deux sont gérées par la
 *         simulation, pas par ce test statique.
 */
[[nodiscard]] constexpr bool isSolid(TileType type) noexcept {
    return type == TileType::Solid || type == TileType::Block || type == TileType::BlockHalf ||
           type == TileType::BlockQuarter;
}

}  // namespace core
