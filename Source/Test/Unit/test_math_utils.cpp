/**
 * @file test_math_utils.cpp
 * @brief Tests unitaires des utilitaires mathématiques (comparaison flottante).
 */

#include <gtest/gtest.h>

#include "Core/Math/MathUtils.h"

/// Deux valeurs identiques sont approximativement égales.
TEST(MathUtilsTest, EgaliteExacte) {
    EXPECT_TRUE(core::approximatelyEqual(1.0f, 1.0f));
    EXPECT_TRUE(core::approximatelyEqual(0.0f, 0.0f));
    EXPECT_TRUE(core::approximatelyEqual(-2.5f, -2.5f));
}

/// Un écart en deçà de la tolérance est considéré comme égal, au-delà comme différent.
TEST(MathUtilsTest, ToleranceParDefaut) {
    EXPECT_TRUE(core::approximatelyEqual(1.0f, 1.0f + 1e-6f));
    EXPECT_FALSE(core::approximatelyEqual(1.0f, 1.1f));
}

/// La tolérance est mise à l'échelle des grandes magnitudes (robustesse relative).
TEST(MathUtilsTest, MiseALEchelleGrandesMagnitudes) {
    EXPECT_TRUE(core::approximatelyEqual(1.0e6f, 1.0e6f + 1.0f));
    EXPECT_FALSE(core::approximatelyEqual(1.0e6f, 1.0e6f + 1000.0f));
}

/// Des signes opposés ne sont pas égaux.
TEST(MathUtilsTest, SignesOpposes) {
    EXPECT_FALSE(core::approximatelyEqual(2.0f, -2.0f));
}

/// Une tolérance explicite élargit l'égalité.
TEST(MathUtilsTest, ToleranceExplicite) {
    EXPECT_FALSE(core::approximatelyEqual(1.0f, 1.5f));
    EXPECT_TRUE(core::approximatelyEqual(1.0f, 1.5f, 1.0f));
}
