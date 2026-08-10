/**
 * @file test_bloc_reduit.cpp
 * @brief Tests d'intégration des blocs à taille réduite (`TileType::BlockHalf`/`BlockQuarter`,
 *        `EX-GP-005`) : composition du balayage sur grille (`CharacterPhysicsSystem`) et du
 *        balayage boîte-boîte (`core::sweepAabbVsAabb`), comme orchestré par `hmi::GameSession`.
 *
 * Ce fichier rejoue **hors HMI** (pas de périphérique Direct3D requis, cf. `test_parcours_complet`
 * pour le même principe côté mécanismes) l'orchestration exacte de `GameSession::update` : blocs
 * puis grille puis composition boîte-boîte — la ligne de risque propre à ce lot (`LOT-24`).
 */

#include <cmath>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/BlockController.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PlayerInput.h"

namespace {

constexpr float STEP = 1.0f / 60.0f;

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

// Rejoue EXACTEMENT l'orchestration de `hmi::GameSession::update` pour un niveau SANS mécanisme
// interrupteur/porte (une `TileMap` statique suffit alors comme grille de base) : blocs (poussée
// puis chute), grille (blocs pleins + murs), puis composition boîte-boîte pour les blocs réduits
// (la restriction la plus stricte des deux l'emporte, jamais l'inverse — voir `AabbVsAabb.h`).
void simulateStep(core::World& world, core::Entity player, const core::TileMap& baseTiles,
                  core::BlockController& blocks, const core::PlayerInput& input,
                  core::CharacterPhysicsSystem& physics) {
    const core::Transform& previousTransform = world.getComponent<core::Transform>(player);
    const core::Collider& collider = world.getComponent<core::Collider>(player);
    const core::Aabb previousBox =
        core::Aabb::fromTopLeftSize(previousTransform.position, collider.size);

    blocks.update(previousBox, input.moveX, baseTiles);
    const core::TileMap collision = blocks.collisionMap(baseTiles);
    physics.update(world, collision, input, STEP);

    core::Transform& transform = world.getComponent<core::Transform>(player);
    const core::Vector2 delta = transform.position - previousBox.min;
    if (delta.x == 0.0f && delta.y == 0.0f) {
        return;
    }
    core::Vector2 bestPosition = transform.position;
    core::Vector2 bestNormal{};
    const std::vector<float>& scales = blocks.scales();
    for (std::size_t index = 0; index < scales.size(); ++index) {
        if (scales[index] >= 1.0f) {
            continue;
        }
        const core::SweepResult result =
            core::sweepAabbVsAabb(previousBox, delta, blocks.boxAt(index));
        if (result.normal.x != 0.0f && std::fabs(result.position.x - previousBox.min.x) <
                                           std::fabs(bestPosition.x - previousBox.min.x)) {
            bestPosition.x = result.position.x;
            bestNormal.x = result.normal.x;
        }
        if (result.normal.y != 0.0f && std::fabs(result.position.y - previousBox.min.y) <
                                           std::fabs(bestPosition.y - previousBox.min.y)) {
            bestPosition.y = result.position.y;
            bestNormal.y = result.normal.y;
        }
    }
    if (bestNormal.x == 0.0f && bestNormal.y == 0.0f) {
        return;
    }
    transform.position = bestPosition;
    core::Velocity& velocity = world.getComponent<core::Velocity>(player);
    core::Player& playerComponent = world.getComponent<core::Player>(player);
    if (bestNormal.x != 0.0f) {
        velocity.value.x = 0.0f;
    }
    if (bestNormal.y != 0.0f) {
        velocity.value.y = 0.0f;
        if (bestNormal.y < 0.0f) {
            playerComponent.grounded = true;
        }
    }
}

}  // namespace

/**
 * @brief Un bloc réduit (`BlockHalf`) coincé contre un mur arrête le personnage exactement au bord
 * de sa boîte réelle — pas au bord de la case entière (`EX-GP-005`).
 * \castest{<b>Un bloc réduit coincé contre un mur arrête le personnage au bord de sa boîte réelle,
 * pas de la case entière.</b><br/>
 * \tcat Integration · Bloc Réduit<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage s'arrête au bord de la boîte réduite (2,25), pas au bord de la case
 * (3,0) — la marge vide de la case n'est jamais traversée à tort comme un mur classique le
 * ferait, mais n'est pas non plus ignorée.
 * }
 */
