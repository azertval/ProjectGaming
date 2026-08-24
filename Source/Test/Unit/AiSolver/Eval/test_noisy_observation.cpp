// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_noisy_observation.cpp
 * @brief Tests unitaires de `NoisyObservationWrapper` (LOT-ANNEXE-15, TACHE-03, `EX-IA-016`).
 */

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/NoisyObservation.h"
#include "AiSolver/Math/Rng.h"
#include "Core/Physics/PlayerInput.h"
#include "../Training/TrivialLevelFixture.h"

using aisolver::EnvironmentConfig;
using aisolver::HeadlessLevelEnvironment;
using aisolver::ObservationEncoder;
using aisolver::Rng;
using aisolver::StepObservation;
using aisolver::eval::NoisyObservationWrapper;
using aisolver_test::TrivialLevelDirectory;

namespace {

StepObservation firstStep(HeadlessLevelEnvironment& environment,
                          const TrivialLevelDirectory& level) {
    const bool loaded = environment.reset(level.levelPath());
    EXPECT_TRUE(loaded);
    return environment.step(core::PlayerInput{});
}

}  // namespace

/**
 * \castest{Amplitude nulle == comportement de l'encodeur sans bruit.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder une observation reelle via ObservationEncoder puis via
 * NoisyObservationWrapper(amplitude=0).<br/>
 * \tattendu Tenseurs identiques ; aucune consommation de rng (meme premier tirage qu'un rng
 * frais).}
 */
TEST(NoisyObservationTest, AmplitudeNulleEstIdentiqueASansBruit) {
    const TrivialLevelDirectory level("noise_zero");
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = 30});
    const StepObservation observation = firstStep(environment, level);

    const ObservationEncoder encoder;
    const NoisyObservationWrapper wrapper(encoder, 0.0f);

    const aisolver::Tensor<float> clean =
        encoder.encode(environment, observation.playerBox, observation.playerState,
                       observation.playerVelocity);

    Rng rng(42);
    const aisolver::Tensor<float> noisy = wrapper.encode(
        environment, observation.playerBox, observation.playerState, observation.playerVelocity, rng);

    ASSERT_EQ(clean.size(), noisy.size());
    for (std::size_t index = 0; index < clean.size(); ++index) {
        EXPECT_FLOAT_EQ(clean.data()[index], noisy.data()[index]);
    }

    // Aucune consommation de rng a amplitude nulle : un rng frais de meme graine tire la meme
    // premiere valeur qu'un rng qui vient de servir a wrapper.encode(...).
    Rng freshRng(42);
    EXPECT_FLOAT_EQ(rng.nextFloat(), freshRng.nextFloat());
}

/**
 * \castest{Une amplitude non nulle modifie l'observation transmise.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder la meme observation via NoisyObservationWrapper(amplitude=0.5), graine
 * fixee.<br/>
 * \tattendu Au moins une composante du tenseur differe de l'encodage propre.}
 */
TEST(NoisyObservationTest, AmplitudeNonNulleModifieLObservation) {
    const TrivialLevelDirectory level("noise_nonzero");
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = 30});
    const StepObservation observation = firstStep(environment, level);

    const ObservationEncoder encoder;
    const NoisyObservationWrapper wrapper(encoder, 0.5f);

    const aisolver::Tensor<float> clean =
        encoder.encode(environment, observation.playerBox, observation.playerState,
                       observation.playerVelocity);

    Rng rng(7);
    const aisolver::Tensor<float> noisy = wrapper.encode(
        environment, observation.playerBox, observation.playerState, observation.playerVelocity, rng);

    ASSERT_EQ(clean.size(), noisy.size());
    bool atLeastOneDiffers = false;
    for (std::size_t index = 0; index < clean.size(); ++index) {
        if (clean.data()[index] != noisy.data()[index]) {
            atLeastOneDiffers = true;
            break;
        }
    }
    EXPECT_TRUE(atLeastOneDiffers);
}

/**
 * \castest{Le bruit d'observation est reproductible a graine fixee.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder deux fois la meme observation avec deux Rng de meme graine.<br/>
 * \tattendu Tenseurs bruites strictement identiques.}
 */
TEST(NoisyObservationTest, ReproductibleAGraineFixee) {
    const TrivialLevelDirectory level("noise_repro");
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = 30});
    const StepObservation observation = firstStep(environment, level);

    const ObservationEncoder encoder;
    const NoisyObservationWrapper wrapper(encoder, 0.3f);

    Rng rngA(99);
    const aisolver::Tensor<float> first = wrapper.encode(
        environment, observation.playerBox, observation.playerState, observation.playerVelocity, rngA);

    Rng rngB(99);
    const aisolver::Tensor<float> second = wrapper.encode(
        environment, observation.playerBox, observation.playerState, observation.playerVelocity, rngB);

    ASSERT_EQ(first.size(), second.size());
    for (std::size_t index = 0; index < first.size(); ++index) {
        EXPECT_FLOAT_EQ(first.data()[index], second.data()[index]);
    }
}

/**
 * \castest{Le bruit ne touche jamais l'etat reel simule.<br/>
 * \tcat Unitaire · AiSolver Eval<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Relever stepIndex/bestDistanceToExit avant encode().<br/>2. Encoder avec bruit
 * (amplitude=1.0).<br/>
 * \tattendu stepIndex et bestDistanceToExit inchanges apres l'appel.}
 */
TEST(NoisyObservationTest, NeModifieJamaisLEtatReelDeLEnvironnement) {
    const TrivialLevelDirectory level("noise_env_untouched");
    HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = 30});
    const StepObservation observation = firstStep(environment, level);
    const int stepIndexBefore = observation.stepIndex;
    const float bestDistanceBefore = environment.bestDistanceToExit();

    const ObservationEncoder encoder;
    const NoisyObservationWrapper wrapper(encoder, 1.0f);
    Rng rng(11);
    const aisolver::Tensor<float> ignored = wrapper.encode(
        environment, observation.playerBox, observation.playerState, observation.playerVelocity, rng);
    (void)ignored;

    // encode() est un decorateur pur : aucun step supplementaire n'a ete simule, aucune
    // progression enregistree par le bruit lui-meme.
    EXPECT_EQ(stepIndexBefore, observation.stepIndex);
    EXPECT_FLOAT_EQ(bestDistanceBefore, environment.bestDistanceToExit());
}
