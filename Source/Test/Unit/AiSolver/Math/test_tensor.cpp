/**
 * @file test_tensor.cpp
 * @brief Vérifie le câblage du squelette `aisolver::Tensor<T>` (LOT-ANNEXE-01, TACHE-02).
 *
 * Squelette : ce test ne vérifie que la compilation/liaison du module `AiSolver` dans
 * `UnitTests`. La couverture réelle (forme, indexation, vues, clone) arrive avec
 * l'implémentation de TACHE-02.
 */

#include <gtest/gtest.h>

#include "AiSolver/Math/Tensor.h"

TEST(TensorSkeletonTest, Construit) {
    aisolver::Tensor<float> tensor({2, 3});
}
