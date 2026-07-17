/**
 * @file test_physique_personnage.cpp
 * @brief Tests d'intégration de la physique du personnage : ECS + grille + balayage assemblés.
 *
 * Le `CharacterPhysicsSystem` combine composants (`Player`/`Transform`/`Velocity`/`Collider`),
 * grille de collision (`TileMap`) et résolution continue (`sweepAabb`). On vérifie de bout en bout
 * la gravité, l'atterrissage, le blocage, la vitesse constante, le non-tunneling et le
 * déterminisme.
 */

#include <algorithm>
#include <filesystem>
#include <functional>

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

// Mesure la hauteur (en tuiles) atteinte par un saut où le bouton est maintenu `holdFrames` pas.
// Le personnage est d'abord posé au sol, puis saute (front à la frame 0), et on relève l'apogée.
float jumpApexHeight(int holdFrames) {
    core::World world;
    core::TileMap tiles(4, 40);
    fillRow(tiles, 30);  // sol bas, beaucoup d'espace au-dessus
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;

    const core::PlayerInput idle{};
    for (int i = 0; i < 400; ++i) {  // se poser au sol
        system.update(world, tiles, idle, STEP);
    }
    const float groundY = world.getComponent<core::Transform>(player).position.y;

    float minY = groundY;  // y décroît vers le haut → apogée = min
    for (int i = 0; i < 250; ++i) {
        core::PlayerInput in;
        in.jumpPressed = (i == 0);       // front à la première frame
        in.jumpHeld = (i < holdFrames);  // maintien pendant `holdFrames` pas
        system.update(world, tiles, in, STEP);
        minY = std::min(minY, world.getComponent<core::Transform>(player).position.y);
    }
    return groundY - minY;  // hauteur en unités monde (tuiles)
}

