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
#include <limits>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PhysicsConfig.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"

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

// Fait apparaître le personnage à sa **vraie taille** (humanoïde 0,4×0,8), centré dans la tuile.
core::Entity spawnHumanoid(core::World& world, core::GridPosition entry) {
    const core::Entity entity = world.createEntity();
    const core::Vector2 size = core::playerSize();
    world.addComponent(
        entity, core::Transform{core::playerSpawnPosition(entry.column, entry.row), size, 0.0f});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Collider{size});
    world.addComponent(entity, core::Player{});
    return entity;
}

// Rejoue un niveau AVEC ses mécanismes (interrupteurs/portes) : mime la boucle du GameScreen
// (physique sur la grille des mécanismes, puis mise à jour des mécanismes), budget appliqué.
core::LevelOutcome simulatePuzzle(const core::Level& level,
                                  const std::function<core::PlayerInput(int)>& input,
                                  int maxSteps = 3000) {
    core::World world;
    const core::Entity player = spawnHumanoid(world, level.entry());
    world.getComponent<core::Player>(player).jumpsRemaining = level.jumpBudget();
    world.getComponent<core::Player>(player).dashesRemaining = level.dashBudget();
    core::CharacterPhysicsSystem system;
    core::MechanismController mechanisms(level);

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < maxSteps && outcome == core::LevelOutcome::Playing; ++step) {
        system.update(world, mechanisms.collisionMap(), input(step), STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);
        mechanisms.update(box);
        outcome = core::evaluateOutcome(box, level);
    }
    return outcome;
}

// Charge un niveau puzzle livré et le rejoue avec ses mécanismes.
core::LevelOutcome playPuzzleFile(const char* file,
                                  const std::function<core::PlayerInput(int)>& input) {
    const std::filesystem::path path = std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        return core::LevelOutcome::Lost;
    }
    return simulatePuzzle(*loaded.level, input);
}

// Rejoue un niveau livré avec un scénario d'entrées (fonction du pas) et renvoie son issue.
// Le personnage a sa vraie taille (humanoïde), pour prouver que le VRAI perso franchit le niveau.
core::LevelOutcome playLevelFile(const char* file,
                                 const std::function<core::PlayerInput(int)>& input) {
    const std::filesystem::path path = std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        return core::LevelOutcome::Lost;
    }
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawnHumanoid(world, level.entry());
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

/**
 * @brief Sans sol, le personnage tombe et sa vitesse verticale croît (gravité continue).
 * \castest{<b>Sans sol, le personnage tombe et sa vitesse verticale croît (gravité
 * continue).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans sol, le personnage tombe et sa vitesse verticale croît (gravité continue).
 * }
 */
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

/**
 * @brief En chute prolongée, la vitesse verticale converge vers une vitesse terminale sans la
 * dépasser (chute newtonienne, `EX-GP-019`).
 * \castest{<b>En chute prolongée, la vitesse verticale converge vers une vitesse terminale sans la
 * dépasser.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu En chute prolongée, la vitesse verticale converge vers une vitesse terminale sans la
 * dépasser.
 * }
 */
TEST(PhysiquePersonnageIntegration, ChuteConvergeVersUneVitesseTerminale) {
    core::World world;
    core::TileMap tiles(4, 1000);  // chute libre, très longue
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{};

    // Vitesse terminale attendue a masse par defaut (1.0) : gravite effective en chute
    // (config.gravity * config.fallGravityMultiplier) / config.fallDragCoefficient.
    const core::PhysicsConfig config;
    const float expectedTerminal = (config.gravity * config.fallGravityMultiplier) /
                                   config.fallDragCoefficient;

    float lastVy = 0.0f;
    for (int i = 0; i < 2000; ++i) {
        system.update(world, tiles, input, STEP);
        lastVy = world.getComponent<core::Velocity>(player).value.y;
        ASSERT_LE(lastVy, expectedTerminal + TOLERANCE);  // jamais depassee (asymptote)
    }
    EXPECT_NEAR(lastVy, expectedTerminal, 0.05f);  // convergence apres un temps suffisant
}

/**
 * @brief L'accélération verticale décroît à mesure que la vitesse approche le régime permanent
 * (courbe asymptotique, pas de plafond net).
 * \castest{<b>L'accélération verticale décroît à mesure que la vitesse approche le régime
 * permanent.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'accélération verticale décroît à mesure que la vitesse approche le régime permanent.
 * }
 */
