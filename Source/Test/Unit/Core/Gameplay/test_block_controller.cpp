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
    core::Level level("bloc-mur", std::move(map), core::GridPosition{0, 0}, core::GridPosition{5, 1},
                      std::vector<core::Mechanism>{});
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
    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));  // tombé d'une case, sur le sol

    for (int step = 0; step < core::BlockController::FALL_INTERVAL_STEPS; ++step) {
        controller.update(boxAt(5, 0), 0.0f, level.tileMap());
    }
    EXPECT_EQ(controller.positions()[0], (core::GridPosition{2, 1}));  // reste posé, arrêté par le sol
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
