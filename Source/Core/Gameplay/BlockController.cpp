#include "Core/Gameplay/BlockController.h"

#include <cmath>
#include <string>

#include "Core/Gameplay/GameplayLog.h"
#include "Core/Physics/Aabb.h"

namespace core {
namespace {

// Tolérance de contact bloc/personnage : les deux bords doivent se toucher à peu près, pas se
// chevaucher franchement (sinon une simple superposition passagère pousserait le bloc).
constexpr float PUSH_TOUCH_TOLERANCE = 0.05f;

// Boîte occupée par un bloc à sa position courante (une case = une unité monde, EX-ARCH-021).
[[nodiscard]] Aabb blockBox(GridPosition position) {
    const Vector2 topLeft{static_cast<float>(position.column), static_cast<float>(position.row)};
    return Aabb::fromTopLeftSize(topLeft, Vector2{1.0f, 1.0f});
}

// Chevauchement vertical entre deux boîtes (aire strictement positive).
[[nodiscard]] bool overlapsVertically(const Aabb& a, const Aabb& b) {
    return a.min.y < b.max.y && a.max.y > b.min.y;
}

}  // namespace

BlockController::BlockController(const Level& level) {
    const TileMap& map = level.tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            if (map.tile(column, row) == TileType::Block) {
                _positions.push_back(GridPosition{column, row});
            }
        }
    }
    _fallTimers.assign(_positions.size(), 0);
}

bool BlockController::isFree(GridPosition target, const TileMap& base,
                             std::size_t excluding) const {
    if (!base.inBounds(target.column, target.row)) {
        return false;
    }
    // `base` porte la position D'ORIGINE de chaque bloc (`TileType::Block`, jamais modifiée) :
    // ignorée ici, seule `_positions` (courante) fait autorité sur l'occupation par un bloc —
    // sinon une case quittée par un bloc resterait perçue comme solide indéfiniment.
    const TileType tile = base.tile(target.column, target.row);
    if (tile != TileType::Block && core::isSolid(tile)) {
        return false;
    }
    for (std::size_t index = 0; index < _positions.size(); ++index) {
        if (index != excluding && _positions[index] == target) {
            return false;
        }
    }
    return true;
}

void BlockController::update(const Aabb& playerBox, float moveIntentX, const TileMap& base) {
    // 1. Poussée : direction du déplacement voulu, bloc touché de ce côté, case suivante libre.
    if (moveIntentX != 0.0f) {
        const int direction = moveIntentX > 0.0f ? 1 : -1;
        for (std::size_t index = 0; index < _positions.size(); ++index) {
            const GridPosition current = _positions[index];
            const Aabb box = blockBox(current);
            if (!overlapsVertically(playerBox, box)) {
                continue;
            }
            const bool touchesFromLeft =
                direction > 0 && std::fabs(playerBox.max.x - box.min.x) <= PUSH_TOUCH_TOLERANCE;
            const bool touchesFromRight =
                direction < 0 && std::fabs(playerBox.min.x - box.max.x) <= PUSH_TOUCH_TOLERANCE;
            if (!touchesFromLeft && !touchesFromRight) {
                continue;
            }
            const GridPosition target{current.column + direction, current.row};
            if (isFree(target, base, index)) {
                _positions[index] = target;
                _fallTimers[index] = 0;  // repart de zéro : re-teste la chute dès le prochain pas
                GAMEPLAY_LOG_TRACE("Bloc #" + std::to_string(index) + " pousse vers (" +
                                   std::to_string(target.column) + ", " +
                                   std::to_string(target.row) + ")");
            }
        }
    }

    // 2. Chute : un bloc non soutenu tombe d'une case au bout de FALL_INTERVAL_STEPS pas.
    for (std::size_t index = 0; index < _positions.size(); ++index) {
        const GridPosition current = _positions[index];
        const GridPosition below{current.column, current.row + 1};
        if (!isFree(below, base, index)) {
            _fallTimers[index] = 0;
            continue;
        }
        if (++_fallTimers[index] >= FALL_INTERVAL_STEPS) {
            _positions[index] = below;
            _fallTimers[index] = 0;
            GAMEPLAY_LOG_TRACE("Bloc #" + std::to_string(index) + " tombe en (" +
                               std::to_string(below.column) + ", " + std::to_string(below.row) +
                               ")");
        }
    }
}

TileMap BlockController::collisionMap(const TileMap& base) const {
    TileMap combined = base;
    // `base` porte encore la position D'ORIGINE de chaque bloc (`TileType::Block`, jamais modifiée
    // par le chargeur ni par les mécanismes) : l'effacer avant de reposer les blocs à leur position
    // COURANTE, sinon une case quittée resterait solide indéfiniment (mur fantôme).
    for (int row = 0; row < combined.height(); ++row) {
        for (int column = 0; column < combined.width(); ++column) {
            if (combined.tile(column, row) == TileType::Block) {
                combined.setTile(column, row, TileType::Empty);
            }
        }
    }
    for (const GridPosition& position : _positions) {
        combined.setTile(position.column, position.row, TileType::Solid);
    }
    return combined;
}

}  // namespace core
