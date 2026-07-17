/**
 * @file test_rect.cpp
 * @brief Tests unitaires du rectangle aligné sur les axes de `Core`.
 */

#include <gtest/gtest.h>

#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

namespace {
constexpr float TOLERANCE = 1e-5f;
}

/// Les bords exposés découlent de la position et de la taille.
TEST(RectTest, Bords) {
    const core::Rect rect{{10.0f, 20.0f}, {30.0f, 40.0f}};
    EXPECT_NEAR(rect.left(), 10.0f, TOLERANCE);
    EXPECT_NEAR(rect.top(), 20.0f, TOLERANCE);
    EXPECT_NEAR(rect.right(), 40.0f, TOLERANCE);
    EXPECT_NEAR(rect.bottom(), 60.0f, TOLERANCE);
}

/// Un point strictement intérieur est contenu.
TEST(RectTest, ContientPointInterieur) {
    const core::Rect rect{{0.0f, 0.0f}, {10.0f, 10.0f}};
    EXPECT_TRUE(rect.contains(core::Vector2{5.0f, 5.0f}));
}

/// Contenance inclusive haut/gauche, exclusive bas/droit.
TEST(RectTest, ContientBords) {
    const core::Rect rect{{0.0f, 0.0f}, {10.0f, 10.0f}};
    EXPECT_TRUE(rect.contains(core::Vector2{0.0f, 0.0f}));    // coin haut-gauche inclus
    EXPECT_FALSE(rect.contains(core::Vector2{10.0f, 5.0f}));  // bord droit exclu
    EXPECT_FALSE(rect.contains(core::Vector2{5.0f, 10.0f}));  // bord bas exclu
}

/// Un point extérieur n'est pas contenu.
TEST(RectTest, NeContientPasExterieur) {
    const core::Rect rect{{0.0f, 0.0f}, {10.0f, 10.0f}};
    EXPECT_FALSE(rect.contains(core::Vector2{-1.0f, 5.0f}));
    EXPECT_FALSE(rect.contains(core::Vector2{5.0f, 100.0f}));
}

/// Deux rectangles qui se recouvrent s'intersectent (relation symétrique).
TEST(RectTest, IntersectionRecouvrement) {
    const core::Rect a{{0.0f, 0.0f}, {10.0f, 10.0f}};
    const core::Rect b{{5.0f, 5.0f}, {10.0f, 10.0f}};
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
}

/// Un simple contact par un bord ne compte pas comme intersection.
TEST(RectTest, ContactBordSansIntersection) {
    const core::Rect a{{0.0f, 0.0f}, {10.0f, 10.0f}};
    const core::Rect b{{10.0f, 0.0f}, {10.0f, 10.0f}};  // colle le bord droit de a
    EXPECT_FALSE(a.intersects(b));
}

/// Deux rectangles disjoints ne s'intersectent pas.
TEST(RectTest, Disjonction) {
    const core::Rect a{{0.0f, 0.0f}, {10.0f, 10.0f}};
    const core::Rect b{{20.0f, 20.0f}, {5.0f, 5.0f}};
    EXPECT_FALSE(a.intersects(b));
}
