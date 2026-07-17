/**
 * @file test_physique_personnage.cpp
 * @brief Tests d'intégration de la physique du personnage : ECS + grille + balayage assemblés.
 *
 * Le `CharacterPhysicsSystem` combine composants (`Player`/`Transform`/`Velocity`/`Collider`),
 * grille de collision (`TileMap`) et résolution continue (`sweepAabb`). On vérifie de bout en bout
 * la gravité, l'atterrissage, le blocage, la vitesse constante, le non-tunneling et le déterminisme.
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PhysicsConfig.h"
#include "Core/Physics/PlayerInput.h"

namespace {

constexpr float STEP = 1.0f / 60.0f;
constexpr float TOLERANCE = 1e-4f;

// Fait apparaître un personnage 1×1 (Player + Transform + Velocity + Collider) au coin (x, y).
core::Entity spawnPlayer(core::World& world, float x, float y) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{x, y}, core::Vector2{1.0f, 1.0f}, 0.0f});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Collider{core::Vector2{1.0f, 1.0f}});
    world.addComponent(entity, core::Player{});
    return entity;
}

// Remplit une ligne entière de tuiles solides (sol/plafond continu).
void fillRow(core::TileMap& tiles, int row) {
    for (int col = 0; col < tiles.width(); ++col) {
        tiles.setTile(col, row, core::TileType::Solid);
    }
}

}  // namespace

/// Sans sol, le personnage tombe et sa vitesse verticale croît (gravité continue).
TEST(PhysiquePersonnageIntegration, TombeSousGraviteVitesseCroissante) {
    core::World world;
    core::TileMap tiles(4, 100);  // aucune tuile solide : chute libre
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{};

    system.update(world, tiles, input, STEP);
    const float vy1 = world.getComponent<core::Velocity>(player).value.y;
    const float y1 = world.getComponent<core::Transform>(player).position.y;
    system.update(world, tiles, input, STEP);
    const float vy2 = world.getComponent<core::Velocity>(player).value.y;
    const float y2 = world.getComponent<core::Transform>(player).position.y;

    EXPECT_GT(vy1, 0.0f);   // y vers le bas : tomber = vitesse positive
    EXPECT_GT(vy2, vy1);    // la gravité continue d'accélérer
    EXPECT_GT(y2, y1);      // le personnage descend
    EXPECT_FALSE(world.getComponent<core::Player>(player).grounded);
}

/// Le personnage se pose sur le sol : vitesse verticale annulée, état « au sol » vrai.
TEST(PhysiquePersonnageIntegration, AtterritSurLeSolEtEstAuSol) {
    core::World world;
    core::TileMap tiles(4, 10);
    fillRow(tiles, 6);  // sol sur la ligne 6
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{};

    for (int i = 0; i < 120; ++i) {  // ~2 s : largement le temps d'atterrir
        system.update(world, tiles, input, STEP);
    }

    const core::Transform& transform = world.getComponent<core::Transform>(player);
    const core::Velocity& velocity = world.getComponent<core::Velocity>(player);
    EXPECT_NEAR(transform.position.y, 5.0f, TOLERANCE);  // bord bas = 6.0 (haut du sol)
    EXPECT_NEAR(velocity.value.y, 0.0f, TOLERANCE);
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
}

/// Poussé contre un mur, le personnage s'arrête au ras du mur (blocage horizontal).
TEST(PhysiquePersonnageIntegration, BloqueParUnMurADroite) {
    core::World world;
    core::TileMap tiles(10, 3);
    fillRow(tiles, 2);                             // sol
    tiles.setTile(4, 0, core::TileType::Solid);    // mur vertical (colonne 4)
    tiles.setTile(4, 1, core::TileType::Solid);
    const core::Entity player = spawnPlayer(world, 0.0f, 1.0f);
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{1.0f};  // pousse à droite en continu

    for (int i = 0; i < 120; ++i) {
        system.update(world, tiles, input, STEP);
    }

    const core::Transform& transform = world.getComponent<core::Transform>(player);
    EXPECT_NEAR(transform.position.x, 3.0f, TOLERANCE);  // bord droit = 4.0 = bord du mur
}

/// Sur terrain libre, l'avancée horizontale vaut vitesse × temps (vitesse constante).
TEST(PhysiquePersonnageIntegration, AvanceAVitesseConstante) {
    core::World world;
    core::TileMap tiles(100, 3);
    fillRow(tiles, 2);  // sol continu
    const core::Entity player = spawnPlayer(world, 0.0f, 1.0f);
    const core::PhysicsConfig config;  // défauts (moveSpeed)
    core::CharacterPhysicsSystem system(config);
    const core::PlayerInput input{1.0f};

    const float startX = world.getComponent<core::Transform>(player).position.x;
    const int steps = 30;
    for (int i = 0; i < steps; ++i) {
        system.update(world, tiles, input, STEP);
    }
    const float endX = world.getComponent<core::Transform>(player).position.x;

    EXPECT_NEAR(endX - startX, config.moveSpeed * steps * STEP, 1e-3f);
}

/// Même en chute très rapide (grand pas), le personnage ne traverse pas le sol (balayage continu).
TEST(PhysiquePersonnageIntegration, NeTraversePasLeSolEnChuteRapide) {
    core::World world;
    core::TileMap tiles(4, 60);
    fillRow(tiles, 50);  // sol loin en bas
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::PhysicsConfig fast;
    fast.gravity = 2000.0f;        // accélération énorme
    fast.maxFallSpeed = 1.0e6f;    // pas de borne : le pas dépasse une tuile
    core::CharacterPhysicsSystem system(fast);
    const core::PlayerInput input{};

    for (int i = 0; i < 300; ++i) {
        system.update(world, tiles, input, STEP);
    }

    const core::Transform& transform = world.getComponent<core::Transform>(player);
    EXPECT_NEAR(transform.position.y, 49.0f, TOLERANCE);  // posé sur le sol, non traversé
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
}

/// Mêmes entrées → même résultat : la simulation est déterministe (pas fixe).
TEST(PhysiquePersonnageIntegration, Deterministe) {
    const auto run = []() {
        core::World world;
        core::TileMap tiles(10, 10);
        fillRow(tiles, 8);
        const core::Entity player = spawnPlayer(world, 0.0f, 7.0f);
        core::CharacterPhysicsSystem system;
        const core::PlayerInput input{1.0f};
        for (int i = 0; i < 200; ++i) {
            system.update(world, tiles, input, STEP);
        }
        return world.getComponent<core::Transform>(player).position;
    };

    const core::Vector2 first = run();
    const core::Vector2 second = run();
    EXPECT_FLOAT_EQ(first.x, second.x);
    EXPECT_FLOAT_EQ(first.y, second.y);
}

/// Le niveau de démonstration livré est **franchissable** sans saut : en maintenant « droite »,
/// le personnage descend l'escalier de paliers et atteint la sortie sans jamais mourir.
TEST(PhysiquePersonnageIntegration, NiveauDemoEstFranchissableEnAllantADroite) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo.json";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player =
        spawnPlayer(world, static_cast<float>(level.entry().column),
                    static_cast<float>(level.entry().row));
    core::CharacterPhysicsSystem system;
    const core::PlayerInput goRight{1.0f};

    // On simule jusqu'à l'issue (borne large pour éviter une boucle infinie si le niveau régresse).
    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < 3000 && outcome == core::LevelOutcome::Playing; ++step) {
        system.update(world, level.tileMap(), goRight, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        outcome = core::evaluateOutcome(
            core::Aabb::fromTopLeftSize(transform.position, collider.size), level);
    }

    EXPECT_EQ(outcome, core::LevelOutcome::Won);  // franchi, jamais Lost
}
