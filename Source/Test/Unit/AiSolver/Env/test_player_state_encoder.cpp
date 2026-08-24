// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_player_state_encoder.cpp
 * @brief Tests unitaires de aisolver::PlayerStateEncoder (LOT-ANNEXE-06, TACHE-02).
 */

#include <gtest/gtest.h>

#include "AiSolver/Env/PlayerStateEncoder.h"
#include "Core/Levels/LevelLoader.h"

namespace {

// Un niveau minimal valide, avec un budget de saut/dash fini connu.
constexpr const char* LEVEL_WITH_FINITE_BUDGETS = R"({
  "name": "Budget",
  "width": 4,
  "height": 3,
  "jumpBudget": 2,
  "dashBudget": 4,
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 1, "type": "exit" }
  ]
})";

constexpr const char* LEVEL_WITH_UNLIMITED_BUDGETS = R"({
  "name": "Illimite",
  "width": 4,
  "height": 3,
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 1, "type": "exit" }
  ]
})";

}  // namespace

/**
 * @brief `size() == 11` (formule documentee, voir en-tete de PlayerStateEncoder.h) ; un `Player`
 * par defaut produit exactement les valeurs attendues, composant par composant.
 * \castest{<b>`size()` vaut 11 et un `Player` par defaut produit le vecteur attendu.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Charger un niveau a budgets illimites.<br/>2. Encoder un `core::Player`/`core::
 * Velocity` par defaut.<br/>
 * \tattendu `PlayerStateEncoder::size() == 11` ; chaque composant correspond a la formule
 * documentee (vitesse nulle, non au sol, budgets a `1.0f`).}
 */
TEST(PlayerStateEncoderTest, TailleEtValeursParDefaut) {
    EXPECT_EQ(aisolver::PlayerStateEncoder::size(), 11);

    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromString(LEVEL_WITH_UNLIMITED_BUDGETS);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const core::Player player{};
    const core::Velocity velocity{};
    const aisolver::PlayerStateEncoder encoder;
    const aisolver::Tensor<float> encoded = encoder.encode(player, velocity, *loaded.level);

    ASSERT_EQ(encoded.shape(), (std::vector<std::size_t>{11}));
    EXPECT_FLOAT_EQ(encoded.at({0}), 0.0f);  // velocity.x
    EXPECT_FLOAT_EQ(encoded.at({1}), 0.0f);  // velocity.y
    EXPECT_FLOAT_EQ(encoded.at({2}), 0.0f);  // grounded
    EXPECT_FLOAT_EQ(encoded.at({3}), 0.0f);  // wallDirection
    EXPECT_FLOAT_EQ(encoded.at({4}), 0.0f);  // coyoteTimer
    EXPECT_FLOAT_EQ(encoded.at({5}), 0.0f);  // jumpBufferTimer
    EXPECT_FLOAT_EQ(encoded.at({6}), 0.0f);  // wallJumpLockTimer
    EXPECT_FLOAT_EQ(encoded.at({7}), 0.0f);  // dashTimer
    EXPECT_FLOAT_EQ(encoded.at({8}), 0.0f);  // dashAvailable (dashChargesRemaining == 0 par defaut)
    EXPECT_FLOAT_EQ(encoded.at({9}), 1.0f);  // budget de saut illimite
    EXPECT_FLOAT_EQ(encoded.at({10}), 1.0f);  // budget de dash illimite
}

/**
 * @brief Un budget de saut illimite (`-1`) produit `1.0f`, quel que soit `jumpsRemaining`.
 * \castest{<b>Budget illimite encode a `1.0f`.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau a budget illimite.<br/>2. Encoder un `Player` avec
 * `jumpsRemaining = 7` (valeur arbitraire).<br/>
 * \tattendu Composante 9 (budget de saut) vaut `1.0f`.}
 */
