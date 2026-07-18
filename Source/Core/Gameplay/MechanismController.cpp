#include "Core/Gameplay/MechanismController.h"

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

namespace core {
namespace {

// La boîte recouvre-t-elle la case (col, row) ? Chevauchement d'aire strictement positive.
bool overlapsCell(const Aabb& box, GridPosition cell) {
    const float left = static_cast<float>(cell.column);
    const float top = static_cast<float>(cell.row);
    return box.min.x < left + 1.0f && box.max.x > left && box.min.y < top + 1.0f && box.max.y > top;
}

}  // namespace

MechanismController::MechanismController(const Level& level)
    : _collision(level.tileMap()),  // copie de la carte : on la mutera pour les portes
      _mechanisms(level.mechanisms()),
      _switchOn(level.mechanisms().size(), false),
      _playerOnSwitchPrev(level.mechanisms().size(), false) {
    // Portes fermées au départ : solides dans la grille de collision.
    for (const Mechanism& mechanism : _mechanisms) {
        _collision.setTile(mechanism.doorPosition.column, mechanism.doorPosition.row,
                           TileType::Solid);
    }
}

void MechanismController::update(const Aabb& playerBox) {
    for (std::size_t index = 0; index < _mechanisms.size(); ++index) {
        const Mechanism& mechanism = _mechanisms[index];
        const bool onSwitch = overlapsCell(playerBox, mechanism.switchPosition);

        // Bascule au FRONT : première frame de contact seulement (rester dessus ne re-bascule pas).
        if (onSwitch && !_playerOnSwitchPrev[index]) {
            _switchOn[index] = !_switchOn[index];
            // Porte ouverte → franchissable (Door) ; fermée → solide.
            const GridPosition door = mechanism.doorPosition;
            _collision.setTile(door.column, door.row,
                               _switchOn[index] ? TileType::Door : TileType::Solid);
        }
        _playerOnSwitchPrev[index] = onSwitch;
    }
}

}  // namespace core
