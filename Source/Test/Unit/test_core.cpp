/**
 * @file test_core.cpp
 * @brief Tests unitaires de la bibliothèque Core (amorçage).
 */

#include <gtest/gtest.h>

#include "Core.h"

/// Vérifie que la version du moteur n'est pas vide.
TEST(EngineTest, VersionNonVide) {
    const core::Engine engine;
    EXPECT_FALSE(engine.version().empty());
}
