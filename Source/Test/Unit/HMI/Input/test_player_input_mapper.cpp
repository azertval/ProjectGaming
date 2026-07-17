/**
 * @file test_player_input_mapper.cpp
 * @brief Tests unitaires de la traduction clavier → intention (`toPlayerInput`).
 */

#include <gtest/gtest.h>

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"
#include "HMI/Input/PlayerInputMapper.h"

namespace {

// Construit un état clavier avec un ensemble de touches enfoncées.
hmi::InputState withKeys(std::initializer_list<hmi::Key> keys) {
    hmi::InputState input;
    for (const hmi::Key key : keys) {
        input.onKeyDown(key);
    }
    return input;
}

}  // namespace

/// Flèche gauche seule → intention vers la gauche (-1).
TEST(PlayerInputMapperTest, FlecheGauche) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Left})).moveX, -1.0f);
}

/// Flèche droite seule → intention vers la droite (+1).
TEST(PlayerInputMapperTest, FlecheDroite) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Right})).moveX, 1.0f);
}

/// Touches alternatives ZQSD : Q → gauche, D → droite.
TEST(PlayerInputMapperTest, TouchesAlternativesQetD) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Q})).moveX, -1.0f);
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::D})).moveX, 1.0f);
}

/// Aucune touche → intention nulle (immobile).
TEST(PlayerInputMapperTest, AucuneTouche) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(hmi::InputState{}).moveX, 0.0f);
}

/// Gauche et droite simultanées → neutralisation (0).
TEST(PlayerInputMapperTest, GaucheEtDroiteSeNeutralisent) {
    EXPECT_FLOAT_EQ(hmi::toPlayerInput(withKeys({hmi::Key::Left, hmi::Key::Right})).moveX, 0.0f);
}
