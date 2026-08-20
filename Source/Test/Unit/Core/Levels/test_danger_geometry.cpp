// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_danger_geometry.cpp
 * @brief Tests unitaires de la géométrie des dangers directionnels (`core::dangerHitbox`,
 * `EX-GP-050`).
 */

#include <gtest/gtest.h>

#include "Core/Levels/DangerGeometry.h"

/**
 * @brief `Danger` (classique) occupe toute la case.
 * \castest{<b>Danger classique occupe toute la case.</b><br/>
 * \tcat Unitaire · Danger Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Danger classique occupe toute la case.
 * }
 */
TEST(DangerGeometryTest, DangerClassiqueOccupeLaCaseEntiere) {
    const core::Aabb box = core::dangerHitbox(core::TileType::Danger, 2, 3);
    EXPECT_FLOAT_EQ(box.min.x, 2.0f);
    EXPECT_FLOAT_EQ(box.min.y, 3.0f);
    EXPECT_FLOAT_EQ(box.max.x, 3.0f);
    EXPECT_FLOAT_EQ(box.max.y, 4.0f);
}

/**
 * @brief `DangerUp` réduit sa zone mortelle à une bande le long du bord haut de la case
 * (`EX-GP-050`).
 * \castest{<b>DangerUp réduit sa zone mortelle au bord haut.</b><br/>
 * \tcat Unitaire · Danger Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu DangerUp réduit sa zone mortelle au bord haut.
 * }
 */
TEST(DangerGeometryTest, DangerUpBordeLeHaut) {
    const core::Aabb box = core::dangerHitbox(core::TileType::DangerUp, 0, 0);
    EXPECT_FLOAT_EQ(box.min.x, 0.0f);
    EXPECT_FLOAT_EQ(box.max.x, 1.0f);
    EXPECT_FLOAT_EQ(box.min.y, 0.0f);
    EXPECT_FLOAT_EQ(box.max.y, core::kDangerEdgeThickness);
}

/**
 * @brief `DangerDown` réduit sa zone mortelle à une bande le long du bord bas de la case
 * (`EX-GP-050`).
 * \castest{<b>DangerDown réduit sa zone mortelle au bord bas.</b><br/>
 * \tcat Unitaire · Danger Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu DangerDown réduit sa zone mortelle au bord bas.
 * }
 */
TEST(DangerGeometryTest, DangerDownBordeLeBas) {
    const core::Aabb box = core::dangerHitbox(core::TileType::DangerDown, 0, 0);
    EXPECT_FLOAT_EQ(box.min.x, 0.0f);
    EXPECT_FLOAT_EQ(box.max.x, 1.0f);
    EXPECT_FLOAT_EQ(box.min.y, 1.0f - core::kDangerEdgeThickness);
    EXPECT_FLOAT_EQ(box.max.y, 1.0f);
}

/**
 * @brief `DangerLeft` réduit sa zone mortelle à une bande le long du bord gauche de la case
 * (`EX-GP-050`).
 * \castest{<b>DangerLeft réduit sa zone mortelle au bord gauche.</b><br/>
 * \tcat Unitaire · Danger Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu DangerLeft réduit sa zone mortelle au bord gauche.
 * }
 */
TEST(DangerGeometryTest, DangerLeftBordeLaGauche) {
    const core::Aabb box = core::dangerHitbox(core::TileType::DangerLeft, 0, 0);
    EXPECT_FLOAT_EQ(box.min.y, 0.0f);
    EXPECT_FLOAT_EQ(box.max.y, 1.0f);
    EXPECT_FLOAT_EQ(box.min.x, 0.0f);
    EXPECT_FLOAT_EQ(box.max.x, core::kDangerEdgeThickness);
}

/**
 * @brief `DangerRight` réduit sa zone mortelle à une bande le long du bord droit de la case
 * (`EX-GP-050`).
 * \castest{<b>DangerRight réduit sa zone mortelle au bord droit.</b><br/>
 * \tcat Unitaire · Danger Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu DangerRight réduit sa zone mortelle au bord droit.
 * }
 */
TEST(DangerGeometryTest, DangerRightBordeLaDroite) {
    const core::Aabb box = core::dangerHitbox(core::TileType::DangerRight, 0, 0);
    EXPECT_FLOAT_EQ(box.min.y, 0.0f);
    EXPECT_FLOAT_EQ(box.max.y, 1.0f);
    EXPECT_FLOAT_EQ(box.min.x, 1.0f - core::kDangerEdgeThickness);
    EXPECT_FLOAT_EQ(box.max.x, 1.0f);
}

/**
 * @brief `DangerMover`/`DangerSwitched`/`DangerBlink` occupent toute la case : leur mortalité est
 * temporelle (position/activation), pas géométrique (`EX-GP-051`/`EX-GP-052`/`EX-GP-053`).
 * \castest{<b>Les dangers temporels occupent toute la case.</b><br/>
 * \tcat Unitaire · Danger Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les dangers temporels occupent toute la case.
 * }
 */
TEST(DangerGeometryTest, DangersTemporelsOccupentLaCaseEntiere) {
    for (const core::TileType type : {core::TileType::DangerMover, core::TileType::DangerSwitched,
                                      core::TileType::DangerBlink}) {
        const core::Aabb box = core::dangerHitbox(type, 1, 2);
        EXPECT_FLOAT_EQ(box.min.x, 1.0f);
        EXPECT_FLOAT_EQ(box.min.y, 2.0f);
        EXPECT_FLOAT_EQ(box.max.x, 2.0f);
        EXPECT_FLOAT_EQ(box.max.y, 3.0f);
    }
}
