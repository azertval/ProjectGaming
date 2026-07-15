/**
 * @file test_movement_system.cpp
 * @brief Tests unitaires du système de mouvement (démo de bout en bout de l'ECS).
 */

#include <memory>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/MovementSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"

namespace {
constexpr float STEP = 1.0f / 60.0f;
constexpr float TOLERANCE = 1e-4f;
}  // namespace

/// Une entité Transform + Velocity avance de `velocity * fixedDelta` par pas ;
/// après N pas, la position attendue est déterministe.
TEST(MovementSystemTest, EntiteMobileAvanceDeVitesseFoisPas) {
    core::World world;
    const core::Entity mover = world.createEntity();
    world.addComponent(mover, core::Transform{});
    world.addComponent(mover, core::Velocity{core::Vector2{2.0f, -3.0f}});

    core::MovementSystem system;
    const int steps = 10;
    for (int i = 0; i < steps; ++i) {
        system.update(world, STEP);
    }

    // position = velocity * (steps * STEP)
    const core::Vector2& position = world.getComponent<core::Transform>(mover).position;
    EXPECT_NEAR(position.x, 2.0f * steps * STEP, TOLERANCE);
    EXPECT_NEAR(position.y, -3.0f * steps * STEP, TOLERANCE);
}

/// Une entité sans Velocity ne bouge pas.
TEST(MovementSystemTest, EntiteSansVelociteNeBougePas) {
    core::World world;
    const core::Entity fixture = world.createEntity();
    world.addComponent(fixture,
                       core::Transform{core::Vector2{5.0f, 5.0f}, core::Vector2{1.0f, 1.0f}, 0.0f});

    core::MovementSystem system;
    for (int i = 0; i < 100; ++i) {
        system.update(world, STEP);
    }

    const core::Vector2& position = world.getComponent<core::Transform>(fixture).position;
    EXPECT_NEAR(position.x, 5.0f, TOLERANCE);
    EXPECT_NEAR(position.y, 5.0f, TOLERANCE);
}

/// Deux entités de vitesses différentes évoluent indépendamment.
TEST(MovementSystemTest, DeuxEntitesEvoluentIndependamment) {
    core::World world;
    const core::Entity slow = world.createEntity();
    const core::Entity fast = world.createEntity();
    world.addComponent(slow, core::Transform{});
    world.addComponent(slow, core::Velocity{core::Vector2{1.0f, 0.0f}});
    world.addComponent(fast, core::Transform{});
    world.addComponent(fast, core::Velocity{core::Vector2{0.0f, 4.0f}});

    core::MovementSystem system;
    const int steps = 60;  // 1 seconde à 60 Hz
    for (int i = 0; i < steps; ++i) {
        system.update(world, STEP);
    }

    const core::Vector2& slowPosition = world.getComponent<core::Transform>(slow).position;
    const core::Vector2& fastPosition = world.getComponent<core::Transform>(fast).position;
    EXPECT_NEAR(slowPosition.x, 1.0f, TOLERANCE);
    EXPECT_NEAR(slowPosition.y, 0.0f, TOLERANCE);
    EXPECT_NEAR(fastPosition.x, 0.0f, TOLERANCE);
    EXPECT_NEAR(fastPosition.y, 4.0f, TOLERANCE);
}

/// Enregistré dans le World, le système s'exécute via World::update (chaîne complète).
TEST(MovementSystemTest, IntegrationViaWorldUpdate) {
    core::World world;
    const core::Entity mover = world.createEntity();
    world.addComponent(mover, core::Transform{});
    world.addComponent(mover, core::Velocity{core::Vector2{10.0f, 0.0f}});
    world.addSystem(std::make_unique<core::MovementSystem>());

    const int steps = 30;
    for (int i = 0; i < steps; ++i) {
        world.update(STEP);
    }

    const core::Vector2& position = world.getComponent<core::Transform>(mover).position;
    EXPECT_NEAR(position.x, 10.0f * steps * STEP, TOLERANCE);
}
