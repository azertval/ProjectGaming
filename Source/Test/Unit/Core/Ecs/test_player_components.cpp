/**
 * @file test_player_components.cpp
 * @brief Tests unitaires des composants du personnage (données pures) : valeurs par défaut.
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/PhysicsConfig.h"
#include "Core/Physics/PlayerInput.h"

/// Un Collider par défaut a une boîte nulle : ses dimensions sont fixées au moment du spawn.
TEST(PlayerComponentsTest, ColliderParDefautEstNul) {
    const core::Collider collider;
    EXPECT_FLOAT_EQ(collider.size.x, 0.0f);
    EXPECT_FLOAT_EQ(collider.size.y, 0.0f);
}

/// Le personnage démarre en l'air (pas au sol), minuteries de saut à zéro.
TEST(PlayerComponentsTest, PlayerParDefautPasAuSol) {
    const core::Player player;
    EXPECT_FALSE(player.grounded);
    EXPECT_FLOAT_EQ(player.coyoteTimer, 0.0f);
    EXPECT_FLOAT_EQ(player.jumpBufferTimer, 0.0f);
}

/// L'intention d'entrée par défaut est neutre : immobile, aucun saut.
TEST(PlayerComponentsTest, PlayerInputParDefautImmobile) {
    const core::PlayerInput input;
    EXPECT_FLOAT_EQ(input.moveX, 0.0f);
    EXPECT_FALSE(input.jumpPressed);
    EXPECT_FALSE(input.jumpHeld);
}

/// Les réglages de physique par défaut sont plausibles et non nuls (garde-fou de cohérence).
TEST(PlayerComponentsTest, PhysicsConfigParDefautPlausible) {
    const core::PhysicsConfig config;
    EXPECT_GT(config.moveSpeed, 0.0f);
    EXPECT_GT(config.gravity, 0.0f);
    EXPECT_GT(config.maxFallSpeed, 0.0f);
    EXPECT_GT(config.jumpSpeed, 0.0f);
    EXPECT_GT(config.coyoteTime, 0.0f);
    EXPECT_GT(config.jumpBufferTime, 0.0f);
    // Facteur de coupe dans [0, 1] : 0 = coupe nette, 1 = pas de coupe.
    EXPECT_GE(config.jumpCutFactor, 0.0f);
    EXPECT_LE(config.jumpCutFactor, 1.0f);
}

/// Les composants sont des agrégats : l'initialisation par accolades renseigne les champs.
TEST(PlayerComponentsTest, AggregationRenseigneLesChamps) {
    const core::Collider collider{core::Vector2{2.0f, 3.0f}};
    EXPECT_FLOAT_EQ(collider.size.x, 2.0f);
    EXPECT_FLOAT_EQ(collider.size.y, 3.0f);

    const core::PlayerInput input{-1.0f};
    EXPECT_FLOAT_EQ(input.moveX, -1.0f);
}
