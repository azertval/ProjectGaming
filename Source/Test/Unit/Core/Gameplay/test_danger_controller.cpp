// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_danger_controller.cpp
 * @brief Tests unitaires des dangers mobile et temporisé (`core::DangerController`,
 * `EX-GP-051`/`EX-GP-053`).
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/DangerController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

namespace {

// Niveau minimal 6x6 avec un seul danger mobile en (1,1), aller-retour horizontal, portee 2.
core::Level makeLevelWithMover(core::DangerMoverAxis axis, int range) {
    core::TileMap map(6, 6);
    map.setTile(1, 1, core::TileType::DangerMover);
    std::vector<core::DangerMoverConfig> moverConfigs{
        core::DangerMoverConfig{core::GridPosition{1, 1}, axis, range}};
    return core::Level("mobile", std::move(map), core::GridPosition{0, 0}, core::GridPosition{5, 5},
                       {}, -1, -1, {}, std::move(moverConfigs));
}

// Niveau minimal 4x4 avec un seul danger temporise en (2,2).
core::Level makeLevelWithBlink(int period, int phase, int activeDuration) {
    core::TileMap map(4, 4);
    map.setTile(2, 2, core::TileType::DangerBlink);
    std::vector<core::DangerBlinkConfig> blinkConfigs{
        core::DangerBlinkConfig{core::GridPosition{2, 2}, period, phase, activeDuration}};
    return core::Level("temporise", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{3, 3}, {}, -1, -1, {}, {}, std::move(blinkConfigs));
}

}  // namespace

/**
 * @brief Au chargement (aucun pas fixe écoulé), un danger mobile est à sa position de départ.
 * \castest{<b>Au chargement, un danger mobile est à sa position de départ.</b><br/>
 * \tcat Unitaire · Danger Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Au chargement, un danger mobile est à sa position de départ.
 * }
 */
TEST(DangerControllerTest, MobileDemarreASaPositionDeDepart) {
    const core::DangerController controller(
        makeLevelWithMover(core::DangerMoverAxis::Horizontal, 2));
    ASSERT_EQ(controller.moverCount(), 1u);
    const core::Aabb box = controller.moverBox(0);
    EXPECT_FLOAT_EQ(box.min.x, 1.0f);
    EXPECT_FLOAT_EQ(box.min.y, 1.0f);
    EXPECT_FLOAT_EQ(box.max.x, 2.0f);
    EXPECT_FLOAT_EQ(box.max.y, 2.0f);
}

/**
 * @brief Un danger mobile horizontal progresse vers l'atteinte de sa portée, revient à sa
 * position de départ à la fin d'un cycle complet — aller-retour déterministe (`EX-GP-051`).
 * \castest{<b>Un danger mobile horizontal fait un aller-retour déterministe.</b><br/>
 * \tcat Unitaire · Danger Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger mobile horizontal fait un aller-retour déterministe.
 * }
 */
TEST(DangerControllerTest, MobileHorizontalAllerRetourDeterministe) {
    core::DangerController controller(makeLevelWithMover(core::DangerMoverAxis::Horizontal, 2));

    const auto advance = [&](int steps) {
        for (int i = 0; i < steps; ++i) {
            controller.update();
        }
    };

    advance(30);  // a mi-chemin de l'aller (vitesse de conception : 2 cases/s a 60 pas/s)
    EXPECT_NEAR(controller.moverBox(0).min.x, 2.0f, 1e-4f);

    advance(30);  // 60 pas au total : au bout de l'aller (portee atteinte)
    EXPECT_NEAR(controller.moverBox(0).min.x, 3.0f, 1e-4f);

    advance(30);  // 90 pas : a mi-chemin du retour
    EXPECT_NEAR(controller.moverBox(0).min.x, 2.0f, 1e-4f);

    advance(30);  // 120 pas : cycle complet, de retour au depart
    EXPECT_NEAR(controller.moverBox(0).min.x, 1.0f, 1e-4f);

    // La ligne ne bouge jamais sur un axe horizontal.
    EXPECT_FLOAT_EQ(controller.moverBox(0).min.y, 1.0f);
}