TEST(PhysiquePersonnageIntegration, AccelerationDeChuteDecroitVersLeRegimePermanent) {
    core::World world;
    core::TileMap tiles(4, 1000);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    // Flottement a l'apex desactive (apexThreshold = 0) : isole la traction newtonienne pure,
    // sans le palier de gravite reduite pres de v=0 (EX-GP-018, comportement inchange par ailleurs).
    core::PhysicsConfig config;
    config.apexThreshold = 0.0f;
    core::CharacterPhysicsSystem system(config);
    const core::PlayerInput input{};

    // Premier pas ecarte : au tout premier pas, la vitesse passe de 0 (aucun multiplicateur de
    // chute, v <= 0) a positive (multiplicateur de chute applique des lors, EX-GP-018) — une
    // marche haute intentionnelle, distincte de la traction newtonienne que ce test isole.
    system.update(world, tiles, input, STEP);
    float previousVy = world.getComponent<core::Velocity>(player).value.y;
    float previousDelta = std::numeric_limits<float>::infinity();
    for (int i = 0; i < 200; ++i) {
        system.update(world, tiles, input, STEP);
        const float vy = world.getComponent<core::Velocity>(player).value.y;
        const float delta = vy - previousVy;
        // Chaque pas gagne MOINS de vitesse que le precedent (accel. decroissante) : un plafond
        // brutal produirait au contraire un delta constant puis soudainement nul.
        EXPECT_LE(delta, previousDelta + TOLERANCE);
        previousDelta = delta;
        previousVy = vy;
    }
}

/**
 * @brief Une masse plus grande produit une vitesse terminale plus élevée, à traînée égale.
 * \castest{<b>Une masse plus grande produit une vitesse terminale plus élevée, à traînée
 * égale.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une masse plus grande produit une vitesse terminale plus élevée, à traînée égale.
 * }
 */
TEST(PhysiquePersonnageIntegration, MasseSuperieureTombePlusVite) {
    core::World world;
    core::TileMap tiles(4, 1000);
    const core::Entity light = spawnPlayer(world, 1.0f, 0.0f);
    const core::Entity heavy = spawnPlayer(world, 2.0f, 0.0f);
    world.getComponent<core::Player>(heavy).mass = 3.0f;  // trois fois plus lourd
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{};

    for (int i = 0; i < 300; ++i) {
        system.update(world, tiles, input, STEP);
    }

    const float lightVy = world.getComponent<core::Velocity>(light).value.y;
    const float heavyVy = world.getComponent<core::Velocity>(heavy).value.y;
    EXPECT_GT(heavyVy, lightVy);
}

/**
 * @brief Le personnage se pose sur le sol : vitesse verticale annulée, état « au sol » vrai.
 * \castest{<b>Le personnage se pose sur le sol : vitesse verticale annulée, état « au sol »
 * vrai.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage se pose sur le sol : vitesse verticale annulée, état « au sol » vrai.
 * }
 */
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

/**
 * @brief Poussé contre un mur, le personnage s'arrête au ras du mur (blocage horizontal).
 * \castest{<b>Poussé contre un mur, le personnage s'arrête au ras du mur (blocage
 * horizontal).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Poussé contre un mur, le personnage s'arrête au ras du mur (blocage horizontal).
 * }
 */
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

/**
 * @brief Sur terrain libre, l'avancée horizontale vaut vitesse × temps (vitesse constante).
 * \castest{<b>Sur terrain libre, l'avancée horizontale vaut vitesse × temps (vitesse
 * constante).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sur terrain libre, l'avancée horizontale vaut vitesse × temps (vitesse constante).
 * }
 */
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

/**
 * @brief Même en chute très rapide (grand pas), le personnage ne traverse pas le sol (balayage
 * continu).
 * \castest{<b>Même en chute très rapide (grand pas), le personnage ne traverse pas le sol (balayage
 * continu).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Même en chute très rapide (grand pas), le personnage ne traverse pas le sol (balayage
 * continu).
 * }
 */
TEST(PhysiquePersonnageIntegration, NeTraversePasLeSolEnChuteRapide) {
    core::World world;
    core::TileMap tiles(4, 60);
    fillRow(tiles, 50);  // sol loin en bas
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::PhysicsConfig fast;
    fast.gravity = 2000.0f;              // accélération énorme
    fast.fallDragCoefficient = 1.0e-6f;  // trainee quasi nulle : pas de borne, le pas dépasse une tuile
    core::CharacterPhysicsSystem system(fast);
    const core::PlayerInput input{};

    for (int i = 0; i < 300; ++i) {
        system.update(world, tiles, input, STEP);
    }

    const core::Transform& transform = world.getComponent<core::Transform>(player);
    EXPECT_NEAR(transform.position.y, 49.0f, TOLERANCE);  // posé sur le sol, non traversé
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
}

/**
 * @brief Mêmes entrées → même résultat : la simulation est déterministe (pas fixe).
 * \castest{<b>Mêmes entrées → même résultat : la simulation est déterministe (pas fixe).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Mêmes entrées → même résultat : la simulation est déterministe (pas fixe).
 * }
 */
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

/**
 * @brief Au sol, une pression de saut fait décoller le personnage (vitesse ascendante, il s'élève).
 * \castest{<b>Au sol, une pression de saut fait décoller le personnage (vitesse ascendante, il
 * s'élève).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Au sol, une pression de saut fait décoller le personnage (vitesse ascendante, il
 * s'élève).
 * }
 */
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

/**
 * @brief En l'air, une pression de saut n'a aucun effet : pas de double saut (`EX-GP-013`).
 * \castest{<b>En l'air, une pression de saut n'a aucun effet : pas de double saut
 * (`EX-GP-013`).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu En l'air, une pression de saut n'a aucun effet : pas de double saut (`EX-GP-013`).
 * }
 */
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

