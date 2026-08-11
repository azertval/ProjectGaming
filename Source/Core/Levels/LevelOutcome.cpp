#include "Core/Levels/LevelOutcome.h"

#include <algorithm>  // std::clamp
#include <cmath>      // std::floor

#include "Core/Levels/DangerGeometry.h"
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
    const auto left = static_cast<float>(col);
    const auto top = static_cast<float>(row);
    return box.min.x < left + 1.0F && box.max.x > left && box.min.y < top + 1.0F && box.max.y > top;
}

// La boîte recouvre-t-elle @p other ? Généralisation de overlapsCell à un rectangle arbitraire
// (bande directionnelle de core::dangerHitbox, boîte fournie par extraDangerBoxes).
bool overlapsBox(const Aabb& box, const Aabb& other) {
    return box.min.x < other.max.x && box.max.x > other.min.x && box.min.y < other.max.y &&
           box.max.y > other.min.y;
}

// Danger dont la mortalité est purement géométrique, résolue depuis la grille statique du niveau
// (pas d'état à faire vivre) : le danger classique et les quatre variantes directionnelles
// (EX-GP-050). DangerMover/DangerSwitched/DangerBlink ont un état (position/activation) porté par
// des contrôleurs de Core/Gameplay, hors de portée de Core/Levels — cf. extraDangerBoxes.
bool isStaticDanger(TileType type) {
    return type == TileType::Danger || type == TileType::DangerUp || type == TileType::DangerDown ||
           type == TileType::DangerLeft || type == TileType::DangerRight;
}

}  // namespace

LevelOutcome evaluateOutcome(const Aabb& playerBox, const Level& level,
                             const std::vector<Aabb>& extraDangerBoxes) {
    const TileMap& map = level.tileMap();

    // Échec prioritaire n°1 : le personnage est passé sous la limite basse du niveau (chute).
    if (playerBox.min.y >= static_cast<float>(map.height())) {
        return LevelOutcome::Lost;
    }

    // Échec prioritaire n°2 : contact avec un danger statique. On ne teste que les cases
    // recouvertes, via la géométrie exacte de core::dangerHitbox (case pleine pour Danger, bande
    // étroite pour les variantes directionnelles, EX-GP-050).
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
            const TileType type = map.tile(col, row);
            if (isStaticDanger(type) && overlapsBox(playerBox, dangerHitbox(type, col, row))) {
                return LevelOutcome::Lost;
            }
        }
    }

    // Échec prioritaire n°3 : contact avec un danger à état actuellement actif (mobile/commuté/
    // temporisé), assemblé par l'appelant (EX-GP-051/052/053).
    for (const Aabb& dangerBox : extraDangerBoxes) {
        if (overlapsBox(playerBox, dangerBox)) {
            return LevelOutcome::Lost;
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
