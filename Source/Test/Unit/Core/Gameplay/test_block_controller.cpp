// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_block_controller.cpp
 * @brief Tests unitaires des blocs poussables (`BlockController`, `EX-GP-022`).
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/BlockController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace {

// Niveau minimal 6×3, sol en bas (rangée 2) : un bloc en (2, 1), posé sur le sol.
core::Level makeLevelWithBlock() {
    core::TileMap map(6, 3);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 2, core::TileType::Solid);  // sol
    }
    map.setTile(2, 1, core::TileType::Block);
    return core::Level("bloc", std::move(map), core::GridPosition{0, 0}, core::GridPosition{5, 1},
                       std::vector<core::Mechanism>{});
}

// Boîte 1×1 posée sur la case (col, row).
core::Aabb boxAt(int col, int row) {
    return core::Aabb::fromTopLeftSize(
        core::Vector2{static_cast<float>(col), static_cast<float>(row)}, core::Vector2{1.0f, 1.0f});
}

}  // namespace

/**
 * @brief À la construction, le bloc est à sa position du niveau, et cette case est solide.
 * \castest{<b>À la construction, le bloc est à sa position du niveau, et cette case est
 * solide.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu À la construction, le bloc est à sa position du niveau, et cette case est solide.
 * }
 */
TEST(BlockControllerTest, PositionInitiale) {
    core::Level level = makeLevelWithBlock();
    core::BlockController controller(level);

    ASSERT_EQ(controller.positions().size(), 1u);
    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));
    EXPECT_TRUE(controller.collisionMap(level.tileMap()).isSolid(2, 1));
}

/**
 * @brief Toucher un bloc à droite en se déplaçant vers la droite le pousse d'une case, si la case
 * suivante est libre.
 * \castest{<b>Toucher un bloc à droite en se déplaçant vers la droite le pousse d'une case, si la
 * case suivante est libre.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Toucher un bloc à droite en se déplaçant vers la droite le pousse d'une case, si la
 * case suivante est libre.
 * }
 */
TEST(BlockControllerTest, PousseVersLaDroiteSiLibre) {
    core::Level level = makeLevelWithBlock();
    core::BlockController controller(level);

    // Personnage juste à gauche du bloc (colonne 1), poussant vers la droite.
    controller.update(boxAt(1, 1), /*moveIntentX=*/1.0f, level.tileMap());

    ASSERT_EQ(controller.positions().size(), 1u);
    EXPECT_EQ(controller.positions()[0], (core::GridPosition{3, 1}));
}

/**
 * @brief Un bloc ne peut pas être poussé à travers un mur.
 * \castest{<b>Un bloc ne peut pas être poussé à travers un mur.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc ne peut pas être poussé à travers un mur.
 * }
 */
TEST(BlockControllerTest, PousseeRefuseeContreUnMur) {
    core::TileMap map(6, 3);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 2, core::TileType::Solid);
    }
    map.setTile(2, 1, core::TileType::Block);
    map.setTile(3, 1, core::TileType::Solid);  // mur juste à droite du bloc
    core::Level level("bloc-mur", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 1}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    controller.update(boxAt(1, 1), /*moveIntentX=*/1.0f, level.tileMap());

    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));  // n'a pas bougé
}

/**
 * @brief Un bloc ne peut pas être poussé sur un autre bloc.
 * \castest{<b>Un bloc ne peut pas être poussé sur un autre bloc.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc ne peut pas être poussé sur un autre bloc.
 * }
 */
TEST(BlockControllerTest, PousseeRefuseeContreUnAutreBloc) {
    core::TileMap map(6, 3);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 2, core::TileType::Solid);
    }
    map.setTile(2, 1, core::TileType::Block);
    map.setTile(3, 1, core::TileType::Block);  // deuxieme bloc juste a droite du premier
    core::Level level("deux-blocs", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 1}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    controller.update(boxAt(1, 1), /*moveIntentX=*/1.0f, level.tileMap());

    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));  // aucun des deux n'a bougé
    EXPECT_EQ(controller.positions()[1], (core::GridPosition{3, 1}));
}

