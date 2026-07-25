/**
 * @file test_room_grid.cpp
 * @brief Tests unitaires de la partition d'un niveau en salles (`RoomGrid`, LOT-32).
 */

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "HMI/Graphics/RoomGrid.h"

/**
 * @brief Un niveau plus petit qu'une salle produit une seule salle couvrant le niveau entier.
 * \castest{<b>Un niveau plus petit qu'une salle produit une seule salle couvrant le niveau
 * entier.</b><br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un niveau plus petit que la taille de salle sur les
 * deux axes.<br/>2. Vérifier `columns()`/`rows()` et le rectangle de l'unique salle.<br/>
 * \tattendu Une seule salle (`columns() == rows() == 1`), dont le rectangle couvre exactement le
 * niveau entier — condition de non-régression exploitée par le cadrage caméra (`LOT-16`).
 * }
 */
TEST(RoomGridTest, NiveauPlusPetitQuUneSalleProduitUneSeuleSalle) {
    const hmi::RoomGrid grid(10, 6);

    EXPECT_EQ(grid.columns(), 1);
    EXPECT_EQ(grid.rows(), 1);

    const hmi::RoomBounds bounds = grid.roomBounds(core::GridPosition{0, 0});
    EXPECT_EQ(bounds.column, 0);
    EXPECT_EQ(bounds.row, 0);
    EXPECT_EQ(bounds.width, 10);
    EXPECT_EQ(bounds.height, 6);
}

/**
 * @brief Un niveau exactement multiple de la taille de salle ne rogne aucune salle.
 * \castest{<b>Un niveau exactement multiple de la taille de salle ne rogne aucune salle.</b><br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un niveau dont chaque dimension est un multiple exact
 * de la taille de salle.<br/>2. Vérifier `columns()`/`rows()` et le rectangle de chaque
 * salle.<br/>
 * \tattendu Le nombre de salles correspond exactement au ratio ; chaque salle a la taille pleine
 * (aucun rectangle rogné), y compris la dernière colonne/ligne.
 * }
 */
TEST(RoomGridTest, NiveauMultipleExactNeRogneAucuneSalle) {
    const int width = hmi::RoomGrid::ROOM_WIDTH_TILES * 3;
    const int height = hmi::RoomGrid::ROOM_HEIGHT_TILES * 2;
    const hmi::RoomGrid grid(width, height);

    EXPECT_EQ(grid.columns(), 3);
    EXPECT_EQ(grid.rows(), 2);

    const hmi::RoomBounds last = grid.roomBounds(core::GridPosition{2, 1});
    EXPECT_EQ(last.column, hmi::RoomGrid::ROOM_WIDTH_TILES * 2);
    EXPECT_EQ(last.row, hmi::RoomGrid::ROOM_HEIGHT_TILES * 1);
    EXPECT_EQ(last.width, hmi::RoomGrid::ROOM_WIDTH_TILES);
    EXPECT_EQ(last.height, hmi::RoomGrid::ROOM_HEIGHT_TILES);
}

/**
 * @brief Un niveau non multiple de la taille de salle rogne la dernière colonne/ligne.
 * \castest{<b>Un niveau non multiple de la taille de salle rogne la dernière colonne/ligne.</b>
 * <br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un niveau dépassant de quelques cases un multiple de
 * la taille de salle.<br/>2. Vérifier le rectangle de la dernière salle sur chaque axe.<br/>
 * \tattendu La dernière colonne/ligne de salles est rognée à la taille réellement restante,
 * jamais agrandie au-delà des bornes du niveau.
 * }
 */
TEST(RoomGridTest, NiveauNonMultipleRogneLaDerniereSalle) {
    const int extraWidth = 5;
    const int extraHeight = 3;
    const int width = hmi::RoomGrid::ROOM_WIDTH_TILES + extraWidth;
    const int height = hmi::RoomGrid::ROOM_HEIGHT_TILES + extraHeight;
    const hmi::RoomGrid grid(width, height);

    EXPECT_EQ(grid.columns(), 2);
    EXPECT_EQ(grid.rows(), 2);

    const hmi::RoomBounds last = grid.roomBounds(core::GridPosition{1, 1});
    EXPECT_EQ(last.column, hmi::RoomGrid::ROOM_WIDTH_TILES);
    EXPECT_EQ(last.row, hmi::RoomGrid::ROOM_HEIGHT_TILES);
    EXPECT_EQ(last.width, extraWidth);
    EXPECT_EQ(last.height, extraHeight);
}

/**
 * @brief `roomIndexAt` renvoie l'indice de salle correct aux quatre coins et au centre.
 * \castest{<b>`roomIndexAt` renvoie l'indice de salle correct aux quatre coins et au centre.</b>
 * <br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` de 2x2 salles.<br/>2. Interroger `roomIndexAt` à des
 * positions choisies dans chacune des quatre salles.<br/>
 * \tattendu Chaque position renvoie l'indice de la salle qui la contient réellement.
 * }
 */
TEST(RoomGridTest, RoomIndexAtQuatreCoins) {
    const int width = hmi::RoomGrid::ROOM_WIDTH_TILES * 2;
    const int height = hmi::RoomGrid::ROOM_HEIGHT_TILES * 2;
    const hmi::RoomGrid grid(width, height);

    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{0, 0}), (core::GridPosition{0, 0}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{width - 1, 0}), (core::GridPosition{1, 0}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{0, height - 1}), (core::GridPosition{0, 1}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{width - 1, height - 1}),
              (core::GridPosition{1, 1}));
}

/**
 * @brief Une position hors des bornes du niveau est bornée à la salle la plus proche.
 * \castest{<b>Une position hors des bornes du niveau est bornée à la salle la plus proche.</b>
 * <br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un petit niveau.<br/>2. Interroger `roomIndexAt` avec
 * des coordonnées négatives puis très supérieures aux bornes.<br/>
 * \tattendu Aucun comportement indéfini : l'indice renvoyé reste dans `[0, columns())` ×
 * `[0, rows())`, borné au bord correspondant du niveau.
 * }
 */
TEST(RoomGridTest, PositionHorsBornesEstBornee) {
    const hmi::RoomGrid grid(10, 6);

    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{-100, -100}), (core::GridPosition{0, 0}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{1000, 1000}), (core::GridPosition{0, 0}));
}
