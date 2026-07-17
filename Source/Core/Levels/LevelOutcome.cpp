#include "Core/Levels/LevelOutcome.h"

#include <algorithm>  // std::clamp
#include <cmath>      // std::floor

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

namespace core {
namespace {

// La boîte recouvre-t-elle la case (col, row) ? Chevauchement d'aire strictement positive :
// se contenter d'effleurer un bord ne compte pas (évite les faux positifs aux frontières).
bool overlapsCell(const Aabb& box, int col, int row) {
    const float left = static_cast<float>(col);
    const float top = static_cast<float>(row);
    return box.min.x < left + 1.0f && box.max.x > left && box.min.y < top + 1.0f && box.max.y > top;
}

}  // namespace

LevelOutcome evaluateOutcome(const Aabb& playerBox, const Level& level) {
    const TileMap& map = level.tileMap();

    // Échec prioritaire n°1 : le personnage est passé sous la limite basse du niveau (chute).
    if (playerBox.min.y >= static_cast<float>(map.height())) {
        return LevelOutcome::Lost;
    }

    // Échec prioritaire n°2 : contact avec une tuile Danger. On ne teste que les cases recouvertes.
    const int colBegin =
        std::clamp(static_cast<int>(std::floor(playerBox.min.x)), 0, map.width() - 1);
    const int colEnd =
        std::clamp(static_cast<int>(std::floor(playerBox.max.x)), 0, map.width() - 1);
    const int rowBegin =
        std::clamp(static_cast<int>(std::floor(playerBox.min.y)), 0, map.height() - 1);
    const int rowEnd =
        std::clamp(static_cast<int>(std::floor(playerBox.max.y)), 0, map.height() - 1);
    for (int row = rowBegin; row <= rowEnd; ++row) {
        for (int col = colBegin; col <= colEnd; ++col) {
            if (map.tile(col, row) == TileType::Danger && overlapsCell(playerBox, col, row)) {
                return LevelOutcome::Lost;
            }
        }
    }

    // Succès : la boîte recouvre la case de sortie.
    const GridPosition exit = level.exit();
    if (overlapsCell(playerBox, exit.column, exit.row)) {
        return LevelOutcome::Won;
    }

    return LevelOutcome::Playing;
}

}  // namespace core
