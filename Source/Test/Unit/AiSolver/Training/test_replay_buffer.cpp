// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_replay_buffer.cpp
 * @brief Tests unitaires de `ReplayBuffer` (LOT-ANNEXE-14, TACHE-01).
 */

#include <set>

#include <gtest/gtest.h>

#include "AiSolver/Math/Rng.h"
#include "AiSolver/Training/Dqn/ReplayBuffer.h"

using aisolver::Rng;
using aisolver::Tensor;
using aisolver::training::ReplayBuffer;
using aisolver::training::Transition;

namespace {

Transition markedTransition(std::size_t marker) {
    Transition transition;
    // reward sert de marqueur unique et lisible pour identifier une transition dans les tests.
    transition.reward = static_cast<float>(marker);
    transition.observation = Tensor<float>({1, 1});
    transition.nextObservation = Tensor<float>({1, 1});
    return transition;
}

}  // namespace

/**
 * @brief Au-delà de sa capacité, `ReplayBuffer` évince la transition la plus ancienne : la taille
 * reste plafonnée et seules les transitions les plus récentes restent présentes.
 * \castest{<b>ReplayBuffer : capacite respectee, evincement de la plus ancienne.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Capacite 3, pousser 5 transitions marquees 0..4.<br/>2. Verifier la taille et les
 * marqueurs presents.<br/>
 * \tattendu Taille == 3 ; marqueurs 0 et 1 absents, marqueurs 2/3/4 presents.}
 */
TEST(ReplayBufferTest, CapaciteRespecteeEvincementDeLaPlusAncienne) {
    ReplayBuffer buffer(3);
    for (std::size_t marker = 0; marker < 5; ++marker) {
        buffer.push(markedTransition(marker));
    }
    EXPECT_EQ(buffer.size(), 3u);
    EXPECT_EQ(buffer.capacity(), 3u);

    Rng rng(1);
    std::set<int> markersPresent;
    for (int i = 0; i < 500; ++i) {
        const auto batch = buffer.sample(1, rng);
        markersPresent.insert(static_cast<int>(batch.front().reward));
    }
    EXPECT_EQ(markersPresent.count(0), 0u);
    EXPECT_EQ(markersPresent.count(1), 0u);
    EXPECT_EQ(markersPresent.count(2), 1u);
    EXPECT_EQ(markersPresent.count(3), 1u);
    EXPECT_EQ(markersPresent.count(4), 1u);
}

/**
 * @brief Un grand nombre de tirages couvre l'ensemble des transitions stockées, sans biais vers les
 * plus récemment poussées.
 * \castest{<b>ReplayBuffer : echantillonnage uniforme, sans biais de recence.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. 5 transitions marquees 0..4 dans un tampon de capacite 5.<br/>2. 2000 tirages d'un
 * element.<br/>
 * \tattendu Chaque marqueur 0..4 est tire au moins une fois.}
 */
TEST(ReplayBufferTest, EchantillonnageUniformeSansBiaisDeRecence) {
    ReplayBuffer buffer(5);
    for (std::size_t marker = 0; marker < 5; ++marker) {
        buffer.push(markedTransition(marker));
    }

    Rng rng(7);
    std::set<int> markersSeen;
    for (int i = 0; i < 2000; ++i) {
        const auto batch = buffer.sample(1, rng);
        markersSeen.insert(static_cast<int>(batch.front().reward));
    }
    EXPECT_EQ(markersSeen.size(), 5u);
}

/**
 * @brief `sample` peut tirer la même transition plusieurs fois dans un même mini-lot (tirage avec
 * remise).
 * \castest{<b>ReplayBuffer : tirage avec remise.</b><br/>
 * \tcat Unitaire · AiSolver Training<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Tampon d'une seule transition.<br/>2. `sample(4, rng)`.<br/>
 * \tattendu Le mini-lot contient 4 elements, tous identiques a l'unique transition stockee.}
 */
TEST(ReplayBufferTest, TirageAvecRemiseSurTamponDUneSeuleTransition) {
    ReplayBuffer buffer(10);
    buffer.push(markedTransition(42));

    Rng rng(3);
    const auto batch = buffer.sample(4, rng);
    ASSERT_EQ(batch.size(), 4u);
    for (const Transition& transition : batch) {
        EXPECT_FLOAT_EQ(transition.reward, 42.0f);
    }
}
