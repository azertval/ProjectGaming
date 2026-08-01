/**
 * @file test_rng.cpp
 * @brief Vérifie le câblage du squelette `aisolver::Rng` (LOT-ANNEXE-01, TACHE-01).
 *
 * Squelette : ce test ne vérifie que la compilation/liaison du module `AiSolver` dans
 * `UnitTests`. La couverture réelle (reproductibilité, bornes, valeurs de référence) arrive avec
 * l'implémentation de TACHE-01.
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Rng.h"

TEST(RngSkeletonTest, Construit) {
    aisolver::Rng rng(42);
    float u1 = rng.nextFloat();
    EXPECT_TRUE(u1 >= 0.0f && u1 < 1.0f);
}
