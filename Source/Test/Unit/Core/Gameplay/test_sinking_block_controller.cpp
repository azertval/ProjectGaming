// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sinking_block_controller.cpp
 * @brief Tests unitaires des blocs descendants (`SinkingBlockController`, `EX-GP-027`).
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/SinkingBlockController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace {

// Niveau 6xH : sol plein sur la DERNIERE rangee, un bloc descendant en (2, 1). Entre les deux,
// du vide -- c'est la hauteur qui decide si le bloc a de quoi descendre.
core::Level makeLevel(int height) {
    core::TileMap map(6, height);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, height - 1, core::TileType::Solid);
    }
    map.setTile(2, 1, core::TileType::SinkingBlock);
    return core::Level("descendant", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{5, 1}, std::vector<core::Mechanism>{});
}

// Boite 1x1 posee sur la case (column, row).
core::Aabb boxAt(int column, int row) {
    return core::Aabb::fromTopLeftSize(
        core::Vector2{static_cast<float>(column), static_cast<float>(row)},
        core::Vector2{1.0f, 1.0f});
}

// Boite hors de portee de tout bloc du niveau : ne peut rien armer.
core::Aabb farAway() {
    return boxAt(5, 0);
}

}  // namespace

/**
 * @brief Tant que personne ne le touche, un bloc descendant ne bouge pas d'un pouce.
 * \castest{<b>Un bloc descendant non arme reste immobile indefiniment.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Poser un bloc descendant en (2, 1) avec du vide en dessous.<br/>2. Avancer 600 pas
 * fixes sans jamais approcher le personnage.<br/>
 * \tattendu Le bloc est toujours a sa position de depart, non arme, et emet encore son echantillon
 * -- sans quoi il ne serait pas solide et le personnage lui passerait au travers.}
 */
TEST(SinkingBlockControllerTest, ImmobileTantQuIlNEstPasTouche) {
    core::Level level = makeLevel(8);
    core::SinkingBlockController controller(level);
    ASSERT_EQ(controller.count(), 1u);

    for (int step = 0; step < 600; ++step) {
        controller.update(farAway(), level.tileMap());
    }

    EXPECT_FALSE(controller.isArmedAt(0));
    EXPECT_FLOAT_EQ(controller.boxAt(0).min.y, 1.0f);
    EXPECT_EQ(controller.samples().size(), 1u) << "un bloc non arme doit rester solide";
}

/**
 * @brief N'importe quelle face arme le bloc : dessus, cote ou dessous.
 * \castest{<b>Un contact par le dessous arme un bloc descendant.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Approcher la boite du personnage PAR LE DESSOUS du bloc (2, 2).<br/>2. Avancer un
 * pas.<br/>
 * \tattendu Le bloc est arme. L'exigence EX-GP-027 retient le contact quelconque, et non un test
 * de portage qui obligerait a definir un seuil de « vraiment pose dessus ».}
 */
TEST(SinkingBlockControllerTest, ArmeParLeDessous) {
    core::Level level = makeLevel(8);
    core::SinkingBlockController controller(level);

    controller.update(boxAt(2, 2), level.tileMap());

    EXPECT_TRUE(controller.isArmedAt(0));
}

/**
 * @brief L'armement est un aller simple : s'eloigner ne l'annule pas.
 * \castest{<b>Un bloc arme continue de descendre apres que le personnage s'est eloigne.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Toucher le bloc une seule fois.<br/>2. Eloigner le personnage et avancer 120
 * pas.<br/>
 * \tattendu Le bloc est descendu strictement plus bas que sa position de depart.}
 */
TEST(SinkingBlockControllerTest, ArmementIrreversible) {
    core::Level level = makeLevel(12);
    core::SinkingBlockController controller(level);

    controller.update(boxAt(2, 0), level.tileMap());  // contact par le dessus
    ASSERT_TRUE(controller.isArmedAt(0));
    for (int step = 0; step < 120; ++step) {
        controller.update(farAway(), level.tileMap());
    }

    EXPECT_GT(controller.boxAt(0).min.y, 1.0f);
}

/**
 * @brief La descente est fonction du seul numero de pas depuis l'armement.
 * \castest{<b>La descente d'un bloc est deterministe et a vitesse constante.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Armer deux controleurs identiques et les avancer du meme nombre de pas.<br/>2.
 * Comparer les positions ; verifier la distance parcourue en une seconde.<br/>
 * \tattendu Positions identiques au flottant pres, et distance parcourue en 60 pas egale a la
 * vitesse configuree (EX-NFR-002 : aucune accumulation).}
 */
TEST(SinkingBlockControllerTest, DescenteDeterministeEtAVitesseConstante) {
    core::Level level = makeLevel(30);
    core::SinkingBlockController first(level);
    core::SinkingBlockController second(level);

    first.update(boxAt(2, 0), level.tileMap());
    second.update(boxAt(2, 0), level.tileMap());
    for (int step = 0; step < 60; ++step) {
        first.update(farAway(), level.tileMap());
        second.update(farAway(), level.tileMap());
    }

    EXPECT_FLOAT_EQ(first.boxAt(0).min.y, second.boxAt(0).min.y);
    EXPECT_NEAR(first.boxAt(0).min.y - 1.0f,
                core::SinkingBlockController::SINK_SPEED_CELLS_PER_SECOND, 0.05f);
}

/**
 * @brief Un bloc descendant ne traverse jamais la matiere, et ne repart pas si elle se libere.
 * \castest{<b>Un bloc descendant s'arrete definitivement contre la matiere pleine.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Armer un bloc au-dessus d'un sol proche et le laisser descendre longuement.<br/>2.
 * Retirer ensuite la case pleine sous lui et avancer encore.<br/>
 * \tattendu Le bloc se cale juste au-dessus du sol et n'y entre jamais ; il ne repart pas une fois
 * la case liberee (l'arret est definitif, decision de cadrage du LOT-74).}
 */
