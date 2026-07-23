#pragma once

#include <cstddef>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/Gameplay/BlockController.h
 * @brief Comportement des blocs poussables : position, poussée et chute (`EX-GP-022`).
 */

namespace core {

struct Aabb;

/**
 * @brief Fait vivre les **blocs poussables** d'un niveau (`TileType::Block`, `EX-GP-022`).
 *
 * Logique **pure** (aucun rendu ni fenêtre), sur le modèle de `MechanismController` : chaque bloc
 * occupe exactement **une case**, sans position continue — pousser ou tomber le déplace d'une
 * case entière, jamais à mi-chemin. Deux comportements, résolus chaque pas fixe :
 * - **Poussée** : si la boîte du personnage touche un bloc du côté vers lequel il se déplace et
 *   que la case suivante dans cette direction est libre (ni solide, ni un autre bloc), le bloc
 *   avance d'une case.
 * - **Chute** : un bloc dont la case du dessous est libre tombe d'une case, au rythme de
 *   `FALL_INTERVAL_STEPS` pas fixes par case (chute discrète plutôt que continue — cohérent avec
 *   le reste des mécanismes de ce moteur, tous résolus case par case).
 *
 * Le contrôleur ne connaît pas les mécanismes interrupteur/porte : `collisionMap()` complète une
 * grille de collision **déjà résolue** par `MechanismController` (portes ouvertes/fermées) avec
 * la position **courante** des blocs, sans jamais modifier la carte statique du `Level`.
 */
class BlockController {
public:
    /// Nombre de pas fixes qu'un bloc non soutenu met à tomber d'une case (chute discrète).
    static constexpr int FALL_INTERVAL_STEPS = 6;

    /// Construit le contrôleur : repère chaque tuile `Block` du niveau comme un bloc mobile.
    explicit BlockController(const Level& level);

    /**
     * @brief Met à jour les blocs pour un pas : tente une poussée puis fait avancer les chutes.
     * @param playerBox     Boîte englobante du personnage (position **avant** le pas physique
     *                      courant : la poussée doit libérer la case avant que la physique ne
     *                      résolve le déplacement du personnage sur ce même pas).
     * @param moveIntentX   Intention de déplacement horizontal du personnage (`PlayerInput::moveX`,
     *                      dans `[-1, 1]`) ; `0` ne pousse aucun bloc.
     * @param baseCollision Grille de collision déjà résolue par les mécanismes (portes), utilisée
     *                      pour savoir si la case visée par une poussée ou une chute est libre.
     */
    void update(const Aabb& playerBox, float moveIntentX, const TileMap& baseCollision);

    /// @return Une copie de @p base où chaque case occupée par un bloc est rendue solide.
    [[nodiscard]] TileMap collisionMap(const TileMap& base) const;

    /// @return Les positions courantes des blocs (pour le rendu : une entité-tuile par bloc).
    [[nodiscard]] const std::vector<GridPosition>& positions() const noexcept {
        return _positions;
    }

private:
    /// @return true si @p target est dans les bornes, non solide dans @p base, et non occupée par
    ///         un bloc autre que celui d'indice @p excluding.
    [[nodiscard]] bool isFree(GridPosition target, const TileMap& base,
                              std::size_t excluding) const;

    std::vector<GridPosition> _positions;
    std::vector<int> _fallTimers;  ///< Pas écoulés depuis que ce bloc est non soutenu.
};

}  // namespace core