/**
 * @brief Hauteur de saut variable : maintenir le bouton saute plus haut que le relâcher tôt.
 * \castest{<b>Hauteur de saut variable : maintenir le bouton saute plus haut que le relâcher
 * tôt.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Hauteur de saut variable : maintenir le bouton saute plus haut que le relâcher tôt.
 * }
 */
TEST(PhysiquePersonnageIntegration, HauteurDeSautVariable) {
    const float fullJump = jumpApexHeight(1000);  // maintenu toute la montée
    const float shortHop = jumpApexHeight(1);     // relâché immédiatement

    EXPECT_GT(fullJump, shortHop + 0.3f);  // maintenir monte nettement plus haut
    EXPECT_GT(fullJump, 2.0f);             // ~2,25 tuiles avec le réglage par défaut
    EXPECT_LT(fullJump, 3.0f);
}

/**
 * @brief Double saut : 1 saut au sol + N sauts aériens (paramétrable), rechargés au contact du sol.
 * \castest{<b>Double saut : 1 saut au sol + N sauts aériens (paramétrable), rechargés au contact du
 * sol.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Double saut : 1 saut au sol + N sauts aériens (paramétrable), rechargés au contact du
 * sol.
 * }
 */
TEST(PhysiquePersonnageIntegration, DoubleSautNombreParametrable) {
    EXPECT_EQ(successfulJumps(1), 2);  // 1 au sol + 1 aérien (double saut)
    EXPECT_EQ(successfulJumps(2), 3);  // 1 au sol + 2 aériens
    EXPECT_EQ(successfulJumps(0), 1);  // aucun saut aérien : 1 seul
}

/**
 * @brief Gravité asymétrique : la chute accélère plus vite que la montée ne décélère (EX-GP-018).
 * \castest{<b>Gravité asymétrique : la chute accélère plus vite que la montée ne décélère
 * (EX-GP-018).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Gravité asymétrique : la chute accélère plus vite que la montée ne décélère
 * (EX-GP-018).
 * }
 */
TEST(PhysiquePersonnageIntegration, ChutePlusRapideQueLaMontee) {
    core::World world;
    core::TileMap tiles(4, 100);
    fillRow(tiles, 90);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    const core::PhysicsConfig config;
    for (int i = 0; i < 800; ++i) {  // se poser
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);

    float riseDelta = -1.0f;
    float fallDelta = -1.0f;
    float previous = world.getComponent<core::Velocity>(player).value.y;
    for (int i = 0; i < 250; ++i) {
        core::PlayerInput hold;
        hold.jumpHeld = true;  // maintenir : pas de coupe de hauteur
        system.update(world, tiles, hold, STEP);
        const float current = world.getComponent<core::Velocity>(player).value.y;
        const float delta = current - previous;  // variation de vitesse verticale sur le pas
        if (current < -config.apexThreshold - 2.0f && riseDelta < 0.0f) {
            riseDelta = delta;  // en montée franche (hors apex) : gravité de base
        }
        if (current > config.apexThreshold + 2.0f && fallDelta < 0.0f) {
            fallDelta = delta;  // en chute franche (hors apex) : gravité renforcée
        }
        previous = current;
    }
    ASSERT_GT(riseDelta, 0.0f);
    ASSERT_GT(fallDelta, 0.0f);
    EXPECT_GT(fallDelta, riseDelta * 1.3f);  // la chute est nettement plus « lourde »
}

/**
 * @brief Apex hang : près du sommet du saut, la gravité est réduite (contrôle flottant).
 * \castest{<b>Apex hang : près du sommet du saut, la gravité est réduite (contrôle
 * flottant).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Apex hang : près du sommet du saut, la gravité est réduite (contrôle flottant).
 * }
 */
TEST(PhysiquePersonnageIntegration, ApexHangReduitLaGravite) {
    core::World world;
    core::TileMap tiles(4, 100);
    fillRow(tiles, 90);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 800; ++i) {
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);

    float earlyRise = -1.0f;
    float atApex = -1.0f;
    float previous = world.getComponent<core::Velocity>(player).value.y;
    for (int i = 0; i < 250; ++i) {
        core::PlayerInput hold;
        hold.jumpHeld = true;
        system.update(world, tiles, hold, STEP);
        const float current = world.getComponent<core::Velocity>(player).value.y;
        const float delta = current - previous;
        if (current < -8.0f && earlyRise < 0.0f) {
            earlyRise = delta;  // montée franche : gravité de base
        }
        if (current < 0.0f && current > -2.0f) {
            atApex = delta;  // proche de l'apex (encore en montée) : gravité réduite
        }
        previous = current;
    }
    ASSERT_GT(earlyRise, 0.0f);
    ASSERT_GT(atApex, 0.0f);
    EXPECT_LT(atApex, earlyRise * 0.8f);  // moins de gravité au sommet
}

/**
 * @brief Fast-fall : maintenir « bas » en l'air fait tomber plus loin qu'une chute libre normale.
 * \castest{<b>Fast-fall : maintenir « bas » en l'air fait tomber plus loin qu'une chute libre
 * normale.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Fast-fall : maintenir « bas » en l'air fait tomber plus loin qu'une chute libre
 * normale.
 * }
 */
