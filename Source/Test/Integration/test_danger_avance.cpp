/**
 * @file test_danger_avance.cpp
 * @brief Tests d'intégration des dangers avancés (`EX-GP-050` à `EX-GP-053`) : `DangerController`
 * et `MechanismController` assemblés avec `CharacterPhysicsSystem`/`evaluateOutcome`, comme dans
 * `HMI::GameScreen::update`.
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/DangerController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"

namespace {

constexpr float STEP = 1.0f / 60.0f;

// Fait apparaître un personnage humanoïde (0,4x0,8) au coin (x, y) exact — pas centré dans une
// case, pour contrôler précisément son recouvrement avec la bande mortelle d'un danger
// directionnel (EX-GP-050).
core::Entity spawnHumanoidAt(core::World& world, float x, float y) {
    const core::Entity entity = world.createEntity();
    const core::Vector2 size = core::playerSize();
    world.addComponent(entity, core::Transform{core::Vector2{x, y}, size, 0.0f});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Collider{size});
    world.addComponent(entity, core::Player{});
    return entity;
}

// Assemble les boîtes actuellement mortelles des dangers mobile/commuté/temporisé, même
// composition que `HMI::GameScreen::collectActiveDangerBoxes`.
std::vector<core::Aabb> collectActiveDangerBoxes(const core::Level& level,
                                                  const core::DangerController& dangers,
                                                  const core::MechanismController& mechanisms) {
    std::vector<core::Aabb> boxes;
    for (std::size_t index = 0; index < dangers.moverCount(); ++index) {
        boxes.push_back(dangers.moverBox(index));
    }
    for (const core::DangerBlinkConfig& config : level.blinkConfigs()) {
        if (dangers.isBlinkActive(config.position)) {
            boxes.push_back(core::dangerHitbox(core::TileType::DangerBlink, config.position.column,
                                               config.position.row));
        }
    }
    for (const core::DangerLink& link : level.dangerLinks()) {
        if (mechanisms.isDangerActive(link.dangerPosition)) {
            boxes.push_back(core::dangerHitbox(core::TileType::DangerSwitched,
                                               link.dangerPosition.column,
                                               link.dangerPosition.row));
        }
    }
    return boxes;
}

}  // namespace

/**
 * @brief Un danger directionnel (`DangerRight`) n'est mortel que sur la bande de son bord droit :
 * un personnage qui ne recouvre que le reste de la case survit, qui recouvre la bande meurt
 * (`EX-GP-050`).
 * \castest{<b>Un danger directionnel n'est mortel que sur la bande de son bord désigné.</b><br/>
 * \tcat Intégration · Dangers avancés<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger directionnel n'est mortel que sur la bande de son bord désigné.
 * }
 */
TEST(DangerAvanceIntegrationTest, DangerDirectionnelMortelSeulementSurSaBande) {
    core::TileMap map(10, 4);
    for (int col = 0; col < map.width(); ++col) {
        map.setTile(col, 3, core::TileType::Solid);  // sol continu
    }
    map.setTile(5, 2, core::TileType::DangerRight);  // bande mortelle : x in [5.75, 6.0]
    const core::Level level("directionnel", std::move(map), core::GridPosition{0, 0},
                            core::GridPosition{0, 0}, {});
    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);

    const auto outcomeAt = [&](float x) {
        core::World world;
        const core::Entity player = spawnHumanoidAt(world, x, 3.0f - core::kPlayerHeight);
        core::CharacterPhysicsSystem system;
        system.update(world, mechanisms.collisionMap(), core::PlayerInput{}, STEP);  // se pose
        mechanisms.update(core::Aabb::fromTopLeftSize(
            world.getComponent<core::Transform>(player).position, core::playerSize()));
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, core::playerSize());
        return core::evaluateOutcome(box, level, collectActiveDangerBoxes(level, dangers, mechanisms));
    };

    EXPECT_EQ(outcomeAt(5.0f), core::LevelOutcome::Playing);  // avant la bande : survit
    EXPECT_EQ(outcomeAt(5.85f), core::LevelOutcome::Lost);    // sur la bande : meurt
}

/**
 * @brief Un danger mobile finit par rattraper un personnage immobile sur sa trajectoire
 * (`EX-GP-051`), sans jamais le toucher tant qu'il n'a pas atteint sa position.
 * \castest{<b>Un danger mobile finit par rattraper un personnage sur sa trajectoire.</b><br/>
 * \tcat Intégration · Dangers avancés<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger mobile finit par rattraper un personnage sur sa trajectoire.
 * }
 */
TEST(DangerAvanceIntegrationTest, DangerMobileRattrapeLePersonnageImmobile) {
    core::TileMap map(10, 4);
    for (int col = 0; col < map.width(); ++col) {
        map.setTile(col, 3, core::TileType::Solid);
    }
    map.setTile(1, 2, core::TileType::DangerMover);
    std::vector<core::DangerMoverConfig> moverConfigs{
        core::DangerMoverConfig{core::GridPosition{1, 2}, core::DangerMoverAxis::Horizontal, 4}};
    const core::Level level("mobile", std::move(map), core::GridPosition{0, 0},
                            core::GridPosition{0, 0}, {}, -1, -1, {}, std::move(moverConfigs));
    core::MechanismController mechanisms(level);
    core::DangerController dangers(level);

    core::World world;
    const core::Entity player =
        spawnHumanoidAt(world, 4.0f, 3.0f - core::kPlayerHeight);  // immobile, hors de portee
    core::CharacterPhysicsSystem system;

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < 150 && outcome == core::LevelOutcome::Playing; ++step) {
        system.update(world, mechanisms.collisionMap(), core::PlayerInput{}, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, core::playerSize());
        mechanisms.update(box);
        dangers.update();
        outcome = core::evaluateOutcome(box, level, collectActiveDangerBoxes(level, dangers, mechanisms));
        if (step == 0) {
            EXPECT_EQ(outcome, core::LevelOutcome::Playing);  // pas encore rattrape au depart
        }
    }
    EXPECT_EQ(outcome, core::LevelOutcome::Lost) << "le danger mobile aurait du rattraper le personnage";
}

