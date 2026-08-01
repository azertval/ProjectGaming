/**
 * @file test_tensor_ops.cpp
 * @brief Vérifie le câblage du squelette des opérations `aisolver::TensorOps` (LOT-ANNEXE-01,
 * TACHE-03).
 *
 * Squelette : ce test ne vérifie que la compilation/liaison du module `AiSolver` dans
 * `UnitTests`. La couverture réelle (résultats numériques, formes incompatibles) arrive avec
 * l'implémentation de TACHE-03.
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Tensor.h"
#include "AiSolver/Math/TensorOps.h"

TEST(TensorOpsSkeletonTest, Compile) {
    aisolver::Tensor<float> a({2, 2});
}