/**
 * @brief Sans intention de déplacement horizontal, un bloc touché ne bouge pas.
 * \castest{<b>Sans intention de déplacement horizontal, un bloc touché ne bouge pas.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans intention de déplacement horizontal, un bloc touché ne bouge pas.
 * }
 */
TEST(BlockControllerTest, ContactSansIntentionNePousseRien) {
    core::Level level = makeLevelWithBlock();
    core::BlockController controller(level);

    controller.update(boxAt(1, 1), /*moveIntentX=*/0.0f, level.tileMap());

    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));
}

/**
 * @brief Un bloc posé sur le sol, non poussé, ne tombe jamais.
 * \castest{<b>Un bloc posé sur le sol, non poussé, ne tombe jamais.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc posé sur le sol, non poussé, ne tombe jamais.
 * }
 */
TEST(BlockControllerTest, BlocSoutenuNeTombePas) {
    core::Level level = makeLevelWithBlock();
    core::BlockController controller(level);

    for (int step = 0; step < core::BlockController::FALL_INTERVAL_STEPS * 3; ++step) {
        controller.update(boxAt(0, 0), 0.0f, level.tileMap());
    }

    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));
}

/**
 * @brief Un bloc sans support tombe d'une case toutes les `FALL_INTERVAL_STEPS` mises à jour.
 * \castest{<b>Un bloc sans support tombe d'une case toutes les FALL_INTERVAL_STEPS mises à
 * jour.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc sans support tombe d'une case toutes les FALL_INTERVAL_STEPS mises à jour.
 * }
 */
TEST(BlockControllerTest, BlocNonSoutenuTombe) {
    // Bloc en (2, 0) ; la rangée 1 est libre, la rangée 2 (sol) est solide : une seule case de
    // chute possible, le bloc doit ensuite rester posé en (2, 1) sans tenter d'entrer dans le sol.
    core::TileMap map(6, 3);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 2, core::TileType::Solid);
    }
    map.setTile(2, 0, core::TileType::Block);
    core::Level level("bloc-suspendu", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 1}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    for (int step = 0; step < core::BlockController::FALL_INTERVAL_STEPS - 1; ++step) {
        controller.update(boxAt(5, 0), 0.0f, level.tileMap());
    }
    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 0}));  // pas encore tombé

    controller.update(boxAt(5, 0), 0.0f, level.tileMap());
    EXPECT_EQ(controller.positions()[0],
              (core::GridPosition{2, 1}));  // tombé d'une case, sur le sol

    for (int step = 0; step < core::BlockController::FALL_INTERVAL_STEPS; ++step) {
        controller.update(boxAt(5, 0), 0.0f, level.tileMap());
    }
    EXPECT_EQ(controller.positions()[0],
              (core::GridPosition{2, 1}));  // reste posé, arrêté par le sol
}

/**
 * @brief `collisionMap` efface la case d'origine d'un bloc déplacé (pas de mur fantôme).
 * \castest{<b>collisionMap efface la case d'origine d'un bloc déplacé (pas de mur
 * fantôme).</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu collisionMap efface la case d'origine d'un bloc déplacé (pas de mur fantôme).
 * }
 */
TEST(BlockControllerTest, CollisionMapEffaceLAncienneCase) {
    core::Level level = makeLevelWithBlock();
    core::BlockController controller(level);

    controller.update(boxAt(1, 1), /*moveIntentX=*/1.0f, level.tileMap());  // pousse (2,1) -> (3,1)

    const core::TileMap collision = controller.collisionMap(level.tileMap());
    EXPECT_FALSE(collision.isSolid(2, 1));  // ancienne case : plus solide
    EXPECT_TRUE(collision.isSolid(3, 1));   // nouvelle case : solide
}

/**
 * @brief Les trois types de bloc (plein, `×0.5`, `×0.25`) sont reconnus avec le facteur de taille
 * attendu (`EX-GP-005`).
 * \castest{<b>Les trois types de bloc sont reconnus avec le facteur de taille attendu.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Facteur 1 pour Block, 0,5 pour BlockHalf, 0,25 pour BlockQuarter, dans l'ordre de
 * lecture de la grille.
 * }
 */
