#include "Core/Gameplay/BlockController.h"

#include <cmath>
#include <string>

#include "Core/Gameplay/GameplayLog.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/SlopeGeometry.h"

namespace core {
namespace {

// Tolérance de contact bloc/personnage : les deux bords doivent se toucher à peu près, pas se
// chevaucher franchement (sinon une simple superposition passagère pousserait le bloc).
constexpr float PUSH_TOUCH_TOLERANCE = 0.05F;

// true si `type` est l'une des trois tailles de bloc (EX-GP-005) ; false sinon.
[[nodiscard]] bool isBlockTile(TileType type) {
    return type == TileType::Block || type == TileType::BlockHalf || type == TileType::BlockQuarter;
}

// Boîte RÉELLE occupée par un bloc à sa position courante : pleine case si `scale == 1`, sinon
// centrée et réduite (`EX-GP-005`) — une case = une unité monde (`EX-ARCH-021`).
[[nodiscard]] Aabb blockBox(GridPosition position, float scale) {
    const float margin = (1.0F - scale) * 0.5F;
    const Vector2 topLeft{static_cast<float>(position.column) + margin,
                          static_cast<float>(position.row) + margin};
    return Aabb::fromTopLeftSize(topLeft, Vector2{scale, scale});
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
            const TileType tile = map.tile(column, row);
            if (isBlockTile(tile)) {
                _positions.push_back(GridPosition{.column = column, .row = row});
                _scales.push_back(tileVisualScale(tile));
            }
        }
    }
    _fallTimers.assign(_positions.size(), 0);
}

Aabb BlockController::boxAt(std::size_t index) const {
    return blockBox(_positions[index], _scales[index]);
}

bool BlockController::isFree(GridPosition target, const TileMap& base,
                             std::size_t excluding) const {
    if (!base.inBounds(target.column, target.row)) {
        return false;
    }
    // `base` porte la position D'ORIGINE de chaque bloc (`Block`/`BlockHalf`/`BlockQuarter`,
    // jamais modifiée) : ignorée ici, seule `_positions` (courante) fait autorité sur l'occupation
    // par un bloc — sinon une case quittée par un bloc resterait perçue comme solide indéfiniment.
    const TileType tile = base.tile(target.column, target.row);
    if (!isBlockTile(tile) && core::isSolid(tile)) {
        return false;
    }
    // Pentes/arrondis (sol ou plafond, EX-GP-003/EX-GP-004/EX-GP-006) : jamais solides pour la
    // grille classique (`core::isSolid`), mais `BlockController` n'a aucune notion de suivi de
    // surface — un bloc n'y "glisse" pas le long d'une pente comme le personnage. Sans cette
    // exclusion, une case de pente serait perçue "libre" et un bloc la traverserait en tombant ou
    // en se faisant pousser dedans. Traitée ici comme un obstacle simple (case par case, comme un
    // `Solid`), pas suivie.
    if (core::isFollowableSurface(tile) || core::isCeilingSlope(tile)) {
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
    if (moveIntentX != 0.0F) {
        const int direction = moveIntentX > 0.0F ? 1 : -1;
        for (std::size_t index = 0; index < _positions.size(); ++index) {
            const GridPosition current = _positions[index];
            const Aabb box = blockBox(current, _scales[index]);
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
            const GridPosition target{.column = current.column + direction, .row = current.row};
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
        const GridPosition below{.column = current.column, .row = current.row + 1};
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
    // `base` porte encore la position D'ORIGINE de chaque bloc (jamais modifiée par le chargeur ni
    // par les mécanismes) : l'effacer avant de reposer les blocs PLEINS à leur position COURANTE,
    // sinon une case quittée resterait solide indéfiniment (mur fantôme).
    for (int row = 0; row < combined.height(); ++row) {
        for (int column = 0; column < combined.width(); ++column) {
            if (isBlockTile(combined.tile(column, row))) {
                combined.setTile(column, row, TileType::Empty);
            }
        }
    }
    // Seuls les blocs PLEINS (scale == 1) sont rendus solides ici : un bloc réduit doit rester
    // franchissable AUTOUR de lui dans la grille classique (EX-GP-005) — sa collision réelle est
    // résolue à part par `core::sweepAabbVsAabb` (voir `boxAt`/`scales`, en-tête de la classe).
    for (std::size_t index = 0; index < _positions.size(); ++index) {
        if (_scales[index] >= 1.0F) {
            combined.setTile(_positions[index].column, _positions[index].row, TileType::Solid);
        }
    }
    return combined;
}

}  // namespace core