TEST(SinkingBlockControllerTest, SArreteContreLaMatiereEtNeRepartPas) {
    core::Level level = makeLevel(5);  // sol en rangee 4, bloc en (2, 1)
    core::SinkingBlockController controller(level);

    controller.update(boxAt(2, 0), level.tileMap());
    for (int step = 0; step < 600; ++step) {
        controller.update(farAway(), level.tileMap());
    }
    EXPECT_FLOAT_EQ(controller.boxAt(0).min.y, 3.0f) << "cale juste au-dessus du sol (rangee 4)";
    EXPECT_FALSE(controller.isRemovedAt(0));

    core::TileMap freed = level.tileMap();
    freed.setTile(2, 4, core::TileType::Empty);
    for (int step = 0; step < 600; ++step) {
        controller.update(farAway(), freed);
    }
    EXPECT_FLOAT_EQ(controller.boxAt(0).min.y, 3.0f) << "un bloc arrete ne repart jamais";
}

/**
 * @brief Sans rien dessous, le bloc finit par sortir du tableau et cesse d'exister.
 * \castest{<b>Un bloc descendant qui franchit le bord bas est retire du niveau.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un niveau dont la colonne du bloc est entierement vide sous lui.<br/>2.
 * Armer le bloc et avancer largement de quoi traverser toute la hauteur.<br/>
 * \tattendu Le bloc est marque retire et n'emet PLUS d'echantillon : un echantillon fantome
 * continuerait de porter le personnage.}
 */
TEST(SinkingBlockControllerTest, SortDuTableauParLeBas) {
    core::TileMap map(6, 8);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 7, core::TileType::Solid);
    }
    map.setTile(2, 7, core::TileType::Empty);  // la colonne du bloc est percee de part en part
    map.setTile(2, 1, core::TileType::SinkingBlock);
    core::Level level("puits", std::move(map), core::GridPosition{0, 0}, core::GridPosition{5, 1},
                      std::vector<core::Mechanism>{});
    core::SinkingBlockController controller(level);

    controller.update(boxAt(2, 0), level.tileMap());
    for (int step = 0; step < 600; ++step) {
        controller.update(farAway(), level.tileMap());
    }

    EXPECT_TRUE(controller.isRemovedAt(0));
    EXPECT_TRUE(controller.samples().empty());
}

/**
 * @brief La grille composee montre le bloc la ou il est VRAIMENT, pas a sa case de fichier.
 * \castest{<b>Un bloc descendant est reporte sur la grille a sa case courante.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Armer un bloc en (2, 1) et le faire descendre de deux cases exactement (48 pas
 * fixes a 2,5 cases/s).<br/>2. Composer la grille avec `collisionMap()`.<br/>
 * \tattendu La case de depart (2, 1) est vide et le bloc apparait en (2, 3). Sans ce report,
 * l'observation de l'agent (`aisolver::ObservationEncoder`) verrait eternellement le bloc a son
 * point de depart et rien la ou il se trouve.}
 */
TEST(SinkingBlockControllerTest, LaGrilleComposeeSuitLaPositionCourante) {
    core::Level level = makeLevel(8);  // sol en rangee 7, bloc en (2, 1)
    core::SinkingBlockController controller(level);

    EXPECT_EQ(controller.collisionMap(level.tileMap()).tile(2, 1), core::TileType::SinkingBlock)
        << "non arme, le bloc reste sur sa case de depart";

    controller.update(boxAt(2, 0), level.tileMap());  // armement : pas 0 de la descente
    for (int step = 0; step < 48; ++step) {           // 48 pas a 2,5 cases/s = 2 cases pile
        controller.update(farAway(), level.tileMap());
    }
    ASSERT_FLOAT_EQ(controller.boxAt(0).min.y, 3.0f);

    const core::TileMap composed = controller.collisionMap(level.tileMap());
    EXPECT_EQ(composed.tile(2, 1), core::TileType::Empty);
    EXPECT_EQ(composed.tile(2, 3), core::TileType::SinkingBlock);
    EXPECT_EQ(level.tileMap().tile(2, 1), core::TileType::SinkingBlock)
        << "la carte du niveau reste immuable, comme pour les blocs volatils";
}

/**
 * @brief Un bloc sorti du tableau n'apparait plus nulle part sur la grille composee.
 * \castest{<b>Un bloc descendant retire disparait de la grille composee.</b><br/>
 * \tcat Unitaire · Sinking Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Faire sortir un bloc par le bas du tableau.<br/>2. Composer la grille.<br/>
 * \tattendu Sa case de depart est vide et aucune case de sa colonne ne porte de bloc descendant.}
 */
TEST(SinkingBlockControllerTest, UnBlocRetireDisparaitDeLaGrilleComposee) {
    core::TileMap map(6, 8);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 7, core::TileType::Solid);
    }
    map.setTile(2, 7, core::TileType::Empty);
    map.setTile(2, 1, core::TileType::SinkingBlock);
    core::Level level("puits", std::move(map), core::GridPosition{0, 0}, core::GridPosition{5, 1},
                      std::vector<core::Mechanism>{});
    core::SinkingBlockController controller(level);

    controller.update(boxAt(2, 0), level.tileMap());
    for (int step = 0; step < 600; ++step) {
        controller.update(farAway(), level.tileMap());
    }
    ASSERT_TRUE(controller.isRemovedAt(0));

    const core::TileMap composed = controller.collisionMap(level.tileMap());
    for (int row = 0; row < composed.height(); ++row) {
        EXPECT_NE(composed.tile(2, row), core::TileType::SinkingBlock) << "rangee " << row;
    }
}
