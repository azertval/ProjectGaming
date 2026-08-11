#include "HMI/Graphics/RoomGrid.h"

#include <algorithm>

namespace hmi {

namespace {
// Division entiere arrondie vers le haut (ceil), pour width/height > 0 et roomSize > 0.
int ceilDiv(int value, int roomSize) {
    return (value + roomSize - 1) / roomSize;
}
}  // namespace

// Construit la partition pour un niveau de levelWidth x levelHeight cases.
RoomGrid::RoomGrid(int levelWidth, int levelHeight)
    : _levelWidth(levelWidth),
      _levelHeight(levelHeight),
      _columns(ceilDiv(levelWidth, ROOM_WIDTH_TILES)),
      _rows(ceilDiv(levelHeight, ROOM_HEIGHT_TILES)) {}

// Indice (colonne, ligne) de la salle contenant tile, bornee aux salles existantes.
core::GridPosition RoomGrid::roomIndexAt(core::GridPosition tile) const noexcept {
    // Borne d'abord la case aux limites du niveau (jamais de comportement indefini pour une
    // position hors bornes) : la division entiere qui suit ne porte alors que sur des valeurs
    // non negatives, retombant toujours dans une salle existante.
    const int column = std::clamp(tile.column, 0, _levelWidth - 1);
    const int row = std::clamp(tile.row, 0, _levelHeight - 1);
    return core::GridPosition{.column = column / ROOM_WIDTH_TILES, .row = row / ROOM_HEIGHT_TILES};
}

// Rectangle (en cases) de la salle d'indice roomIndex, rogne aux bornes du niveau.
RoomBounds RoomGrid::roomBounds(core::GridPosition roomIndex) const noexcept {
    const int originColumn = roomIndex.column * ROOM_WIDTH_TILES;
    const int originRow = roomIndex.row * ROOM_HEIGHT_TILES;
    return RoomBounds{.column = originColumn,
                      .row = originRow,
                      .width = (std::min)(ROOM_WIDTH_TILES, _levelWidth - originColumn),
                      .height = (std::min)(ROOM_HEIGHT_TILES, _levelHeight - originRow)};
}

}  // namespace hmi
