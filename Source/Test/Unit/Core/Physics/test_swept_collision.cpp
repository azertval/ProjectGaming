/**
 * @file test_swept_collision.cpp
 * @brief Tests unitaires du balayage AABB continu (`sweepAabb`) contre une grille de tuiles.
 */

#include <gtest/gtest.h>

#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/SweptCollision.h"

namespace {

// Grille sans tuile solide, de dimensions données (toutes les cases vides par défaut).
core::TileMap emptyMap(int width, int height) {
    return core::TileMap(width, height);
}

// Boîte 1×1 dont le coin haut-gauche est en (x, y).
core::Aabb unitBox(float x, float y) {
    return core::Aabb::fromTopLeftSize(core::Vector2{x, y}, core::Vector2{1.0f, 1.0f});
}

}  // namespace

/// Sans obstacle, la boîte parcourt tout son déplacement, sans contact.
TEST(SweptCollisionTest, TrajetLibre) {
    const core::TileMap tiles = emptyMap(10, 10);
    const core::SweepResult r = core::sweepAabb(unitBox(0.0f, 0.0f), core::Vector2{3.0f, 2.0f}, tiles);

    EXPECT_FALSE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 3.0f);
    EXPECT_FLOAT_EQ(r.position.y, 2.0f);
    EXPECT_FLOAT_EQ(r.normal.x, 0.0f);
    EXPECT_FLOAT_EQ(r.normal.y, 0.0f);
}

/// Un mur à droite stoppe le mouvement horizontal, la boîte s'arrête au ras du mur.
TEST(SweptCollisionTest, ButeeHorizontale) {
    core::TileMap tiles = emptyMap(10, 3);
    tiles.setTile(3, 1, core::TileType::Solid);
    const core::SweepResult r = core::sweepAabb(unitBox(0.0f, 1.0f), core::Vector2{5.0f, 0.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 2.0f);  // bord droit = 3.0 = bord gauche du mur
    EXPECT_FLOAT_EQ(r.position.y, 1.0f);
    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
}

/// Un sol stoppe la chute : la boîte se pose dessus, normale vers le haut (appui au sol).
TEST(SweptCollisionTest, ButeeVerticale) {
    core::TileMap tiles = emptyMap(3, 10);
    tiles.setTile(1, 5, core::TileType::Solid);
    const core::SweepResult r = core::sweepAabb(unitBox(1.0f, 0.0f), core::Vector2{0.0f, 10.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.y, 4.0f);  // bord bas = 5.0 = haut du sol
    EXPECT_FLOAT_EQ(r.position.x, 1.0f);
    EXPECT_FLOAT_EQ(r.normal.y, -1.0f);
}

/// Cœur de la décision de cadrage : un pas plus grand qu'une tuile ne traverse pas le mur.
TEST(SweptCollisionTest, NonTunneling) {
    core::TileMap tiles = emptyMap(8, 2);
    tiles.setTile(5, 0, core::TileType::Solid);
    const core::SweepResult r = core::sweepAabb(unitBox(0.0f, 0.0f), core::Vector2{10.0f, 0.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 4.0f);  // stoppé au ras du mur malgré delta = 10
    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
}

/// Glissement : en butant contre un mur vertical, la boîte continue de descendre le long du mur.
TEST(SweptCollisionTest, GlissementLeLongDuMur) {
    core::TileMap tiles = emptyMap(6, 4);
    tiles.setTile(3, 0, core::TileType::Solid);
    tiles.setTile(3, 1, core::TileType::Solid);
    tiles.setTile(3, 2, core::TileType::Solid);
    const core::SweepResult r = core::sweepAabb(unitBox(0.0f, 0.0f), core::Vector2{5.0f, 2.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 2.0f);  // bloqué au ras du mur (bord droit = 3.0)
    EXPECT_FLOAT_EQ(r.position.y, 2.0f);  // mais la descente complète (glissement) a eu lieu
    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
}

/// Un mur à gauche stoppe le déplacement vers la gauche, bord gauche collé au mur.
TEST(SweptCollisionTest, ButeeGauche) {
    core::TileMap tiles = emptyMap(10, 3);
    tiles.setTile(2, 1, core::TileType::Solid);
    const core::SweepResult r = core::sweepAabb(unitBox(6.0f, 1.0f), core::Vector2{-5.0f, 0.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 3.0f);  // bord gauche = 3.0 = bord droit du mur (col 2)
    EXPECT_FLOAT_EQ(r.normal.x, 1.0f);
}

/// Un plafond stoppe la montée, bord haut collé sous le plafond.
TEST(SweptCollisionTest, ButeePlafond) {
    core::TileMap tiles = emptyMap(3, 10);
    tiles.setTile(1, 2, core::TileType::Solid);
    const core::SweepResult r = core::sweepAabb(unitBox(1.0f, 6.0f), core::Vector2{0.0f, -5.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.y, 3.0f);  // bord haut = 3.0 = bas du plafond (ligne 2)
    EXPECT_FLOAT_EQ(r.normal.y, 1.0f);
}

/// En marchant le long d'un sol, la boîte n'est pas bloquée horizontalement par ce sol (peau).
TEST(SweptCollisionTest, MarcheSurLeSolSansBlocageHorizontal) {
    core::TileMap tiles = emptyMap(10, 3);
    for (int col = 0; col < 10; ++col) {
        tiles.setTile(col, 2, core::TileType::Solid);  // sol continu sur la ligne 2
    }
    // La boîte repose sur le sol (bord bas = 2.0) et avance à droite.
    const core::SweepResult r = core::sweepAabb(unitBox(0.0f, 1.0f), core::Vector2{3.0f, 0.0f}, tiles);

    EXPECT_FALSE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 3.0f);  // avance librement, le sol dessous ne bloque pas
}
