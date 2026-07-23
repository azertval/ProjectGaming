#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/Levels/Level.h
 * @brief Niveau chargé : grille de tuiles, entrée/sortie et mécanismes.
 */

namespace core {

/**
 * @brief Liaison d'un **interrupteur** à une **porte**, par positions résolues.
 *
 * Dans le fichier, la liaison est exprimée par identifiant (`switch.id` ↔ `door.opensWith`) ;
 * le chargeur la résout en positions de grille. Le **comportement** (l'interrupteur ou la plaque
 * de pression ouvre la porte) est résolu chaque pas fixe par `core::MechanismController`.
 */
struct Mechanism {
    GridPosition switchPosition;
    GridPosition doorPosition;
};

/**
 * @brief Niveau complet en mémoire : nom, grille de tuiles, entrée/sortie et mécanismes.
 *
 * Assemblé par le chargeur (après parsing et validation) puis lu par le rendu et, à terme, le
 * gameplay. Donnée pure (`EX-ARCH-011`, `EX-LVL-002`) : aucune dépendance rendu ni fichier.
 */
class Level {
public:
    /**
     * @brief Construit un niveau à partir de ses composantes.
     * @param name       Nom du niveau.
     * @param tileMap    Grille de tuiles typées (déplacée).
     * @param entry      Position d'apparition (case `Entry`).
     * @param exit       Position de sortie (case `Exit`).
     * @param mechanisms Liaisons interrupteur↔porte résolues.
     * @param jumpBudget Budget de sauts du tableau (`EX-GP-024`) ; -1 = illimité.
     * @param dashBudget Budget de dashs du tableau (`EX-GP-024`) ; -1 = illimité.
     */
    Level(std::string name, TileMap tileMap, GridPosition entry, GridPosition exit,
          std::vector<Mechanism> mechanisms, int jumpBudget = -1, int dashBudget = -1)
        : _name(std::move(name)),
          _tileMap(std::move(tileMap)),
          _entry(entry),
          _exit(exit),
          _mechanisms(std::move(mechanisms)),
          _jumpBudget(jumpBudget),
          _dashBudget(dashBudget) {}

    /// @return Le nom du niveau.
    [[nodiscard]] const std::string& name() const noexcept {
        return _name;
    }

    /// @return La grille de tuiles du niveau.
    [[nodiscard]] const TileMap& tileMap() const noexcept {
        return _tileMap;
    }

    /// @return La position d'apparition.
    [[nodiscard]] GridPosition entry() const noexcept {
        return _entry;
    }

    /// @return La position de sortie.
    [[nodiscard]] GridPosition exit() const noexcept {
        return _exit;
    }

    /// @return Les liaisons de mécanismes du niveau.
    [[nodiscard]] const std::vector<Mechanism>& mechanisms() const noexcept {
        return _mechanisms;
    }

    /// @return Budget de **sauts** du tableau (`EX-GP-024`) ; **-1 = illimité**.
    [[nodiscard]] int jumpBudget() const noexcept {
        return _jumpBudget;
    }

    /// @return Budget de **dashs** du tableau (`EX-GP-024`) ; **-1 = illimité**.
    [[nodiscard]] int dashBudget() const noexcept {
        return _dashBudget;
    }

private:
    std::string _name;
    TileMap _tileMap;
    GridPosition _entry;
    GridPosition _exit;
    std::vector<Mechanism> _mechanisms;
    int _jumpBudget = -1;
    int _dashBudget = -1;
};

}  // namespace core