/**
 * @brief Un danger mobile vertical déplace sa ligne, jamais sa colonne (`EX-GP-051`).
 * \castest{<b>Un danger mobile vertical déplace sa ligne, jamais sa colonne.</b><br/>
 * \tcat Unitaire · Danger Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger mobile vertical déplace sa ligne, jamais sa colonne.
 * }
 */
TEST(DangerControllerTest, MobileVerticalDeplaceLaLigne) {
    core::DangerController controller(makeLevelWithMover(core::DangerMoverAxis::Vertical, 2));
    for (int i = 0; i < 60; ++i) {
        controller.update();
    }
    const core::Aabb box = controller.moverBox(0);
    EXPECT_FLOAT_EQ(box.min.x, 1.0f);     // colonne inchangee
    EXPECT_NEAR(box.min.y, 3.0f, 1e-4f);  // ligne au bout de l'aller (portee 2, depart 1)
}

/**
 * @brief Une portée nulle immobilise le danger mobile à sa position de départ.
 * \castest{<b>Une portée nulle immobilise le danger mobile.</b><br/>
 * \tcat Unitaire · Danger Controller<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une portée nulle immobilise le danger mobile.
 * }
 */
TEST(DangerControllerTest, PorteeNulleImmobiliseLeMobile) {
    core::DangerController controller(makeLevelWithMover(core::DangerMoverAxis::Horizontal, 0));
    for (int i = 0; i < 100; ++i) {
        controller.update();
    }
    const core::Aabb box = controller.moverBox(0);
    EXPECT_FLOAT_EQ(box.min.x, 1.0f);
    EXPECT_FLOAT_EQ(box.min.y, 1.0f);
}

/**
 * @brief Un danger temporisé alterne actif/inactif selon sa période et sa durée active
 * (`EX-GP-053`).
 * \castest{<b>Un danger temporisé alterne actif/inactif selon sa période.</b><br/>
 * \tcat Unitaire · Danger Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger temporisé alterne actif/inactif selon sa période.
 * }
 */
TEST(DangerControllerTest, TemporiseAlterneSelonSaPeriode) {
    core::DangerController controller(makeLevelWithBlink(/*period=*/4, /*phase=*/0,
                                                         /*activeDuration=*/2));
    const core::GridPosition position{2, 2};

    EXPECT_TRUE(controller.isBlinkActive(position));  // pas 0 : actif
    controller.update();
    EXPECT_TRUE(controller.isBlinkActive(position));  // pas 1 : actif
    controller.update();
    EXPECT_FALSE(controller.isBlinkActive(position));  // pas 2 : inactif
    controller.update();
    EXPECT_FALSE(controller.isBlinkActive(position));  // pas 3 : inactif
    controller.update();
    EXPECT_TRUE(controller.isBlinkActive(position));  // pas 4 : nouveau cycle, actif
}

/**
 * @brief Un déphasage décale le motif actif/inactif d'un danger temporisé (`EX-GP-053`).
 * \castest{<b>Un déphasage décale le motif actif/inactif.</b><br/>
 * \tcat Unitaire · Danger Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un déphasage décale le motif actif/inactif.
 * }
 */
TEST(DangerControllerTest, DephasageDecaleLeMotif) {
    core::DangerController controller(makeLevelWithBlink(/*period=*/4, /*phase=*/1,
                                                         /*activeDuration=*/2));
    const core::GridPosition position{2, 2};

    EXPECT_FALSE(controller.isBlinkActive(position));  // pas 0 : dephase, inactif
    controller.update();
    EXPECT_TRUE(controller.isBlinkActive(position));  // pas 1 : actif
}

/**
 * @brief `isBlinkActive` renvoie faux pour une position sans configuration connue (robustesse).
 * \castest{<b>isBlinkActive renvoie faux pour une position sans configuration.</b><br/>
 * \tcat Unitaire · Danger Controller<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu isBlinkActive renvoie faux pour une position sans configuration.
 * }
 */
TEST(DangerControllerTest, TemporiseFauxSansConfiguration) {
    const core::DangerController controller(makeLevelWithBlink(4, 0, 2));
    EXPECT_FALSE(controller.isBlinkActive(core::GridPosition{0, 0}));
}