TEST(BlockControllerTest, ReconnaitLesTroisFacteursDeTaille) {
    core::TileMap map(6, 3);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 2, core::TileType::Solid);
    }
    map.setTile(1, 1, core::TileType::Block);
    map.setTile(2, 1, core::TileType::BlockHalf);
    map.setTile(3, 1, core::TileType::BlockQuarter);
    core::Level level("bloc-tailles", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 0}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    ASSERT_EQ(controller.positions().size(), 3u);
    ASSERT_EQ(controller.scales().size(), 3u);
    EXPECT_FLOAT_EQ(controller.scales()[0], 1.0f);
    EXPECT_FLOAT_EQ(controller.scales()[1], 0.5f);
    EXPECT_FLOAT_EQ(controller.scales()[2], 0.25f);
}

/**
 * @brief `boxAt` renvoie une boîte centrée et réduite pour un bloc `×0.5`/`×0.25`, pleine case
 * pour un bloc plein.
 * \castest{<b>boxAt renvoie une boîte centrée et réduite pour un bloc réduit, pleine case pour un
 * bloc plein.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Bloc plein : boîte occupant toute la case (2,1)-(3,2). Bloc ×0,5 en (3,1) : boîte
 * centrée de 0,5×0,5, marge de 0,25 de chaque côté.
 * }
 */
TEST(BlockControllerTest, BoxAtCentreEtReduitSelonLaTaille) {
    core::TileMap map(6, 3);
    map.setTile(2, 1, core::TileType::Block);
    map.setTile(3, 1, core::TileType::BlockHalf);
    core::Level level("bloc-box", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 0}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    const core::Aabb fullBox = controller.boxAt(0);
    EXPECT_FLOAT_EQ(fullBox.min.x, 2.0f);
    EXPECT_FLOAT_EQ(fullBox.min.y, 1.0f);
    EXPECT_FLOAT_EQ(fullBox.max.x, 3.0f);
    EXPECT_FLOAT_EQ(fullBox.max.y, 2.0f);

    const core::Aabb halfBox = controller.boxAt(1);
    EXPECT_FLOAT_EQ(halfBox.min.x, 3.25f);  // 3 + (1 - 0.5) / 2
    EXPECT_FLOAT_EQ(halfBox.min.y, 1.25f);
    EXPECT_FLOAT_EQ(halfBox.max.x, 3.75f);
    EXPECT_FLOAT_EQ(halfBox.max.y, 1.75f);
}

/**
 * @brief `collisionMap` ne rend jamais solide la case d'un bloc réduit — elle doit rester
 * franchissable autour de lui (`EX-GP-005`), à la différence d'un bloc plein.
 * \castest{<b>collisionMap ne rend jamais solide la case d'un bloc réduit, à la différence d'un
 * bloc plein.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Case du bloc plein solide, cases des blocs réduits (`×0,5`/`×0,25`) non solides.
 * }
 */
TEST(BlockControllerTest, CollisionMapNeSolidifiePasLesBlocsReduits) {
    core::TileMap map(6, 3);
    map.setTile(1, 1, core::TileType::Block);
    map.setTile(2, 1, core::TileType::BlockHalf);
    map.setTile(3, 1, core::TileType::BlockQuarter);
    core::Level level("bloc-reduit-grille", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 0}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    const core::TileMap collision = controller.collisionMap(level.tileMap());
    EXPECT_TRUE(collision.isSolid(1, 1));   // bloc plein : solide dans la grille classique
    EXPECT_FALSE(collision.isSolid(2, 1));  // bloc x0,5 : franc dans la grille (box-boite a part)
    EXPECT_FALSE(collision.isSolid(3, 1));  // bloc x0,25 : idem
}

/**
 * @brief Un bloc réduit (`×0.5`) se pousse exactement comme un bloc plein — case par case —, mais
 * le contact déclencheur se teste contre sa boîte RÉDUITE, pas contre la case entière
 * (`EX-GP-005`) : un personnage aligné sur le bord de la case (pas de la boîte réduite) ne le
 * touche pas encore.
 * \castest{<b>Un bloc réduit se pousse comme un bloc plein, mais le contact se teste contre sa
 * boîte réduite, pas la case entière.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Aligné sur le bord de la CASE (2,0) : aucune poussée (l'espace vide autour du bloc
 * réduit n'est pas un contact). Aligné sur le bord de sa boîte RÉDUITE (2,25) : poussée d'une case
 * entière, comme un bloc plein.
 * }
 */
