/**
 * @file test_fixed_timestep.cpp
 * @brief Tests unitaires du cadenceur à pas de temps fixe.
 */

#include <gtest/gtest.h>

#include "Core/FixedTimestep.h"

namespace {
constexpr float STEP = 1.0f / 60.0f;
}

/// Un temps écoulé égal au pas fixe produit exactement un pas.
TEST(FixedTimestepTest, UnPasExact) {
    core::FixedTimestep timestep(STEP);
    EXPECT_EQ(timestep.advance(STEP), 1);
}

/// Un temps écoulé inférieur au pas ne produit aucun pas.
TEST(FixedTimestepTest, TempsInsuffisant) {
    core::FixedTimestep timestep(STEP);
    EXPECT_EQ(timestep.advance(STEP * 0.5f), 0);
}

/// Un temps écoulé nul ou négatif ne produit aucun pas.
TEST(FixedTimestepTest, TempsNulOuNegatif) {
    core::FixedTimestep timestep(STEP);
    EXPECT_EQ(timestep.advance(0.0f), 0);
    EXPECT_EQ(timestep.advance(-1.0f), 0);
}

/// 2,5 pas donnent 2 pas, et le reste (0,5 pas) est conservé puis complété.
TEST(FixedTimestepTest, ResteConserve) {
    core::FixedTimestep timestep(STEP);
    EXPECT_EQ(timestep.advance(STEP * 2.5f), 2);
    // Le reste vaut 0,5 pas : l'interpolation doit refléter cette fraction.
    EXPECT_NEAR(timestep.interpolationAlpha(), 0.5f, 1e-4f);
    // Un apport supplémentaire qui, cumulé au reste conservé, dépasse un pas
    // complet déclenche exactement un pas de plus (le reste n'a pas été perdu).
    EXPECT_EQ(timestep.advance(STEP * 0.6f), 1);
}

/// Un temps écoulé énorme est plafonné (anti-spirale de la mort).
TEST(FixedTimestepTest, PlafondAntiSpirale) {
    const int maximum = 5;
    core::FixedTimestep timestep(STEP, maximum);
    EXPECT_EQ(timestep.advance(STEP * 1000.0f), maximum);
    // Le retard a été abandonné : l'appel suivant repart de zéro.
    EXPECT_EQ(timestep.advance(0.0f), 0);
}

/// Le pas fixe exposé correspond à la configuration.
TEST(FixedTimestepTest, PasFixeExpose) {
    core::FixedTimestep timestep(STEP);
    EXPECT_NEAR(timestep.fixedDeltaSeconds(), STEP, 1e-6f);
}