TEST(BlocReduitIntegration, ArreteAuBordDeSaBoiteReelle) {
    core::TileMap tiles(10, 4);
    for (int col = 0; col < 10; ++col) {
        tiles.setTile(col, 3, core::TileType::Solid);  // sol
    }
    tiles.setTile(2, 2, core::TileType::BlockHalf);  // coince contre le mur, ne se pousse pas
    tiles.setTile(3, 2, core::TileType::Solid);      // mur derriere le bloc : la poussee echoue
    core::Level level("bloc-reduit", tiles, core::GridPosition{0, 2}, core::GridPosition{9, 2},
                      std::vector<core::Mechanism>{});
    core::BlockController blocks(level);
    core::World world;
    const core::Entity player = spawnPlayer(world, 0.0f, 2.0f);
    core::CharacterPhysicsSystem physics;
    const core::PlayerInput input{1.0f};

    for (int i = 0; i < 300; ++i) {
        simulateStep(world, player, tiles, blocks, input, physics);
    }

    // Le bloc n'a pas bouge (case suivante occupee par un mur) : sa boite reelle reste centree en
    // (2,2), soit x in [2.25, 2.75]. Le personnage (1 unite de large) doit s'arreter bord droit =
    // 2.25 (position.x = 1.25), PAS au bord de la case (position.x = 1.0, bord droit = 2.0) ni au
    // travers (position.x > 1.25).
    EXPECT_EQ(blocks.positions()[0], (core::GridPosition{2, 2}));  // bloc immobile (mur derriere)
    EXPECT_NEAR(world.getComponent<core::Transform>(player).position.x, 1.25f, 1e-3f);
}

/**
 * @brief L'espace laissé libre autour d'un bloc réduit reste franchissable : un personnage qui
 * passe au-dessus (hors de la bande verticale occupée par la boîte réelle) n'est jamais bloqué par
 * la case entière (`EX-GP-005`).
 * \castest{<b>L'espace autour d'un bloc réduit, hors de sa boîte réelle, reste
 * franchissable.</b><br/>
 * \tcat Integration · Bloc Réduit<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage franchit la colonne du bloc réduit sans ralentir ni s'arrêter : la case
 * entière n'est jamais traitée comme un mur (seule sa boîte réelle l'aurait été).
 * }
 */
TEST(BlocReduitIntegration, EspaceAutourResteFranchissable) {
    core::TileMap tiles(10, 6);
    for (int col = 0; col < 10; ++col) {
        tiles.setTile(col, 5, core::TileType::Solid);  // sol, loin sous le personnage
    }
    tiles.setTile(4, 3, core::TileType::BlockQuarter);  // pose au sol, tres bas dans sa case
    core::Level level("bloc-reduit-libre", tiles, core::GridPosition{0, 0},
                      core::GridPosition{9, 0}, std::vector<core::Mechanism>{});
    core::BlockController blocks(level);
    core::World world;
    // Personnage haut dans la colonne du bloc (loin de sa boite reelle centree en y~3.5-3.75) :
    // ne devrait JAMAIS entrer en contact avec le bloc quarter, ni etre freine par sa case.
    const core::Entity player = spawnPlayer(world, 0.0f, 0.0f);
    core::CharacterPhysicsSystem physics;
    const core::PlayerInput input{1.0f};

    const float startX = world.getComponent<core::Transform>(player).position.x;
    for (int i = 0; i < 180;
         ++i) {  // 3 s : largement de quoi traverser toute la largeur (10 unites/3 vitesse)
        simulateStep(world, player, tiles, blocks, input, physics);
    }
    const float endX = world.getComponent<core::Transform>(player).position.x;

    // Vitesse constante par defaut (moveSpeed = 3 u/s), 180 pas = 3s -> 9 unites parcourues sans
    // AUCUN ralentissement : si la case du bloc reduit avait ete traitee comme un mur, le
    // personnage se serait arrete bien avant (colonne 4).
    EXPECT_GT(endX - startX, 8.0f);
}
