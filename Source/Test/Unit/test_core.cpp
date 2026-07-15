/**
 * @file test_core.cpp
 * @brief Tests unitaires de la bibliothèque Core (placeholder).
 */

#include <gtest/gtest.h>

#include "Core.h"

TEST(CoreTest, VersionNonVide) {
    EXPECT_FALSE(core::version().empty());
}