/**
 * @brief Un danger commuté ne devient mortel qu'une fois son déclencheur actionné : un personnage
 * qui marche jusqu'à lui sans déclencheur lié survit, avec un déclencheur touché en chemin il
 * meurt (`EX-GP-052`).
 * \castest{<b>Un danger commuté ne devient mortel qu'une fois son déclencheur actionné.</b><br/>
 * \tcat Intégration · Dangers avancés<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger commuté ne devient mortel qu'une fois son déclencheur actionné.
 * }
 */
TEST(DangerAvanceIntegrationTest, DangerCommuteMortelSeulementApresDeclenchement) {
    const auto walkAndCheckIfEverLost = [](bool linked) {
        core::TileMap map(10, 4);
        for (int col = 0; col < map.width(); ++col) {
            map.setTile(col, 3, core::TileType::Solid);
        }
        map.setTile(3, 2, core::TileType::Switch);
        map.setTile(7, 2, core::TileType::DangerSwitched);
        std::vector<core::DangerLink> dangerLinks;
        if (linked) {
            dangerLinks.push_back(
                core::DangerLink{core::GridPosition{3, 2}, core::GridPosition{7, 2}});
        }
        const core::Level level("commute", std::move(map), core::GridPosition{0, 0},
                                core::GridPosition{0, 0}, {}, -1, -1, std::move(dangerLinks));
        core::MechanismController mechanisms(level);
        core::DangerController dangers(level);

        core::World world;
        const core::Entity player = spawnHumanoidAt(world, 0.0f, 3.0f - core::kPlayerHeight);
        core::CharacterPhysicsSystem system;
        core::PlayerInput walkRight;
        walkRight.moveX = 1.0f;

        bool sawLost = false;
        for (int step = 0; step < 200; ++step) {
            system.update(world, mechanisms.collisionMap(), walkRight, STEP);
            const core::Transform& transform = world.getComponent<core::Transform>(player);
            const core::Aabb box =
                core::Aabb::fromTopLeftSize(transform.position, core::playerSize());
            mechanisms.update(box);
            dangers.update();
            if (core::evaluateOutcome(box, level, collectActiveDangerBoxes(level, dangers, mechanisms)) ==
                core::LevelOutcome::Lost) {
                sawLost = true;
                break;
            }
        }
        return sawLost;
    };

    EXPECT_FALSE(walkAndCheckIfEverLost(false))
        << "sans liaison, le danger commute reste inerte";
    EXPECT_TRUE(walkAndCheckIfEverLost(true))
        << "l'interrupteur touche en chemin doit activer le danger lie";
}

/**
 * @brief Un danger temporisé n'est mortel que pendant sa fenêtre active du cycle ; un déphasage la
 * décale (`EX-GP-053`).
 * \castest{<b>Un danger temporisé n'est mortel que pendant sa fenêtre active.</b><br/>
 * \tcat Intégration · Dangers avancés<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un danger temporisé n'est mortel que pendant sa fenêtre active.
 * }
 */
TEST(DangerAvanceIntegrationTest, DangerTemporiseMortelSeulementPendantSaFenetreActive) {
    const auto outcomeAtSpawn = [](int phase) {
        core::TileMap map(4, 4);
        for (int col = 0; col < map.width(); ++col) {
            map.setTile(col, 3, core::TileType::Solid);
        }
        map.setTile(2, 2, core::TileType::DangerBlink);
        std::vector<core::DangerBlinkConfig> blinkConfigs{
            core::DangerBlinkConfig{core::GridPosition{2, 2}, /*period=*/60, phase,
                                    /*activeDuration=*/30}};
        const core::Level level("temporise", std::move(map), core::GridPosition{0, 0},
                                core::GridPosition{0, 0}, {}, -1, -1, {}, {},
                                std::move(blinkConfigs));
        core::MechanismController mechanisms(level);
        core::DangerController dangers(level);

        core::World world;
        // Personnage deja pose EXACTEMENT sur la case du danger temporise (case pleine, pas de
        // geometrie directionnelle a gerer ici).
        const core::Entity player = spawnHumanoidAt(
            world, 2.0f + (1.0f - core::kPlayerWidth) * 0.5f, 3.0f - core::kPlayerHeight);
        core::CharacterPhysicsSystem system;
        system.update(world, mechanisms.collisionMap(), core::PlayerInput{}, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, core::playerSize());
        mechanisms.update(box);
        return core::evaluateOutcome(box, level, collectActiveDangerBoxes(level, dangers, mechanisms));
    };

    EXPECT_EQ(outcomeAtSpawn(0), core::LevelOutcome::Lost);      // phase 0 : actif des le depart
    EXPECT_EQ(outcomeAtSpawn(15), core::LevelOutcome::Playing);  // dephase : inactif au depart
}