// Marche à droite jusqu'à quitter un rebord, attend `delayFrames`, puis presse le saut.
// Renvoie true si le saut a décollé (vitesse ascendante) — teste le *coyote time*.
bool jumpAfterLeavingLedge(int delayFrames) {
    core::World world;
    core::TileMap tiles(20, 20);
    for (int col = 0; col <= 3; ++col) {  // rebord : sol sur les colonnes 0..3
        tiles.setTile(col, 12, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 1.0f, 10.0f);
    core::PhysicsConfig config;
    config.airJumps = 0;  // isole le coyote time (pas de saut aérien pour masquer l'expiration)
    core::CharacterPhysicsSystem system(config);

    for (int i = 0; i < 200; ++i) {  // se poser sur le rebord
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    const core::PlayerInput right{1.0f};
    int guard = 0;
    while (world.getComponent<core::Player>(player).grounded && guard < 600) {
        system.update(world, tiles, right, STEP);  // marcher jusqu'à quitter le sol
        ++guard;
    }
    for (int i = 0; i < delayFrames; ++i) {
        system.update(world, tiles, right, STEP);  // attendre en l'air
    }
    core::PlayerInput jump{1.0f};
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);
    return world.getComponent<core::Velocity>(player).value.y < 0.0f;  // a décollé ?
}

// Presse le saut `framesBeforeLanding` avant l'atterrissage ; renvoie true si un saut part une fois
// posé — teste le *jump buffering*. Deux passages : trouver l'atterrissage, puis presser au moment.
bool bufferedJump(int framesBeforeLanding) {
    int landingFrame = -1;
    {
        core::World world;
        core::TileMap tiles(4, 20);
        fillRow(tiles, 15);
        const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
        core::CharacterPhysicsSystem system;
        for (int i = 0; i < 200 && landingFrame < 0; ++i) {
            system.update(world, tiles, core::PlayerInput{}, STEP);
            if (world.getComponent<core::Player>(player).grounded) {
                landingFrame = i;
            }
        }
    }
    if (landingFrame < 0) {
        return false;
    }
    const int pressFrame = landingFrame - framesBeforeLanding;

    core::World world;
    core::TileMap tiles(4, 20);
    fillRow(tiles, 15);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    bool landed = false;
    bool jumped = false;
    for (int i = 0; i < 200; ++i) {
        core::PlayerInput in;
        in.jumpPressed = (i == pressFrame);
        system.update(world, tiles, in, STEP);
        if (world.getComponent<core::Player>(player).grounded) {
            landed = true;
        }
        // Après avoir touché le sol, une vitesse ascendante trahit un saut (bufferisé).
        if (landed && world.getComponent<core::Velocity>(player).value.y < -1.0f) {
            jumped = true;
        }
    }
    return jumped;
}

// Compte combien de sauts « prennent » (impulsion ascendante nette) : un premier au sol, puis des
// sauts répétés en l'air. Attendu = 1 (sol) + `airJumpsConfig` (aériens).
int successfulJumps(int airJumpsConfig) {
    core::PhysicsConfig config;
    config.airJumps = airJumpsConfig;
    core::World world;
    core::TileMap tiles(4, 200);
    fillRow(tiles, 150);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system(config);
    for (int i = 0; i < 1500; ++i) {  // se poser
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }

    int jumps = 0;
    for (int attempt = 0; attempt < airJumpsConfig + 3; ++attempt) {
        core::PlayerInput jump;
        jump.jumpPressed = true;
        jump.jumpHeld = true;
        const float before = world.getComponent<core::Velocity>(player).value.y;
        system.update(world, tiles, jump, STEP);
        if (world.getComponent<core::Velocity>(player).value.y < before - 5.0f) {
            ++jumps;  // l'impulsion a « pris »
        }
        for (int i = 0; i < 8; ++i) {  // rester en l'air avant la prochaine tentative
            system.update(world, tiles, core::PlayerInput{}, STEP);
        }
    }
    return jumps;
}

// Rejoue un niveau livré avec un scénario d'entrées (fonction du pas) et renvoie son issue.
core::LevelOutcome playLevelFile(const char* file,
                                 const std::function<core::PlayerInput(int)>& input) {
    const std::filesystem::path path = std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        return core::LevelOutcome::Lost;
    }
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawnPlayer(world, static_cast<float>(level.entry().column),
                                            static_cast<float>(level.entry().row));
    core::CharacterPhysicsSystem system;

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < 3000 && outcome == core::LevelOutcome::Playing; ++step) {
        system.update(world, level.tileMap(), input(step), STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        outcome = core::evaluateOutcome(
            core::Aabb::fromTopLeftSize(transform.position, collider.size), level);
    }
    return outcome;
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

    EXPECT_GT(vy1, 0.0f);  // y vers le bas : tomber = vitesse positive
    EXPECT_GT(vy2, vy1);   // la gravité continue d'accélérer
    EXPECT_GT(y2, y1);     // le personnage descend
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
    fillRow(tiles, 2);                           // sol
    tiles.setTile(4, 0, core::TileType::Solid);  // mur vertical (colonne 4)
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
    fast.gravity = 2000.0f;      // accélération énorme
    fast.maxFallSpeed = 1.0e6f;  // pas de borne : le pas dépasse une tuile
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

/// Au sol, une pression de saut fait décoller le personnage (vitesse ascendante, il s'élève).
TEST(PhysiquePersonnageIntegration, SauteDepuisLeSol) {
    core::World world;
    core::TileMap tiles(4, 20);
    fillRow(tiles, 15);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;

    const core::PlayerInput idle{};
    for (int i = 0; i < 300; ++i) {
        system.update(world, tiles, idle, STEP);
    }
    ASSERT_TRUE(world.getComponent<core::Player>(player).grounded);
    const float groundY = world.getComponent<core::Transform>(player).position.y;

    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);
    EXPECT_LT(world.getComponent<core::Velocity>(player).value.y, 0.0f);  // monte (y négatif)

    core::PlayerInput hold;
    hold.jumpHeld = true;
    for (int i = 0; i < 5; ++i) {
        system.update(world, tiles, hold, STEP);
    }
    EXPECT_LT(world.getComponent<core::Transform>(player).position.y, groundY);  // au-dessus du sol
}