TEST(PhysiquePersonnageIntegration, FastFallAccelereLaChute) {
    const auto fallDistance = [](bool holdDown) {
        core::World world;
        core::TileMap tiles(4, 200);  // pas de sol : chute libre
        const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
        core::CharacterPhysicsSystem system;
        for (int i = 0; i < 14; ++i) {
            core::PlayerInput in;
            in.moveY = holdDown ? 1.0f : 0.0f;  // « bas » maintenu ?
            system.update(world, tiles, in, STEP);
        }
        return world.getComponent<core::Transform>(player).position.y;  // profondeur atteinte
    };
    EXPECT_GT(fallDistance(true), fallDistance(false));  // fast-fall descend plus bas
}

/**
 * @brief Dash horizontal : une ruée rapide (≫ vitesse normale) fait parcourir une grande distance.
 * \castest{<b>Dash horizontal : une ruée rapide (≫ vitesse normale) fait parcourir une grande
 * distance.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Dash horizontal : une ruée rapide (≫ vitesse normale) fait parcourir une grande
 * distance.
 * }
 */
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
    // Distance ≈ dashSpeed × dashDuration ; seuil relatif à la config (robuste au tuning).
    EXPECT_GT(world.getComponent<core::Transform>(player).position.x - startX,
              config.dashSpeed * config.dashDuration * 0.7f);
}

/**
 * @brief Dash diagonal (8 directions) : viser haut-droite envoie en +X et −Y.
 * \castest{<b>Dash diagonal (8 directions) : viser haut-droite envoie en +X et −Y.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Dash diagonal (8 directions) : viser haut-droite envoie en +X et −Y.
 * }
 */
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

/**
 * @brief Pendant un dash horizontal, la gravité est suspendue (la composante verticale reste
 * nulle).
 * \castest{<b>Pendant un dash horizontal, la gravité est suspendue (la composante verticale reste
 * nulle).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Pendant un dash horizontal, la gravité est suspendue (la composante verticale reste
 * nulle).
 * }
 */
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

/**
 * @brief Une seule ruée par phase aérienne : le second dash en l'air est refusé jusqu'au retour au
 * sol.
 * \castest{<b>Une seule ruée par phase aérienne : le second dash en l'air est refusé jusqu'au
 * retour au sol.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une seule ruée par phase aérienne : le second dash en l'air est refusé jusqu'au retour
 * au sol.
 * }
 */
