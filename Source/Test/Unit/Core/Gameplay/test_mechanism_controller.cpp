/**
 * @file test_mechanism_controller.cpp
 * @brief Tests unitaires des mécanismes interrupteur↔porte (`MechanismController`).
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace {

// Niveau minimal 6×3 : un interrupteur en (2,1) lié à une porte en (4,1).
core::Level makeLevelWithMechanism() {
    core::TileMap map(6, 3);
    map.setTile(2, 1, core::TileType::Switch);
    map.setTile(4, 1, core::TileType::Door);
    std::vector<core::Mechanism> mechanisms{
        core::Mechanism{core::GridPosition{2, 1}, core::GridPosition{4, 1}}};
    return core::Level("puzzle", std::move(map), core::GridPosition{0, 0}, core::GridPosition{5, 2},
                       std::move(mechanisms));
}

// Boîte 1×1 posée sur la case (col, row).
core::Aabb boxAt(int col, int row) {
    return core::Aabb::fromTopLeftSize(
        core::Vector2{static_cast<float>(col), static_cast<float>(row)}, core::Vector2{1.0f, 1.0f});
}

}  // namespace

/// Au départ, la porte est **fermée** (solide) ; toucher l'interrupteur l'**ouvre**.
TEST(MechanismControllerTest, ContactOuvreLaPorte) {
    core::MechanismController controller(makeLevelWithMechanism());
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));  // porte fermée = solide
    EXPECT_FALSE(controller.isDoorOpen(0));

    controller.update(boxAt(2, 1));                         // le personnage touche l'interrupteur
    EXPECT_FALSE(controller.collisionMap().isSolid(4, 1));  // porte ouverte = franchissable
    EXPECT_TRUE(controller.isDoorOpen(0));
}

/// La bascule est **sur front** : rester sur l'interrupteur ne re-bascule pas ; revenir bascule.
TEST(MechanismControllerTest, BasculeSurFront) {
    core::MechanismController controller(makeLevelWithMechanism());

    controller.update(boxAt(2, 1));  // 1er contact : ouvre
    EXPECT_TRUE(controller.isDoorOpen(0));
    controller.update(boxAt(2, 1));  // toujours dessus : pas de re-bascule
    EXPECT_TRUE(controller.isDoorOpen(0));

    controller.update(boxAt(0, 1));  // quitte l'interrupteur : rien
    EXPECT_TRUE(controller.isDoorOpen(0));
    controller.update(boxAt(2, 1));  // revient : re-bascule (ferme)
    EXPECT_FALSE(controller.isDoorOpen(0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));
}

/// Loin de l'interrupteur, rien ne change (la porte reste fermée).
TEST(MechanismControllerTest, SansContactRienNeChange) {
    core::MechanismController controller(makeLevelWithMechanism());
    controller.update(boxAt(0, 0));
    EXPECT_TRUE(controller.collisionMap().isSolid(4, 1));
    EXPECT_FALSE(controller.isDoorOpen(0));
}