TEST(PlayerStateEncoderTest, BudgetIllimiteEncodeUn) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromString(LEVEL_WITH_UNLIMITED_BUDGETS);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    ASSERT_EQ(loaded.level->jumpBudget(), -1);

    core::Player player{};
    player.jumpsRemaining = 7;
    const aisolver::PlayerStateEncoder encoder;
    const aisolver::Tensor<float> encoded = encoder.encode(player, core::Velocity{}, *loaded.level);
    EXPECT_FLOAT_EQ(encoded.at({9}), 1.0f);
}

/**
 * @brief Un budget fini partiellement consomme produit le ratio exact.
 * \castest{<b>Budget fini partiellement consomme encode le ratio exact.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau `jumpBudget = 2`, `dashBudget = 4`.<br/>2. Encoder un `Player` avec
 * `jumpsRemaining = 1`, `dashesRemaining = 1`.<br/>
 * \tattendu Composante 9 (budget de saut) vaut `0.5f`, composante 10 (budget de dash) vaut
 * `0.25f`.}
 */
TEST(PlayerStateEncoderTest, BudgetFiniPartiellementConsomme) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromString(LEVEL_WITH_FINITE_BUDGETS);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    ASSERT_EQ(loaded.level->jumpBudget(), 2);
    ASSERT_EQ(loaded.level->dashBudget(), 4);

    core::Player player{};
    player.jumpsRemaining = 1;
    player.dashesRemaining = 1;
    const aisolver::PlayerStateEncoder encoder;
    const aisolver::Tensor<float> encoded = encoder.encode(player, core::Velocity{}, *loaded.level);
    EXPECT_FLOAT_EQ(encoded.at({9}), 0.5f);
    EXPECT_FLOAT_EQ(encoded.at({10}), 0.25f);
}

/**
 * @brief Les minuteries fraichement apparues (toutes a `0.0f`, comme apres `reset`) encodent des
 * composants de timer tous a `0.0f`.
 * \castest{<b>Minuteries a zero encodent des composants de timer a zero.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau valide.<br/>2. Encoder un `Player` par defaut (minuteries a
 * `0.0f`).<br/>
 * \tattendu Composantes 4 a 7 (timers normalises) valent toutes `0.0f`.}
 */
TEST(PlayerStateEncoderTest, TimersAZeroEncodentZero) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromString(LEVEL_WITH_UNLIMITED_BUDGETS);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const core::Player player{};
    const aisolver::PlayerStateEncoder encoder;
    const aisolver::Tensor<float> encoded = encoder.encode(player, core::Velocity{}, *loaded.level);
    EXPECT_FLOAT_EQ(encoded.at({4}), 0.0f);
    EXPECT_FLOAT_EQ(encoded.at({5}), 0.0f);
    EXPECT_FLOAT_EQ(encoded.at({6}), 0.0f);
    EXPECT_FLOAT_EQ(encoded.at({7}), 0.0f);
}

/**
 * @brief Deux appels `encode` sur le meme `Player`/`Velocity` produisent des vecteurs identiques.
 * \castest{<b>`encode` est deterministe.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder deux fois le meme `Player`/`Velocity`.<br/>
 * \tattendu Les deux vecteurs sont bit-a-bit identiques.}
 */
TEST(PlayerStateEncoderTest, EncodageDeterministe) {
    const core::LevelLoadResult loaded =
        core::LevelLoader::loadFromString(LEVEL_WITH_FINITE_BUDGETS);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    core::Player player{};
    player.jumpsRemaining = 1;
    player.dashesRemaining = 3;
    player.grounded = true;
    player.wallDirection = -1.0f;
    core::Velocity velocity{};
    velocity.value = core::Vector2{2.5f, -3.0f};

    const aisolver::PlayerStateEncoder encoder;
    const aisolver::Tensor<float> first = encoder.encode(player, velocity, *loaded.level);
    const aisolver::Tensor<float> second = encoder.encode(player, velocity, *loaded.level);

    ASSERT_EQ(first.size(), second.size());
    for (std::size_t index = 0; index < first.size(); ++index) {
        EXPECT_EQ(first.data()[index], second.data()[index]);
    }
}
