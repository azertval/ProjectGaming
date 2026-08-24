// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_action_decoding.cpp
 * @brief Tests unitaires de aisolver::decodeArgmax/decodeStochastic (LOT-ANNEXE-07, TACHE-02).
 */

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/ActionDecoding.h"

namespace {

/// Construit une distribution one-hot (tout a 0, sauf `oneHotIndex` a 1) sur `actionCount()`.
aisolver::Tensor<float> oneHotDistribution(std::size_t oneHotIndex) {
    aisolver::Tensor<float> distribution({aisolver::actionCount()});
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = (index == oneHotIndex) ? 1.0f : 0.0f;
    }
    return distribution;
}

/// Construit une distribution uniforme sur `actionCount()`.
aisolver::Tensor<float> uniformDistribution() {
    aisolver::Tensor<float> distribution({aisolver::actionCount()});
    const float p = 1.0f / static_cast<float>(aisolver::actionCount());
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = p;
    }
    return distribution;
}

}  // namespace

/**
 * @brief `decodeArgmax` sur un maximum unique retourne l'action attendue.
 * \castest{<b>`decodeArgmax` sur un maximum unique retourne l'action attendue.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire une distribution one-hot a l'indice 5.<br/>2. Appeler
 * `decodeArgmax`.<br/>
 * \tattendu L'action retournee correspond exactement a `actionAt(5)`.}
 */
TEST(ActionDecodingTest, ArgmaxSurMaximumUnique) {
    const aisolver::Tensor<float> distribution = oneHotDistribution(5);
    EXPECT_EQ(aisolver::decodeArgmax(distribution), aisolver::actionAt(5));
}

/**
 * @brief `decodeArgmax` en cas d'egalite stricte retourne systematiquement le premier indice.
 * \castest{<b>`decodeArgmax` en cas d'egalite stricte retourne le premier indice.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une distribution avec deux valeurs maximales egales (indices 3 et
 * 7).<br/>2. Appeler `decodeArgmax`.<br/>
 * \tattendu L'action retournee correspond a `actionAt(3)` (le premier des deux).}
 */
TEST(ActionDecodingTest, ArgmaxEgaliteRetourneLePremierIndice) {
    aisolver::Tensor<float> distribution({aisolver::actionCount()});
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = 0.0f;
    }
    distribution.data()[3] = 0.5f;
    distribution.data()[7] = 0.5f;
    EXPECT_EQ(aisolver::decodeArgmax(distribution), aisolver::actionAt(3));
}

/**
 * @brief `decodeStochastic`, sur un grand nombre de tirages, reproduit la distribution d'entree a
 * une tolerance statistique documentee.
 * \castest{<b>`decodeStochastic` reproduit la distribution d'entree sans biais
 * systematique.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une distribution non uniforme (deux actions concentrent la masse de
 * probabilite).<br/>2. Tirer 100000 actions avec `decodeStochastic`, temperature 1.0.<br/>
 * \tattendu La frequence empirique de chaque action concentree s'ecarte de sa probabilite
 * attendue de moins de 0.01 (tolerance documentee, pas une preuve statistique rigoureuse).}
 */
TEST(ActionDecodingTest, StochastiqueReproduitLaDistributionSansBiais) {
    aisolver::Tensor<float> distribution({aisolver::actionCount()});
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = 0.0f;
    }
    distribution.data()[0] = 0.6f;
    distribution.data()[1] = 0.3f;
    // Reliquat de masse reparti sur les indices restants pour rester une distribution valide.
    const float remaining = 0.1f / static_cast<float>(aisolver::actionCount() - 2);
    for (std::size_t index = 2; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = remaining;
    }

    aisolver::Rng rng(4242);
    constexpr int TRIALS = 100000;
    int countIndex0 = 0;
    int countIndex1 = 0;
    for (int trial = 0; trial < TRIALS; ++trial) {
        const aisolver::Action action = aisolver::decodeStochastic(distribution, 1.0f, rng);
        const std::size_t index = aisolver::indexOf(action);
        if (index == 0) {
            ++countIndex0;
        } else if (index == 1) {
            ++countIndex1;
        }
    }

    const float frequency0 = static_cast<float>(countIndex0) / static_cast<float>(TRIALS);
    const float frequency1 = static_cast<float>(countIndex1) / static_cast<float>(TRIALS);
    EXPECT_NEAR(frequency0, 0.6f, 0.01f);
    EXPECT_NEAR(frequency1, 0.3f, 0.01f);
}

