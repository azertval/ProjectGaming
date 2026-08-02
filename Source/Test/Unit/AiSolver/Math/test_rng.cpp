/**
 * @file test_rng.cpp
 * @brief Tests unitaires du générateur pseudo-aléatoire déterministe `aisolver::Rng`
 * (LOT-ANNEXE-01, TACHE-01).
 */

#include <cmath>

#include <gtest/gtest.h>

#include "AiSolver/Math/Rng.h"

/**
 * @brief Deux instances de `Rng` construites avec la même graine produisent exactement la même
 * séquence de sorties, sur les quatre méthodes.
 * \castest{<b>Rng : reproductibilité à graine fixée.</b><br/>
 * \tcat Unitaire · RNG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux `Rng` avec la même graine `42`.<br/>2. Tirer `nextFloat`,
 * `nextFloat(min,max)`, `nextGaussian`, `nextInt` sur chacune.<br/>
 * \tattendu Chaque paire de tirages est strictement égale.}
 */
TEST(RngTest, ReproductibiliteAGraineFixee) {
    aisolver::Rng rng1(42);
    aisolver::Rng rng2(42);

    EXPECT_EQ(rng1.nextFloat(), rng2.nextFloat());
    EXPECT_EQ(rng1.nextFloat(0.0f, 1.0f), rng2.nextFloat(0.0f, 1.0f));
    EXPECT_EQ(rng1.nextGaussian(0.0f, 1.0f), rng2.nextGaussian(0.0f, 1.0f));
    EXPECT_EQ(rng1.nextInt(0, 10), rng2.nextInt(0, 10));
}

/**
 * @brief Deux instances de `Rng` à graines différentes produisent des séquences différentes
 * (test de non-trivialité, pas une statistique poussée).
 * \castest{<b>Rng : deux graines différentes divergent.</b><br/>
 * \tcat Unitaire · RNG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux `Rng` de graines `42` et `43`.<br/>2. Tirer les quatre méthodes sur
 * chacune.<br/>
 * \tattendu Chaque paire de tirages diffère.}
 */
TEST(RngTest, GrainesDifferentesDivergent) {
    aisolver::Rng rng1(42);
    aisolver::Rng rng2(43);

    EXPECT_NE(rng1.nextFloat(), rng2.nextFloat());
    EXPECT_NE(rng1.nextFloat(0.0f, 1.0f), rng2.nextFloat(0.0f, 1.0f));
    EXPECT_NE(rng1.nextGaussian(0.0f, 1.0f), rng2.nextGaussian(0.0f, 1.0f));
    EXPECT_NE(rng1.nextInt(0, 10), rng2.nextInt(0, 10));
}

/**
 * @brief `nextFloat(min, max)` reste dans `[min, max)` et `nextInt(min, max)` dans `[min, max]`
 * sur un grand nombre de tirages.
 * \castest{<b>Rng : bornes respectées sur un grand nombre de tirages.</b><br/>
 * \tcat Unitaire · RNG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `Rng`.<br/>2. Tirer 10000 fois `nextFloat(-3, 5)` et
 * `nextInt(-3, 5)`.<br/>
 * \tattendu Chaque flottant tiré est dans `[-3, 5)`, chaque entier tiré dans `[-3, 5]`.}
 */
TEST(RngTest, BornesRespecteesSurGrandNombreDeTirages) {
    aisolver::Rng rng(7);
    constexpr int kDraws = 10000;
    constexpr float kMinFloat = -3.0f;
    constexpr float kMaxFloat = 5.0f;
    constexpr int kMinInt = -3;
    constexpr int kMaxInt = 5;

    for (int i = 0; i < kDraws; ++i) {
        const float f = rng.nextFloat(kMinFloat, kMaxFloat);
        EXPECT_GE(f, kMinFloat);
        EXPECT_LT(f, kMaxFloat);

        const int n = rng.nextInt(kMinInt, kMaxInt);
        EXPECT_GE(n, kMinInt);
        EXPECT_LE(n, kMaxInt);
    }
}

/**
 * @brief Sur un grand nombre de tirages, la moyenne et l'écart-type empiriques de `nextGaussian`
 * restent proches (tolérance large) des paramètres demandés — garde-fou faible contre une
 * implémentation inversée, pas une preuve statistique.
 * \castest{<b>Rng : `nextGaussian` moyenne/écart-type plausibles.</b><br/>
 * \tcat Unitaire · RNG<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire un `Rng`.<br/>2. Tirer 20000 fois `nextGaussian(5, 2)`.<br/>3. Calculer
 * moyenne et écart-type empiriques.<br/>
 * \tattendu Moyenne proche de `5` (± `0.1`), écart-type proche de `2` (± `0.1`).}
 */
TEST(RngTest, NextGaussianMoyenneEcartTypePlausibles) {
    aisolver::Rng rng(11);
    constexpr int kDraws = 20000;
    constexpr float kMean = 5.0f;
    constexpr float kStddev = 2.0f;

    double sum = 0.0;
    double sumSquares = 0.0;
    for (int i = 0; i < kDraws; ++i) {
        const double value = rng.nextGaussian(kMean, kStddev);
        sum += value;
        sumSquares += value * value;
    }

    const double empiricalMean = sum / kDraws;
    const double empiricalVariance = sumSquares / kDraws - empiricalMean * empiricalMean;
    const double empiricalStddev = std::sqrt(empiricalVariance);

    EXPECT_NEAR(empiricalMean, kMean, 0.1);
    EXPECT_NEAR(empiricalStddev, kStddev, 0.1);
}

/**
 * @brief Les cinq premières valeurs de `nextFloat()` pour la graine `42` sont figées et
 * comparées à `1e-6` près.
 *
 * Valeurs de référence (*golden values*) obtenues à la première implémentation de TACHE-01 puis
 * figées ici — ce ne sont **pas** des valeurs calculées à la main, mais la sortie observée de
 * `Rng::nextFloat()`. Le test échoue si l'implémentation change de formule (mapping 24 bits sur
 * `_engine() >> 40`) : c'est précisément ce qui garantit qu'un rejeu enregistré avant un tel
 * changement ne serait plus silencieusement rejouable avec des valeurs différentes.
 * \castest{<b>Rng : valeurs de référence figées de `nextFloat()`.</b><br/>
 * \tcat Unitaire · RNG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire `Rng(42)`.<br/>2. Tirer les cinq premiers `nextFloat()`.<br/>
 * \tattendu Chaque valeur correspond à la valeur de référence figée, à `1e-6` près.}
 */
TEST(RngTest, ValeursReferenceFigeesNextFloat) {
    aisolver::Rng rng(42);

    EXPECT_NEAR(rng.nextFloat(), 0.755155504f, 1e-6f);
    EXPECT_NEAR(rng.nextFloat(), 0.639031351f, 1e-6f);
    EXPECT_NEAR(rng.nextFloat(), 0.752145171f, 1e-6f);
    EXPECT_NEAR(rng.nextFloat(), 0.136272669f, 1e-6f);
    EXPECT_NEAR(rng.nextFloat(), 0.903268933f, 1e-6f);
}