/// En l'air, une pression de saut n'a aucun effet : pas de double saut (`EX-GP-013`).
TEST(PhysiquePersonnageIntegration, PasDeSautEnLAir) {
    core::World world;
    core::TileMap tiles(4, 50);  // pas de sol à portée : le personnage tombe
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;

    const core::PlayerInput idle{};
    system.update(world, tiles, idle, STEP);
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);

    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);
    EXPECT_GT(world.getComponent<core::Velocity>(player).value.y,
              0.0f);  // tombe encore, pas de saut
}

/// Hauteur de saut variable : maintenir le bouton saute plus haut que le relâcher tôt.
TEST(PhysiquePersonnageIntegration, HauteurDeSautVariable) {
    const float fullJump = jumpApexHeight(1000);  // maintenu toute la montée
    const float shortHop = jumpApexHeight(1);     // relâché immédiatement

    EXPECT_GT(fullJump, shortHop + 0.3f);  // maintenir monte nettement plus haut
    EXPECT_GT(fullJump, 2.0f);             // ~2,25 tuiles avec le réglage par défaut
    EXPECT_LT(fullJump, 3.0f);
}

/// Double saut : 1 saut au sol + N sauts aériens (paramétrable), rechargés au contact du sol.
TEST(PhysiquePersonnageIntegration, DoubleSautNombreParametrable) {
    EXPECT_EQ(successfulJumps(1), 2);  // 1 au sol + 1 aérien (double saut)
    EXPECT_EQ(successfulJumps(2), 3);  // 1 au sol + 2 aériens
    EXPECT_EQ(successfulJumps(0), 1);  // aucun saut aérien : 1 seul
}

