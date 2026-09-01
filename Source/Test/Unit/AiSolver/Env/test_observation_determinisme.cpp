// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_observation_determinisme.cpp
 * @brief Déterminisme de l'observation complète assemblée (LOT-ANNEXE-06, TACHE-04).
 */

#include <cmath>
#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "AiSolver/Env/MechanismStateEncoder.h"
#include "AiSolver/Env/PlayerStateEncoder.h"
#include "AiSolver/Env/TileWindowEncoder.h"
#include "Core/Physics/PlayerInput.h"

namespace {

std::filesystem::path levelPath(const char* file) {
    return std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
}

// Case de grille ou se trouve le centre d'une boite -- conversion a la charge de l'appelant
// (TileWindowEncoder/MechanismStateEncoder restent des donnees pures, voir TACHE-01).
core::GridPosition centerCell(const core::Aabb& box) {
    const core::Vector2 center = (box.min + box.max) * 0.5f;
    return core::GridPosition{static_cast<int>(std::floor(center.x)),
                              static_cast<int>(std::floor(center.y))};
}

// Assemble l'observation complete (les trois encodeurs de TACHE-01/02/03) pour un etat donne de
// HeadlessLevelEnvironment, apres un step().
struct AssembledObservation {
    aisolver::Tensor<float> tiles;
    aisolver::Tensor<float> player;
    aisolver::Tensor<float> mechanisms;
};

AssembledObservation assemble(const aisolver::HeadlessLevelEnvironment& env,
                              const aisolver::StepObservation& observation, int radius) {
    const aisolver::TileWindowEncoder tileEncoder(radius);
    const aisolver::PlayerStateEncoder playerEncoder;
    const aisolver::MechanismStateEncoder mechanismEncoder;

    const core::GridPosition center = centerCell(observation.playerBox);
    return AssembledObservation{
        tileEncoder.encode(env.collisionMap(), center),
        playerEncoder.encode(observation.playerState, observation.playerVelocity, env.level()),
        mechanismEncoder.encode(env.mechanisms(), env.dangers(), env.platforms(), env.level(),
                                center, radius),
    };
}

void expectBitIdentical(const aisolver::Tensor<float>& first,
                        const aisolver::Tensor<float>& second) {
    ASSERT_EQ(first.size(), second.size());
    for (std::size_t index = 0; index < first.size(); ++index) {
        EXPECT_EQ(first.data()[index], second.data()[index]);
    }
}

// Rejoue @p file avec un script constant "avancer a droite" pendant @p steps pas, puis encode
// l'observation complete DEUX FOIS sur le meme etat (sans avancer entre les deux), et compare.
void expectDeterministicObservation(const char* file, int steps) {
    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(levelPath(file))) << file;

    aisolver::StepObservation last{};
    for (int step = 0; step < steps; ++step) {
        last = env.step(core::PlayerInput{1.0f});
    }

    const AssembledObservation first = assemble(env, last, 3);
    const AssembledObservation second = assemble(env, last, 3);

    expectBitIdentical(first.tiles, second.tiles);
    expectBitIdentical(first.player, second.player);
    expectBitIdentical(first.mechanisms, second.mechanisms);
}

}  // namespace

/**
 * @brief L'observation complete assemblee est deterministe sur un niveau a mecanisme.
 * \castest{<b>Observation deterministe sur un niveau a mecanisme (interrupteur/porte).</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Rejouer `demo-interrupteur.json` 30 pas.<br/>2. Assembler l'observation complete deux
 * fois sur le meme etat.<br/>
 * \tattendu Les deux observations (fenetre, etat joueur, mecanismes) sont bit-a-bit identiques.}
 */
TEST(ObservationDeterminismeTest, NiveauAMecanisme) {
    expectDeterministicObservation("demo-interrupteur.json", 30);
}

/**
 * @brief L'observation complete assemblee est deterministe sur un niveau a bloc poussable.
 * \castest{<b>Observation deterministe sur un niveau a bloc poussable.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Rejouer `demo-bloc.json` 30 pas.<br/>2. Assembler l'observation complete deux fois
 * sur le meme etat.<br/>
 * \tattendu Les deux observations sont bit-a-bit identiques.}
 */
TEST(ObservationDeterminismeTest, NiveauABlocPoussable) {
    expectDeterministicObservation("demo-bloc.json", 30);
}

/**
 * @brief L'observation complete assemblee est deterministe sur un niveau a danger avance.
 * \castest{<b>Observation deterministe sur un niveau a danger avance.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Rejouer `demo-dangers-avances.json` 30 pas.<br/>2. Assembler l'observation complete
 * deux fois sur le meme etat.<br/>
 * \tattendu Les deux observations sont bit-a-bit identiques.}
 */
TEST(ObservationDeterminismeTest, NiveauADangerAvance) {
    expectDeterministicObservation("demo-dangers-avances.json", 30);
}