TEST(PhysiquePersonnageIntegration, DashUneSeuleFoisEnLAir) {
    core::World world;
    core::TileMap tiles(30, 60);
    for (int col = 0; col <= 4; ++col) {  // plateforme à gauche, le vide à droite
        tiles.setTile(col, 30, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 1.0f, 28.0f);
    core::CharacterPhysicsSystem system;
    const core::PhysicsConfig config;
    for (int i = 0; i < 400; ++i) {  // se poser (dash rechargé au sol)
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    // Marcher à droite jusqu'à quitter la plateforme (chute dans le vide) : le dash reste chargé.
    const core::PlayerInput right{1.0f};
    int guard = 0;
    while (world.getComponent<core::Player>(player).grounded && guard < 600) {
        system.update(world, tiles, right, STEP);
        ++guard;
    }
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);
    ASSERT_TRUE(world.getComponent<core::Player>(player).dashAvailable);

    // 1er dash en l'air (horizontal).
    core::PlayerInput dash;
    dash.dashPressed = true;
    dash.moveX = 1.0f;
    system.update(world, tiles, dash, STEP);
    EXPECT_GT(world.getComponent<core::Velocity>(player).value.x, config.dashSpeed * 0.9f);

    for (int i = 0; i < 15; ++i) {  // fin du dash, toujours en chute dans le vide
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    ASSERT_FALSE(world.getComponent<core::Player>(player).dashAvailable);  // consommé
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);

    // 2e dash refusé (indisponible jusqu'au retour au sol).
    core::PlayerInput dash2;
    dash2.dashPressed = true;
    dash2.moveX = 1.0f;
    system.update(world, tiles, dash2, STEP);
    EXPECT_LT(world.getComponent<core::Velocity>(player).value.x, config.dashSpeed * 0.9f);
}

/**
 * @brief Un dash vers un mur ne le traverse pas (résolu par le balayage).
 * \castest{<b>Un dash vers un mur ne le traverse pas (résolu par le balayage).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un dash vers un mur ne le traverse pas (résolu par le balayage).
 * }
 */
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

/**
 * @brief Budget de sauts (EX-GP-024) : avec 1 saut, le premier fonctionne, le suivant est refusé.
 * \castest{<b>Budget de sauts (EX-GP-024) : avec 1 saut, le premier fonctionne, le suivant est
 * refusé.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Budget de sauts (EX-GP-024) : avec 1 saut, le premier fonctionne, le suivant est
 * refusé.
 * }
 */
TEST(PhysiquePersonnageIntegration, BudgetDeSautsRefuseAuDela) {
    core::World world;
    core::TileMap tiles(4, 200);
    fillRow(tiles, 150);
    const core::Entity player = spawnPlayer(world, 1.0f, 0.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 1500; ++i) {  // se poser
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    world.getComponent<core::Player>(player).jumpsRemaining = 1;  // budget : un seul saut

    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);
    EXPECT_LT(world.getComponent<core::Velocity>(player).value.y, 0.0f);  // a sauté
    EXPECT_EQ(world.getComponent<core::Player>(player).jumpsRemaining, 0);

    for (int i = 0; i < 500; ++i) {  // retombe et se pose
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    system.update(world, tiles, jump, STEP);  // 2e saut : budget épuisé
    EXPECT_GT(world.getComponent<core::Velocity>(player).value.y, -1.0f);  // aucune impulsion
}

/**
 * @brief Budget de dashs (EX-GP-024) : avec 1 dash, le premier fonctionne, le suivant est refusé.
 * \castest{<b>Budget de dashs (EX-GP-024) : avec 1 dash, le premier fonctionne, le suivant est
 * refusé.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Budget de dashs (EX-GP-024) : avec 1 dash, le premier fonctionne, le suivant est
 * refusé.
 * }
 */
TEST(PhysiquePersonnageIntegration, BudgetDeDashsRefuseAuDela) {
    core::World world;
    core::TileMap tiles(100, 5);
    fillRow(tiles, 3);
    const core::Entity player = spawnPlayer(world, 1.0f, 2.0f);
    core::CharacterPhysicsSystem system;
    for (int i = 0; i < 60; ++i) {
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    world.getComponent<core::Player>(player).dashesRemaining = 1;  // budget : un seul dash

    core::PlayerInput dash;
    dash.dashPressed = true;
    system.update(world, tiles, dash, STEP);
    EXPECT_GT(world.getComponent<core::Velocity>(player).value.x, 10.0f);  // a dashé
    EXPECT_EQ(world.getComponent<core::Player>(player).dashesRemaining, 0);

    for (int i = 0; i < 60; ++i) {  // fin du dash, se repose
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    system.update(world, tiles, dash, STEP);  // 2e dash : budget épuisé
    EXPECT_LT(world.getComponent<core::Velocity>(player).value.x, 10.0f);  // aucune ruée
}

/**
 * @brief Wall slide : collé à un mur en l'air, la vitesse de chute est plafonnée (descente
 * ralentie).
 * \castest{<b>Wall slide : collé à un mur en l'air, la vitesse de chute est plafonnée (descente
 * ralentie).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Wall slide : collé à un mur en l'air, la vitesse de chute est plafonnée (descente
 * ralentie).
 * }
 */
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

/**
 * @brief Wall jump : contre un mur à droite, un saut éjecte vers la gauche et le haut.
 * \castest{<b>Wall jump : contre un mur à droite, un saut éjecte vers la gauche et le
 * haut.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Wall jump : contre un mur à droite, un saut éjecte vers la gauche et le haut.
 * }
 */
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

/**
 * @brief Sans mur, la logique de wall jump ne s'active pas (et sans saut aérien, aucun saut en
 * l'air).
 * \castest{<b>Sans mur, la logique de wall jump ne s'active pas (et sans saut aérien, aucun saut en
 * l'air).</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans mur, la logique de wall jump ne s'active pas (et sans saut aérien, aucun saut en
 * l'air).
 * }
 */
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

/**
 * @brief Le niveau 2 est franchissable **avec le saut** : en avançant et sautant, on atteint la
 * sortie.
 * \castest{<b>Le niveau 2 est franchissable **avec le saut** : en avançant et sautant, on atteint
 * la sortie.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau 2 est franchissable **avec le saut** : en avançant et sautant, on atteint la
 * sortie.
 * }
 */
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

/**
 * @brief Le niveau 2 **exige** le saut : en avançant seulement (sans sauter), la marche bloque.
 * \castest{<b>Le niveau 2 **exige** le saut : en avançant seulement (sans sauter), la marche
 * bloque.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau 2 **exige** le saut : en avançant seulement (sans sauter), la marche bloque.
 * }
 */
TEST(PhysiquePersonnageIntegration, Niveau2RequiertLeSaut) {
    const auto rightOnly = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        return in;
    };
    EXPECT_NE(playLevelFile("demo2.json", rightOnly),
              core::LevelOutcome::Won);  // bloqué à la marche
}

/**
 * @brief Le niveau 3 (couloir bas + fosse) est franchissable **au dash** : avancer + dasher
 * franchit.
 * \castest{<b>Le niveau 3 (couloir bas + fosse) est franchissable **au dash** : avancer + dasher
 * franchit.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau 3 (couloir bas + fosse) est franchissable **au dash** : avancer + dasher
 * franchit.
 * }
 */
TEST(PhysiquePersonnageIntegration, Niveau3FranchissableAvecDash) {
    const auto rightAndDash = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        in.dashPressed = true;  // dash répété : franchit la fosse (plafond bas → saut impossible)
        return in;
    };
    EXPECT_EQ(playLevelFile("demo3.json", rightAndDash), core::LevelOutcome::Won);
}

/**
 * @brief Le niveau 3 **exige** le dash : en avançant seulement, on tombe dans la fosse de danger.
 * \castest{<b>Le niveau 3 **exige** le dash : en avançant seulement, on tombe dans la fosse de
 * danger.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau 3 **exige** le dash : en avançant seulement, on tombe dans la fosse de
 * danger.
 * }
 */
TEST(PhysiquePersonnageIntegration, Niveau3RequiertLeDash) {
    const auto rightOnly = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        return in;
    };
    EXPECT_NE(playLevelFile("demo3.json", rightOnly),
              core::LevelOutcome::Won);  // tombe dans la fosse
}

/**
 * @brief Niveau 4 (puzzle) : toucher l'interrupteur ouvre la porte → la sortie devient atteignable.
 * \castest{<b>Niveau 4 (puzzle) : toucher l'interrupteur ouvre la porte → la sortie devient
 * atteignable.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Niveau 4 (puzzle) : toucher l'interrupteur ouvre la porte → la sortie devient
 * atteignable.
 * }
 */
TEST(PhysiquePersonnageIntegration, Niveau4FranchissableAvecInterrupteur) {
    const auto right = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;  // le trajet passe sur l'interrupteur (ouvre la porte) puis la sortie
        return in;
    };
    EXPECT_EQ(playPuzzleFile("demo4.json", right), core::LevelOutcome::Won);
}

