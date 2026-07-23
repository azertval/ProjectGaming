#include "Core/Gameplay/MechanismController.h"

#include <string>

#include "Core/Gameplay/GameplayLog.h"
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

// Seuil de poids (masse, `core::Player::mass`) requis pour activer une plaque de pression
// (`EX-GP-025`) — calé sur la masse par défaut du personnage, pour qu'une plaque fonctionne
// « prête à l'emploi » sans configuration tant qu'aucune autre masse n'existe dans le jeu.
constexpr float MIN_TRIGGER_MASS = 1.0f;

}  // namespace

MechanismController::MechanismController(const Level& level)
    : _collision(level.tileMap()),  // copie de la carte : on la mutera pour les portes
      _mechanisms(level.mechanisms()),
      _switchOn(level.mechanisms().size(), false),
      _playerOnSwitchPrev(level.mechanisms().size(), false) {
    // Nature de chaque déclencheur, figée d'après la tuile d'origine (avant toute mutation de
    // _collision ci-dessous — seules les portes sont réécrites, jamais les déclencheurs eux-mêmes).
    _continuous.reserve(_mechanisms.size());
    for (const Mechanism& mechanism : _mechanisms) {
        _continuous.push_back(_collision.tile(mechanism.switchPosition.column,
                                              mechanism.switchPosition.row) ==
                              TileType::PressurePlate);
    }
    // Portes fermées au départ : solides dans la grille de collision.
    for (const Mechanism& mechanism : _mechanisms) {
        _collision.setTile(mechanism.doorPosition.column, mechanism.doorPosition.row,
                           TileType::Solid);
    }
}

void MechanismController::update(const Aabb& playerBox, float playerMass) {
    for (std::size_t index = 0; index < _mechanisms.size(); ++index) {
        const Mechanism& mechanism = _mechanisms[index];
        const bool onSwitch = overlapsCell(playerBox, mechanism.switchPosition);
        const GridPosition door = mechanism.doorPosition;

        if (_continuous[index]) {
            // Plaque de pression (EX-GP-025) : ouverte tant qu'un poids suffisant y repose,
            // refermee des qu'il en repart — activation CONTINUE, reevaluee a chaque pas (pas de
            // front). Le poids n'intervient QUE pour ce type de declencheur.
            const bool shouldBeOpen = onSwitch && playerMass >= MIN_TRIGGER_MASS;
            if (shouldBeOpen != _switchOn[index]) {
                _switchOn[index] = shouldBeOpen;
                _collision.setTile(door.column, door.row,
                                   _switchOn[index] ? TileType::Door : TileType::Solid);
                GAMEPLAY_LOG_TRACE("Plaque de pression #" + std::to_string(index) + " -> porte (" +
                                   std::to_string(door.column) + ", " + std::to_string(door.row) +
                                   ") " + (_switchOn[index] ? "ouverte" : "fermee"));
            }
        } else {
            // Interrupteur a bascule (EX-GP-020) : bascule au FRONT seulement, comportement
            // inchange (rester dessus ne re-bascule pas ; le poids n'intervient pas).
            if (onSwitch && !_playerOnSwitchPrev[index]) {
                _switchOn[index] = !_switchOn[index];
                _collision.setTile(door.column, door.row,
                                   _switchOn[index] ? TileType::Door : TileType::Solid);
                GAMEPLAY_LOG_TRACE("Interrupteur #" + std::to_string(index) + " -> porte (" +
                                   std::to_string(door.column) + ", " + std::to_string(door.row) +
                                   ") " + (_switchOn[index] ? "ouverte" : "fermee"));
            }
        }
        _playerOnSwitchPrev[index] = onSwitch;
    }
}

}  // namespace core
