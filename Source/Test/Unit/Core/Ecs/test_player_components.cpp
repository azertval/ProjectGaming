/**
 * @file test_player_components.cpp
 * @brief Tests unitaires des composants du personnage (données pures) : valeurs par défaut.
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/PlayerInput.h"

/// Un Collider par défaut a une boîte nulle : ses dimensions sont fixées au moment du spawn.
TEST(PlayerComponentsTest, ColliderParDefautEstNul) {
    const core::Collider collider;
    EXPECT_FLOAT_EQ(collider.size.x, 0.0f);
    EXPECT_FLOAT_EQ(collider.size.y, 0.0f);
}

/// Le personnage démarre en l'air (pas au sol) : la gravité s'applique dès le spawn.
TEST(PlayerComponentsTest, PlayerParDefautPasAuSol) {
    const core::Player player;
    EXPECT_FALSE(player.grounded);
}

/// L'intention d'entrée par défaut est neutre (immobile).
TEST(PlayerComponentsTest, PlayerInputParDefautImmobile) {
    const core::PlayerInput input;
    EXPECT_FLOAT_EQ(input.moveX, 0.0f);
}

/// Les composants sont des agrégats : l'initialisation par accolades renseigne les champs.
TEST(PlayerComponentsTest, AggregationRenseigneLesChamps) {
    const core::Collider collider{core::Vector2{2.0f, 3.0f}};
    EXPECT_FLOAT_EQ(collider.size.x, 2.0f);
    EXPECT_FLOAT_EQ(collider.size.y, 3.0f);

    const core::PlayerInput input{-1.0f};
    EXPECT_FLOAT_EQ(input.moveX, -1.0f);
}