/// Dash horizontal : une ruée rapide (≫ vitesse normale) fait parcourir une grande distance.
TEST(PhysiquePersonnageIntegration, DashHorizontalRapide) {
    core::World world;
    core::TileMap tiles(100, 5);
    fillRow(tiles, 3);
    const core::Entity player = spawnPlayer(world, 1.0f, 2.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 60; ++i) {  // se poser (dash rechargé au sol)
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    const float startX = world.getComponent<core::Transform>(player).position.x;

    core::PlayerInput dash;
    dash.dashPressed = true;  // direction par défaut = orientation (droite)
    system.update(world, tiles, dash, STEP);
    const core::PhysicsConfig config;
    EXPECT_GT(world.getComponent<core::Velocity>(player).value.x, config.moveSpeed * 2.0f);

    for (int i = 0; i < 12; ++i) {
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    EXPECT_GT(world.getComponent<core::Transform>(player).position.x - startX, 3.0f);
}

/// Dash diagonal (8 directions) : viser haut-droite envoie en +X et −Y.
TEST(PhysiquePersonnageIntegration, DashDiagonalHautDroite) {
    core::World world;
    core::TileMap tiles(20, 20);
    fillRow(tiles, 15);
    const core::Entity player = spawnPlayer(world, 2.0f, 14.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 60; ++i) {
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    core::PlayerInput dash;
    dash.dashPressed = true;
    dash.moveX = 1.0f;
    dash.moveY = -1.0f;  // haut-droite
    system.update(world, tiles, dash, STEP);

    const core::Velocity& velocity = world.getComponent<core::Velocity>(player);
    EXPECT_GT(velocity.value.x, 0.0f);  // droite
    EXPECT_LT(velocity.value.y, 0.0f);  // haut
}

/// Pendant un dash horizontal, la gravité est suspendue (la composante verticale reste nulle).
TEST(PhysiquePersonnageIntegration, GraviteSuspenduePendantDash) {
    core::World world;
    core::TileMap tiles(100, 5);
    fillRow(tiles, 3);
    const core::Entity player = spawnPlayer(world, 1.0f, 2.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 60; ++i) {
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    core::PlayerInput dash;
    dash.dashPressed = true;
    system.update(world, tiles, dash, STEP);
    for (int i = 0; i < 5; ++i) {  // pendant la durée du dash
        system.update(world, tiles, core::PlayerInput{}, STEP);
        EXPECT_NEAR(world.getComponent<core::Velocity>(player).value.y, 0.0f, 0.1f);
    }
}

/// Une seule ruée par phase aérienne : le second dash en l'air est refusé jusqu'au retour au sol.
TEST(PhysiquePersonnageIntegration, DashUneSeuleFoisEnLAir) {
    core::World world;
    core::TileMap tiles(4, 100);
    fillRow(tiles, 90);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 800; ++i) {  // se poser (dash + saut rechargés)
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);  // sauter (dash reste chargé)
    for (int i = 0; i < 3; ++i) {
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);

    core::PlayerInput dash;
    dash.dashPressed = true;
    dash.moveX = 1.0f;
    dash.moveY = -1.0f;  // dash haut-droite : gagne de la hauteur (reste en l'air)
    system.update(world, tiles, dash, STEP);  // 1er dash en l'air
    EXPECT_GT(world.getComponent<core::Velocity>(player).value.x, 10.0f);
    for (int i = 0; i < 20; ++i) {  // fin du dash, toujours en l'air
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    ASSERT_FALSE(world.getComponent<core::Player>(player).dashAvailable);
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);

    core::PlayerInput dash2;
    dash2.dashPressed = true;
    dash2.moveX = 1.0f;
    system.update(world, tiles, dash2, STEP);                              // 2e dash refusé
    EXPECT_LT(world.getComponent<core::Velocity>(player).value.x, 20.0f);  // pas de ruée (30)
}

/// Un dash vers un mur ne le traverse pas (résolu par le balayage).
TEST(PhysiquePersonnageIntegration, DashNeTraversePasLeMur) {
    core::World world;
    core::TileMap tiles(10, 5);
    fillRow(tiles, 3);
    tiles.setTile(5, 2, core::TileType::Solid);  // mur sur la trajectoire (ligne du personnage)
    const core::Entity player = spawnPlayer(world, 1.0f, 2.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 60; ++i) {
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    for (int i = 0; i < 12; ++i) {
        core::PlayerInput in;
        in.dashPressed = (i == 0);  // dash droite au premier pas
        system.update(world, tiles, in, STEP);
    }
    EXPECT_LE(world.getComponent<core::Transform>(player).position.x, 4.0f + 0.01f);  // bord ≤ mur
}

/// Wall slide : collé à un mur en l'air, la vitesse de chute est plafonnée (descente ralentie).
TEST(PhysiquePersonnageIntegration, WallSlideRalentitLaChute) {
    core::World world;
    core::TileMap tiles(8, 20);
    for (int row = 0; row < 20; ++row) {  // mur vertical colonne 5
        tiles.setTile(5, row, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 4.0f, 2.0f);  // à gauche du mur, en l'air
    core::CharacterPhysicsSystem system;
    const core::PlayerInput pushRight{1.0f};  // pousse vers le mur
    for (int i = 0; i < 30; ++i) {
        system.update(world, tiles, pushRight, STEP);
    }

    const core::PhysicsConfig config;
    const core::Velocity& velocity = world.getComponent<core::Velocity>(player);
    EXPECT_NE(world.getComponent<core::Player>(player).wallDirection, 0.0f);  // collé au mur
    EXPECT_GT(velocity.value.y, 0.0f);                                        // descend
    EXPECT_LE(velocity.value.y, config.wallSlideSpeed + 0.5f);  // ralenti (≪ chute libre ~25)
}

/// Wall jump : contre un mur à droite, un saut éjecte vers la gauche et le haut.
TEST(PhysiquePersonnageIntegration, WallJumpEjecteAlOpposeDuMur) {
    core::World world;
    core::TileMap tiles(8, 20);
    for (int row = 0; row < 20; ++row) {
        tiles.setTile(5, row, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 4.0f, 2.0f);
    core::CharacterPhysicsSystem system;
    const core::PlayerInput pushRight{1.0f};
    for (int i = 0; i < 20; ++i) {  // se coller au mur
        system.update(world, tiles, pushRight, STEP);
    }
    ASSERT_NE(world.getComponent<core::Player>(player).wallDirection, 0.0f);

    core::PlayerInput wallJump{1.0f};  // saut en poussant encore vers le mur
    wallJump.jumpPressed = true;
    wallJump.jumpHeld = true;
    system.update(world, tiles, wallJump, STEP);

    const core::Velocity& velocity = world.getComponent<core::Velocity>(player);
    EXPECT_LT(velocity.value.x, 0.0f);  // éjecté à gauche (opposé au mur), malgré moveX = +1
    EXPECT_LT(velocity.value.y, 0.0f);  // et vers le haut
}

/// Sans mur, la logique de wall jump ne s'active pas (et sans saut aérien, aucun saut en l'air).
TEST(PhysiquePersonnageIntegration, PasDeWallJumpSansMur) {
    core::World world;
    core::TileMap tiles(8, 50);  // ni mur ni sol à portée
    const core::Entity player = spawnPlayer(world, 4.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    system.update(world, tiles, core::PlayerInput{}, STEP);  // commence à tomber
    ASSERT_FLOAT_EQ(world.getComponent<core::Player>(player).wallDirection, 0.0f);

    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    const float before = world.getComponent<core::Velocity>(player).value.y;
    system.update(world, tiles, jump, STEP);
    const core::Velocity& velocity = world.getComponent<core::Velocity>(player);
    EXPECT_FLOAT_EQ(velocity.value.x, 0.0f);  // aucune éjection horizontale
    EXPECT_GT(velocity.value.y, before);      // continue de tomber (aucun saut disponible)
}

/// Le niveau 2 est franchissable **avec le saut** : en avançant et sautant, on atteint la sortie.
TEST(PhysiquePersonnageIntegration, Niveau2FranchissableAvecSaut) {
    const auto rightAndJump = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        in.jumpPressed = true;  // saut maintenu/répété : franchit la marche ascendante
        in.jumpHeld = true;
        return in;
    };
    EXPECT_EQ(playLevelFile("demo2.json", rightAndJump), core::LevelOutcome::Won);
}

/// Le niveau 2 **exige** le saut : en avançant seulement (sans sauter), la marche bloque.
TEST(PhysiquePersonnageIntegration, Niveau2RequiertLeSaut) {
    const auto rightOnly = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        return in;
    };
    EXPECT_NE(playLevelFile("demo2.json", rightOnly),
              core::LevelOutcome::Won);  // bloqué à la marche
}

/// Coyote time : sauter juste après avoir quitté un bord fonctionne ; trop tard, non.
TEST(PhysiquePersonnageIntegration, CoyoteTimeAutoriseUnSautJusteApresLeBord) {
    EXPECT_TRUE(jumpAfterLeavingLedge(0));    // à peine quitté le sol → saut permis
    EXPECT_FALSE(jumpAfterLeavingLedge(15));  // trop tard → fenêtre coyote expirée
}

/// Jump buffering : un saut pré-appuyé peu avant l'atterrissage s'exécute à la pose ; trop tôt,
/// non.
TEST(PhysiquePersonnageIntegration, JumpBufferingHonoreUnSautPreAppuye) {
    EXPECT_TRUE(bufferedJump(2));    // appuyé 2 pas avant l'atterrissage → saute à la pose
    EXPECT_FALSE(bufferedJump(30));  // appuyé bien trop tôt → buffer expiré
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
    const core::Entity player = spawnPlayer(world, static_cast<float>(level.entry().column),
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