/**
 * @brief Une porte **fermée** (interrupteur non touché) **bloque** : la sortie reste inatteignable.
 * \castest{<b>Une porte **fermée** (interrupteur non touché) **bloque** : la sortie reste
 * inatteignable.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une porte **fermée** (interrupteur non touché) **bloque** : la sortie reste
 * inatteignable.
 * }
 */
TEST(PhysiquePersonnageIntegration, PorteFermeeBloque) {
    // Couloir au sol (ligne 4) ; interrupteur EN HAUT (hors du chemin), porte sur le chemin.
    core::TileMap map(12, 5);
    for (int col = 0; col < 12; ++col) {
        map.setTile(col, 4, core::TileType::Solid);
    }
    map.setTile(6, 0, core::TileType::Switch);  // inatteignable en marchant
    map.setTile(6, 3, core::TileType::Door);    // barre le couloir (ligne du personnage)
    std::vector<core::Mechanism> mechanisms{
        core::Mechanism{core::GridPosition{6, 0}, core::GridPosition{6, 3}}};
    const core::Level level("bloc", std::move(map), core::GridPosition{1, 3},
                            core::GridPosition{10, 3}, std::move(mechanisms));

    const auto right = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;  // n'atteint jamais l'interrupteur en haut → porte fermée
        return in;
    };
    EXPECT_NE(simulatePuzzle(level, right, 600), core::LevelOutcome::Won);  // bloqué par la porte
}

/**
 * @brief Coyote time : sauter juste après avoir quitté un bord fonctionne ; trop tard, non.
 * \castest{<b>Coyote time : sauter juste après avoir quitté un bord fonctionne ; trop tard,
 * non.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Coyote time : sauter juste après avoir quitté un bord fonctionne ; trop tard, non.
 * }
 */
TEST(PhysiquePersonnageIntegration, CoyoteTimeAutoriseUnSautJusteApresLeBord) {
    EXPECT_TRUE(jumpAfterLeavingLedge(0));    // à peine quitté le sol → saut permis
    EXPECT_FALSE(jumpAfterLeavingLedge(15));  // trop tard → fenêtre coyote expirée
}

/**
 * @brief Jump buffering : un saut pré-appuyé peu avant l'atterrissage s'exécute à la pose ; trop
 * tôt, non.
 * \castest{<b>Jump buffering : un saut pré-appuyé peu avant l'atterrissage s'exécute à la pose ;
 * trop tôt, non.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Jump buffering : un saut pré-appuyé peu avant l'atterrissage s'exécute à la pose ; trop
 * tôt, non.
 * }
 */
TEST(PhysiquePersonnageIntegration, JumpBufferingHonoreUnSautPreAppuye) {
    EXPECT_TRUE(bufferedJump(2));    // appuyé 2 pas avant l'atterrissage → saute à la pose
    EXPECT_FALSE(bufferedJump(30));  // appuyé bien trop tôt → buffer expiré
}

/**
 * @brief Le niveau de démonstration livré est **franchissable** sans saut : en maintenant « droite
 * », le personnage descend l'escalier de paliers et atteint la sortie sans jamais mourir.
 * \castest{<b>Le niveau de démonstration livré est **franchissable** sans saut : en maintenant «
 * droite », le personnage descend l'escalier de paliers et atteint la sortie sans jamais
 * mourir.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau de démonstration livré est **franchissable** sans saut : en maintenant «
 * droite », le personnage descend l'escalier de paliers et atteint la sortie sans jamais mourir.
 * }
 */
TEST(PhysiquePersonnageIntegration, NiveauDemoEstFranchissableEnAllantADroite) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo.json";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawnHumanoid(world, level.entry());
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