TEST(BlockControllerTest, BlocReduitPousseSelonSaBoiteReelle) {
    core::TileMap map(6, 3);
    map.setTile(2, 1,
                core::TileType::BlockHalf);  // boîte réelle centrée : [2.25, 2.75] x [1.25, 1.75]
    core::Level level("bloc-demi", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 1}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    // Aligné sur le bord de la CASE entière (x max = 2.0) : encore dans le vide autour du bloc
    // réduit (qui ne commence qu'à x = 2.25) — aucun contact, aucune poussée.
    const core::Aabb atCellEdge =
        core::Aabb::fromTopLeftSize(core::Vector2{1.0f, 1.25f}, core::Vector2{1.0f, 1.0f});
    controller.update(atCellEdge, /*moveIntentX=*/1.0f, level.tileMap());
    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));  // pas encore pousse

    // Aligné sur le bord de la boîte RÉDUITE (x max = 2.25) : contact réel, poussée d'une case
    // entière (comme un bloc plein — seul le contact a changé, pas le déplacement).
    const core::Aabb atReducedBoxEdge =
        core::Aabb::fromTopLeftSize(core::Vector2{1.25f, 1.25f}, core::Vector2{1.0f, 1.0f});
    controller.update(atReducedBoxEdge, /*moveIntentX=*/1.0f, level.tileMap());
    EXPECT_EQ(controller.positions()[0], (core::GridPosition{3, 1}));  // pousse d'une case entiere
}

/**
 * @brief Un bloc suspendu au-dessus d'une pente ne tombe pas au travers (`EX-GP-003`/`EX-GP-022`)
 * : `BlockController` n'a aucune notion de suivi de surface, une pente est donc traitée comme un
 * obstacle simple (case par case), pas comme une case libre.
 * \castest{<b>Un bloc suspendu au-dessus d'une pente ne tombe pas au travers.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le bloc reste posé sur la pente (une case au-dessus), même après largement plus que
 * `FALL_INTERVAL_STEPS` pas.
 * }
 */
TEST(BlockControllerTest, BlocNeTombePasATraversUnePente) {
    // Bloc en (2, 0) ; pente en (2, 1), juste en dessous ; sol tres eloigne (rangee 5), pour
    // prouver qu'un bloc qui traverserait la pente tomberait bien plus bas, pas seulement d'une
    // case par coincidence.
    core::TileMap map(6, 6);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 5, core::TileType::Solid);
    }
    map.setTile(2, 1, core::TileType::SlopeUpRight);
    map.setTile(2, 0, core::TileType::Block);
    core::Level level("bloc-pente", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 0}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    for (int step = 0; step < core::BlockController::FALL_INTERVAL_STEPS * 3; ++step) {
        controller.update(boxAt(5, 0), 0.0f, level.tileMap());
    }

    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 0}));  // n'a pas bouge
}

/**
 * @brief Un bloc ne peut pas être poussé sur une pente (`EX-GP-003`/`EX-GP-022`) : traitée comme
 * un obstacle simple, pas comme une case libre.
 * \castest{<b>Un bloc ne peut pas être poussé sur une pente.</b><br/>
 * \tcat Unitaire · Block Controller<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le bloc n'a pas bougé, la case de la pente reste occupée par la pente.
 * }
 */
TEST(BlockControllerTest, PousseeRefuseeContreUnePente) {
    core::TileMap map(6, 3);
    for (int column = 0; column < 6; ++column) {
        map.setTile(column, 2, core::TileType::Solid);
    }
    map.setTile(2, 1, core::TileType::Block);
    map.setTile(3, 1, core::TileType::SlopeUpRight);  // pente juste a droite du bloc
    core::Level level("bloc-pente-poussee", std::move(map), core::GridPosition{0, 0},
                      core::GridPosition{5, 1}, std::vector<core::Mechanism>{});
    core::BlockController controller(level);

    controller.update(boxAt(1, 1), /*moveIntentX=*/1.0f, level.tileMap());

    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));  // n'a pas bouge
}