/**
 * @brief Une temperature basse rend `decodeStochastic` quasi-equivalent a `decodeArgmax`.
 * \castest{<b>Une temperature basse rend le decodage stochastique quasi-deterministe.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Distribution non uniforme (indice 2 domine).<br/>2. 1000 tirages a temperature
 * 0.01.<br/>
 * \tattendu Plus de 99% des tirages selectionnent l'action de probabilite maximale.}
 */
TEST(ActionDecodingTest, TemperatureBasseQuasiDeterministe) {
    aisolver::Tensor<float> distribution({aisolver::actionCount()});
    for (std::size_t index = 0; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = 0.02f;
    }
    distribution.data()[2] = 1.0f - 0.02f * static_cast<float>(aisolver::actionCount() - 1);

    aisolver::Rng rng(7);
    constexpr int TRIALS = 1000;
    int countExpected = 0;
    for (int trial = 0; trial < TRIALS; ++trial) {
        if (aisolver::indexOf(aisolver::decodeStochastic(distribution, 0.01f, rng)) == 2) {
            ++countExpected;
        }
    }
    EXPECT_GT(countExpected, 990);
}

/**
 * @brief Une temperature haute aplatit la distribution vers un tirage quasi uniforme.
 * \castest{<b>Une temperature haute aplatit vers un tirage quasi uniforme.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Distribution tres concentree (indice 0 a 0.9).<br/>2. 100000 tirages a temperature
 * 50.0.<br/>
 * \tattendu La frequence empirique de l'indice 0 s'approche de 1/actionCount() (tolerance
 * 0.02), loin des 0.9 d'origine.}
 */
TEST(ActionDecodingTest, TemperatureHauteAplatitVersUniforme) {
    aisolver::Tensor<float> distribution({aisolver::actionCount()});
    const float remaining = 0.1f / static_cast<float>(aisolver::actionCount() - 1);
    for (std::size_t index = 1; index < aisolver::actionCount(); ++index) {
        distribution.data()[index] = remaining;
    }
    distribution.data()[0] = 0.9f;

    aisolver::Rng rng(99);
    constexpr int TRIALS = 100000;
    int countIndex0 = 0;
    for (int trial = 0; trial < TRIALS; ++trial) {
        if (aisolver::indexOf(aisolver::decodeStochastic(distribution, 50.0f, rng)) == 0) {
            ++countIndex0;
        }
    }
    const float frequency0 = static_cast<float>(countIndex0) / static_cast<float>(TRIALS);
    const float uniform = 1.0f / static_cast<float>(aisolver::actionCount());
    EXPECT_NEAR(frequency0, uniform, 0.02f);
}

/**
 * @brief Deux instances de `Rng` de meme graine produisent la meme action au premier appel
 * (determinisme intra-graine).
 * \castest{<b>Meme graine -> meme action au premier tirage.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Deux `Rng` construits avec la meme graine.<br/>2. `decodeStochastic` sur chacun avec
 * la meme distribution.<br/>
 * \tattendu Les deux actions retournees sont identiques.}
 */
TEST(ActionDecodingTest, MemeGraineMemeActionAuPremierTirage) {
    const aisolver::Tensor<float> distribution = uniformDistribution();
    aisolver::Rng rngA(123456789ULL);
    aisolver::Rng rngB(123456789ULL);
    EXPECT_EQ(aisolver::decodeStochastic(distribution, 1.0f, rngA),
              aisolver::decodeStochastic(distribution, 1.0f, rngB));
}
