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
#include "Core/Physics/PlayerSpawn.h"

/// Un Collider par défaut a une boîte nulle : ses dimensions sont fixées au moment du spawn.
TEST(PlayerComponentsTest, ColliderParDefautEstNul) {
    const core::Collider collider;
    EXPECT_FLOAT_EQ(collider.size.x, 0.0f);
    EXPECT_FLOAT_EQ(collider.size.y, 0.0f);
}

/// Le personnage démarre en l'air, minuteries/compteurs à zéro, orienté à droite, dash
/// indisponible.
TEST(PlayerComponentsTest, PlayerParDefautPasAuSol) {
    const core::Player player;
    EXPECT_FALSE(player.grounded);
    EXPECT_FLOAT_EQ(player.coyoteTimer, 0.0f);
    EXPECT_FLOAT_EQ(player.jumpBufferTimer, 0.0f);
    EXPECT_EQ(player.airJumpsRemaining, 0);
    EXPECT_FLOAT_EQ(player.facing, 1.0f);
    EXPECT_FLOAT_EQ(player.wallDirection, 0.0f);
    EXPECT_FLOAT_EQ(player.wallJumpLockTimer, 0.0f);
    EXPECT_FALSE(player.dashAvailable);
    EXPECT_FLOAT_EQ(player.dashTimer, 0.0f);
    EXPECT_EQ(player.jumpsRemaining, -1);  // budget illimité par défaut
    EXPECT_EQ(player.dashesRemaining, -1);
}

/// L'intention d'entrée par défaut est neutre : immobile, aucun saut, aucun dash.
TEST(PlayerComponentsTest, PlayerInputParDefautImmobile) {
    const core::PlayerInput input;
    EXPECT_FLOAT_EQ(input.moveX, 0.0f);
    EXPECT_FLOAT_EQ(input.moveY, 0.0f);
    EXPECT_FALSE(input.jumpPressed);
    EXPECT_FALSE(input.jumpHeld);
    EXPECT_FALSE(input.dashPressed);
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
    // Mécaniques avancées (LOT-10) : réglages non nuls et plausibles.
    EXPECT_GE(config.airJumps, 1);
    EXPECT_GT(config.wallSlideSpeed, 0.0f);
    EXPECT_GT(config.wallJumpSpeedX, 0.0f);
    EXPECT_GT(config.wallJumpSpeedY, 0.0f);
    EXPECT_GT(config.dashSpeed, 0.0f);
    EXPECT_GT(config.dashDuration, 0.0f);
    // Ressenti vertical (LOT-11) : chute renforcée (>1), flottement apex (<1), fast-fall (>1).
    EXPECT_GT(config.fallGravityMultiplier, 1.0f);
    EXPECT_GT(config.apexThreshold, 0.0f);
    EXPECT_GT(config.apexGravityMultiplier, 0.0f);
    EXPECT_LT(config.apexGravityMultiplier, 1.0f);
    EXPECT_GT(config.fastFallMultiplier, 1.0f);
}

/// La taille du personnage est humanoïde (0,4 × 0,8) et le spawn le centre dans la tuile.
TEST(PlayerComponentsTest, TailleEtSpawnHumanoide) {
    EXPECT_FLOAT_EQ(core::playerSize().x, 0.4f);
    EXPECT_FLOAT_EQ(core::playerSize().y, 0.8f);

    // Tuile (3, 5) : marges symétriques (1-0,4)/2 = 0,3 en x, (1-0,8)/2 = 0,1 en y.
    const core::Vector2 spawn = core::playerSpawnPosition(3, 5);
    EXPECT_FLOAT_EQ(spawn.x, 3.3f);
    EXPECT_FLOAT_EQ(spawn.y, 5.1f);
}

/// Les composants sont des agrégats : l'initialisation par accolades renseigne les champs.
TEST(PlayerComponentsTest, AggregationRenseigneLesChamps) {
    const core::Collider collider{core::Vector2{2.0f, 3.0f}};
    EXPECT_FLOAT_EQ(collider.size.x, 2.0f);
    EXPECT_FLOAT_EQ(collider.size.y, 3.0f);

    const core::PlayerInput input{-1.0f};
    EXPECT_FLOAT_EQ(input.moveX, -1.0f);
}