/**
 * @brief Marcher sur une pente ascendante fait monter le personnage progressivement, sans
 * traverser la surface (`EX-GP-003`).
 * \castest{<b>Marcher sur une pente ascendante fait monter le personnage progressivement, sans
 * traverser la surface.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Marcher sur une pente ascendante fait monter le personnage progressivement, sans
 * traverser la surface.
 * }
 */
TEST(PhysiquePersonnageIntegration, SuitUnePenteAscendanteEnMarchant) {
    core::World world;
    core::TileMap tiles(8, 8);
    for (int col = 0; col <= 1; ++col) {  // sol bas, avant la pente
        tiles.setTile(col, 6, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::SlopeUpRight);  // monte de gauche a droite
    for (int col = 3; col <= 7; ++col) {                // sol haut, apres la pente
        tiles.setTile(col, 5, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 0.3f, 5.0f);  // bord bas = 6.0 : posé au sol bas
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{1.0f};

    bool sawSlopeSample = false;
    float yEnterSlope = 0.0f;
    float yExitSlope = 0.0f;
    int guard = 0;
    float centerX = 0.0f;
    do {
        const float previousBottom = world.getComponent<core::Transform>(player).position.y + 1.0f;
        system.update(world, tiles, input, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        centerX = transform.position.x + 0.5f;
        if (centerX >= 2.0f && centerX < 3.0f) {  // au-dessus de la case de la pente
            const float bottom = transform.position.y + 1.0f;
            EXPECT_LE(bottom, 6.0f + TOLERANCE);  // jamais sous la surface basse de la pente
            EXPECT_GE(bottom, 5.0f - TOLERANCE);  // jamais au-dessus de sa surface haute
            if (!sawSlopeSample) {
                yEnterSlope = previousBottom;
                sawSlopeSample = true;
            }
            yExitSlope = bottom;
        }
        ++guard;
    } while (centerX < 3.5f && guard < 600);

    ASSERT_TRUE(sawSlopeSample);
    EXPECT_LT(yExitSlope, yEnterSlope - 0.5f);  // franchement monté en traversant la pente

    ASSERT_LT(guard, 600);
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
    EXPECT_NEAR(world.getComponent<core::Transform>(player).position.y, 4.0f,
                0.05f);  // posé sur le sol haut (bord bas = 5.0)
}

/**
 * @brief Marcher sur une pente descendante fait descendre le personnage progressivement,
 * symétrique de la montée (`EX-GP-003`).
 * \castest{<b>Marcher sur une pente descendante fait descendre le personnage progressivement,
 * symétrique de la montée.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Marcher sur une pente descendante fait descendre le personnage progressivement,
 * symétrique de la montée.
 * }
 */
TEST(PhysiquePersonnageIntegration, SuitUnePenteDescendanteEnMarchant) {
    core::World world;
    core::TileMap tiles(8, 8);
    for (int col = 0; col <= 1; ++col) {  // sol haut, avant la pente
        tiles.setTile(col, 5, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::SlopeUpLeft);  // descend de gauche a droite
    for (int col = 3; col <= 7; ++col) {               // sol bas, apres la pente
        tiles.setTile(col, 6, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 0.3f, 4.0f);  // bord bas = 5.0 : posé au sol haut
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{1.0f};

    bool sawSlopeSample = false;
    float yEnterSlope = 0.0f;
    float yExitSlope = 0.0f;
    int guard = 0;
    float centerX = 0.0f;
    do {
        const float previousBottom = world.getComponent<core::Transform>(player).position.y + 1.0f;
        system.update(world, tiles, input, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        centerX = transform.position.x + 0.5f;
        if (centerX >= 2.0f && centerX < 3.0f) {
            const float bottom = transform.position.y + 1.0f;
            EXPECT_LE(bottom, 6.0f + TOLERANCE);
            EXPECT_GE(bottom, 5.0f - TOLERANCE);
            if (!sawSlopeSample) {
                yEnterSlope = previousBottom;
                sawSlopeSample = true;
            }
            yExitSlope = bottom;
        }
        ++guard;
    } while (centerX < 3.5f && guard < 600);

    ASSERT_TRUE(sawSlopeSample);
    EXPECT_GT(yExitSlope, yEnterSlope + 0.5f);  // franchement descendu en traversant la pente

    ASSERT_LT(guard, 600);
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
    EXPECT_NEAR(world.getComponent<core::Transform>(player).position.y, 5.0f,
                0.05f);  // posé sur le sol bas (bord bas = 6.0)
}

/**
 * @brief Même en chute très rapide, le personnage tombant sur une pente se pose sur sa surface au
 * premier contact, sans la traverser (`EX-GP-003`).
 * \castest{<b>Même en chute très rapide, le personnage tombant sur une pente se pose sur sa surface
 * au premier contact, sans la traverser.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Même en chute très rapide, le personnage tombant sur une pente se pose sur sa surface
 * au premier contact, sans la traverser.
 * }
 */
TEST(PhysiquePersonnageIntegration, ChuteRapideSurUnePenteSansLaTraverser) {
    core::World world;
    core::TileMap tiles(4, 60);
    tiles.setTile(1, 50, core::TileType::SlopeUpRight);  // localX = 0.8 → hauteur = 0.2
    const core::Entity player = spawnPlayer(world, 1.3f, 0.0f);  // tombe de haut, colonne 1
    core::PhysicsConfig fast;
    fast.gravity = 2000.0f;
    fast.fallDragCoefficient = 1.0e-6f;  // pas fixe largement > une tuile (comme en chute sur sol)
    core::CharacterPhysicsSystem system(fast);
    const core::PlayerInput input{};

    for (int i = 0; i < 300; ++i) {
        system.update(world, tiles, input, STEP);
    }

    const core::Transform& transform = world.getComponent<core::Transform>(player);
    EXPECT_NEAR(transform.position.y, 49.2f, TOLERANCE);  // posé sur la surface, non traversée
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
}

/**
 * @brief Sauter depuis une pente produit une impulsion identique à un saut depuis un sol plat : le
 * suivi de pente ne l'absorbe pas (`EX-GP-003`).
 * \castest{<b>Sauter depuis une pente produit une impulsion identique à un saut depuis un sol plat
 * : le suivi de pente ne l'absorbe pas.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sauter depuis une pente produit une impulsion identique à un saut depuis un sol plat :
 * le suivi de pente ne l'absorbe pas.
 * }
 */
TEST(PhysiquePersonnageIntegration, SauteDepuisUnePenteImpulsionNormale) {
    core::World world;
    core::TileMap tiles(4, 60);
    tiles.setTile(1, 50, core::TileType::SlopeUpRight);
    const core::Entity player = spawnPlayer(world, 1.3f, 0.0f);
    core::CharacterPhysicsSystem system;
    const core::PlayerInput idle{};

    int guard = 0;  // tombe puis se pose sur la pente
    while (!world.getComponent<core::Player>(player).grounded && guard < 400) {
        system.update(world, tiles, idle, STEP);
        ++guard;
    }
    ASSERT_TRUE(world.getComponent<core::Player>(player).grounded);
    const float landedY = world.getComponent<core::Transform>(player).position.y;

    core::PlayerInput jump;
    jump.jumpPressed = true;
    jump.jumpHeld = true;
    system.update(world, tiles, jump, STEP);
    EXPECT_LT(world.getComponent<core::Velocity>(player).value.y,
              -10.0f);  // impulsion pleine, pas amortie par le calage de pente
    EXPECT_FALSE(world.getComponent<core::Player>(player).grounded);  // décollé, plus calé

    core::PlayerInput hold;
    hold.jumpHeld = true;
    for (int i = 0; i < 5; ++i) {
        system.update(world, tiles, hold, STEP);
    }
    EXPECT_LT(world.getComponent<core::Transform>(player).position.y, landedY);  // s'est élevé
}

/**
 * @brief La transition entre une pente et le sol plat se fait sans à-coup ni blocage : la position
 * verticale varie continûment (`EX-GP-003`).
 * \castest{<b>La transition entre une pente et le sol plat se fait sans à-coup ni blocage : la
 * position verticale varie continûment.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La transition entre une pente et le sol plat se fait sans à-coup ni blocage : la
 * position verticale varie continûment.
 * }
 */
TEST(PhysiquePersonnageIntegration, TransitionPenteSolPlatSansAACoup) {
    core::World world;
    core::TileMap tiles(8, 8);
    for (int col = 0; col <= 1; ++col) {
        tiles.setTile(col, 6, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::SlopeUpRight);
    for (int col = 3; col <= 7; ++col) {
        tiles.setTile(col, 5, core::TileType::Solid);
    }
    // Taille RÉELLE du personnage (0,4×0,8, `spawnHumanoid`) : bien plus étroite qu'une case, donc
    // bien plus proche du jeu réel que la boîte 1×1 des autres tests de ce fichier — pertinent ici
    // car la largeur influe directement sur l'ampleur du raccord pente → sol plat.
    const core::Vector2 size = core::playerSize();
    const core::Entity player = world.createEntity();
    world.addComponent(player, core::Transform{core::Vector2{0.3f, 6.0f - size.y}, size, 0.0f});
    world.addComponent(player, core::Velocity{});
    world.addComponent(player, core::Collider{size});
    world.addComponent(player, core::Player{});
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{1.0f};

    float previousBottom = world.getComponent<core::Transform>(player).position.y + size.y;
    float maxStep = 0.0f;
    for (int i = 0; i < 300; ++i) {
        system.update(world, tiles, input, STEP);
        const float bottom = world.getComponent<core::Transform>(player).position.y + size.y;
        maxStep = std::max(maxStep, std::abs(bottom - previousBottom));
        previousBottom = bottom;
    }

    // Une variation d'un pas ~ vitesse horizontale × pas de temps (pente à 45°) : un à-coup (mur
    // invisible, téléportation) produirait un saut bien plus grand qu'un seul pas de marche. Le
    // raccord final (sortie de la pente vers le plein solide adjacent) produit un rattrapage un
    // peu plus large qu'un pas normal (largeur du personnage < largeur d'une case) : on borne
    // large (une demi-case) pour couvrir ce rattrapage sans laisser passer un vrai à-coup (≥ 1 case).
    EXPECT_LT(maxStep, 0.5f);
}
