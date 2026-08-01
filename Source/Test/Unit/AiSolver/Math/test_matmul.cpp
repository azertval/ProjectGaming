/**
 * @file test_matmul.cpp
 * @brief Vérifie le câblage du squelette `aisolver::matmul`/`transpose` (LOT-ANNEXE-01, TACHE-04).
 *
 * Squelette : ce test ne vérifie que la compilation/liaison du module `AiSolver` dans
 * `UnitTests`. La couverture réelle (produits croisés, dimensions incompatibles) arrive avec
 * l'implémentation de TACHE-04.
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Matmul.h"
#include "AiSolver/Math/Tensor.h"

TEST(MatmulSkeletonTest, Compile) {
    aisolver::Tensor<float> a({2, 2});
}
