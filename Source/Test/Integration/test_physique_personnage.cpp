// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_physique_personnage.cpp
 * @brief Tests d'intégration de la physique du personnage : ECS + grille + balayage assemblés.
 *
 * Le `CharacterPhysicsSystem` combine composants (`Player`/`Transform`/`Velocity`/`Collider`),
 * grille de collision (`TileMap`) et résolution continue (`sweepAabb`). On vérifie de bout en bout
 * la gravité, l'atterrissage, le blocage, la vitesse constante, le non-tunneling et le
 * déterminisme.
 *
 * **Ce module ne rejoue plus les tableaux livrés** (`LOT-65` TACHE-07). Il portait une solution
 * scriptée par tableau — un doublon, plus faible, de ce que `ParcoursCompletSysteme` rejoue déjà en
 * entier, avec des scripts réactifs et deux garde-fous que celui-ci n'avait pas (anti-couloir,
 * proximité au trajet). Maintenir la solution d'un tableau à **deux** endroits est un piège : la
 * refonte du contenu a cassé la copie la plus faible, sans rien apprendre que le test système
 * n'ait déjà dit. Les tests **négatifs** restent ici (« le niveau *exige* le saut / le dash »,
 * « une porte fermée bloque ») : ils n'encodent aucune solution, seulement une propriété que la
 * physique doit garantir quelle que soit la géométrie livrée.
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
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
#include "Core/Gameplay/BlockController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Gameplay/PlatformController.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
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
    for (int column = 0; column < tiles.width(); ++column) {
        tiles.setTile(column, row, core::TileType::Solid);
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
    for (int column = 0; column <= 3; ++column) {  // rebord : sol sur les colonnes 0..3
        tiles.setTile(column, 12, core::TileType::Solid);
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

// Rejoue un niveau AVEC ses mécanismes (interrupteurs/portes) : mime la boucle du GameSession
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

// Script d'entrées REACTIF : fonction du pas ET de l'état courant (au sol, position), pour les
// scénarios où le bon moment d'agir dépend de la trajectoire (double saut, wall jump, blocs) et ne
// peut pas être fixé à l'avance par un simple numéro de pas.
using ReactiveInput =
    std::function<core::PlayerInput(int step, const core::Player& player, float x, float y)>;

// Rejoue un niveau livré, mécanismes ET blocs poussables compris (même composition que
// `hmi::GameSession::update` : blocs -> grille -> balayage -> boîte-boîte pour les blocs réduits),
// avec un script d'entrées réactif. Couvre tous les niveaux démo qui combinent plusieurs
// mécaniques (LOT-25).
core::LevelOutcome playReactiveFile(const char* file, const ReactiveInput& input,
                                    int maxSteps = 3000) {
    const std::filesystem::path path = std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        return core::LevelOutcome::Lost;
    }
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawnHumanoid(world, level.entry());
    world.getComponent<core::Player>(player).jumpsRemaining = level.jumpBudget();
    world.getComponent<core::Player>(player).dashesRemaining = level.dashBudget();
    core::CharacterPhysicsSystem system;
    core::BlockController blocks(level);
    core::MechanismController mechanisms(level);
    core::PlatformController platforms(level);

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < maxSteps && outcome == core::LevelOutcome::Playing; ++step) {
        const core::Transform& previousTransform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        const core::Aabb previousBox =
            core::Aabb::fromTopLeftSize(previousTransform.position, collider.size);
        const core::PlayerInput in =
            input(step, world.getComponent<core::Player>(player), previousTransform.position.x,
                  previousTransform.position.y);

        // Plateformes mobiles (EX-GP-026) : deplacees EN PREMIER, comme hmi::GameSession::update.
        platforms.update();
        const std::vector<core::PlatformSample> platformSamples = platforms.samples();

        const core::TileMap mechanismMap = mechanisms.collisionMap();
        blocks.update(previousBox, in.moveX, mechanismMap, platformSamples);
        const core::TileMap collision = blocks.collisionMap(mechanismMap);
        system.update(world, collision, in, STEP, platformSamples);

        // Composition boîte-boîte pour les blocs réduits (EX-GP-005), comme GameSession::update.
        core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Vector2 delta = transform.position - previousBox.min;
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
        if (bestNormal.x != 0.0f || bestNormal.y != 0.0f) {
            transform.position = bestPosition;
            core::Velocity& velocity = world.getComponent<core::Velocity>(player);
            if (bestNormal.x != 0.0f) {
                velocity.value.x = 0.0f;
            }
            if (bestNormal.y != 0.0f) {
                velocity.value.y = 0.0f;
                if (bestNormal.y < 0.0f) {
                    world.getComponent<core::Player>(player).grounded = true;
                }
            }
        }

        const core::Aabb box = core::Aabb::fromTopLeftSize(transform.position, collider.size);
        mechanisms.update(box, 1.0f, in.interactPressed);

        // Ecrasement par une plateforme mobile (EX-GP-026) : mortel, comme
        // hmi::GameSession::update.
        std::vector<core::Aabb> extraDangerBoxes;
        if (world.getComponent<core::Player>(player).squished) {
            extraDangerBoxes.push_back(box);
        }
        outcome = core::evaluateOutcome(box, level, extraDangerBoxes);
    }
    return outcome;
}

// Rejoue un niveau livré (physique seule, sans mécanisme ni bloc) avec un script d'entrées
// réactif — pour les scénarios purement physiques dont le timing dépend de la trajectoire
// (wall jump : la fenêtre de contact avec un mur ne peut pas être prédite à l'avance).
core::LevelOutcome playReactivePhysicsOnly(const char* file, const ReactiveInput& input,
                                           int maxSteps = 3000) {
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
    for (int step = 0; step < maxSteps && outcome == core::LevelOutcome::Playing; ++step) {
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::PlayerInput in = input(step, world.getComponent<core::Player>(player),
                                           transform.position.x, transform.position.y);
        system.update(world, level.tileMap(), in, STEP);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        outcome = core::evaluateOutcome(
            core::Aabb::fromTopLeftSize(world.getComponent<core::Transform>(player).position,
                                        collider.size),
            level);
    }
    return outcome;
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
    const float expectedTerminal =
        (config.gravity * config.fallGravityMultiplier) / config.fallDragCoefficient;

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
    // sans le palier de gravite reduite pres de v=0 (EX-GP-018, comportement inchange par
    // ailleurs).
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
    fast.gravity = 2000.0f;  // accélération énorme
    fast.fallDragCoefficient =
        1.0e-6f;  // trainee quasi nulle : pas de borne, le pas dépasse une tuile
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
    for (int column = 0; column <= 4; ++column) {  // plateforme à gauche, le vide à droite
        tiles.setTile(column, 30, core::TileType::Solid);
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
    ASSERT_GT(world.getComponent<core::Player>(player).dashChargesRemaining, 0);

    // 1er dash en l'air (horizontal).
    core::PlayerInput dash;
    dash.dashPressed = true;
    dash.moveX = 1.0f;
    system.update(world, tiles, dash, STEP);
    EXPECT_GT(world.getComponent<core::Velocity>(player).value.x, config.dashSpeed * 0.9f);

    for (int i = 0; i < 15; ++i) {  // fin du dash, toujours en chute dans le vide
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    ASSERT_EQ(world.getComponent<core::Player>(player).dashChargesRemaining, 0);  // consommée
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);

    // 2e dash refusé (indisponible jusqu'au retour au sol).
    core::PlayerInput dash2;
    dash2.dashPressed = true;
    dash2.moveX = 1.0f;
    system.update(world, tiles, dash2, STEP);
    EXPECT_LT(world.getComponent<core::Velocity>(player).value.x, config.dashSpeed * 0.9f);
}

/**
 * @brief Un tableau accordant deux charges de dash en autorise deux d'affilée en l'air, puis
 *        refuse la troisième jusqu'au retour au sol (`EX-GP-055`).
 * \castest{<b>Deux charges de dash autorisent deux ruées en l'air, pas trois.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Configurer la physique avec deux charges de dash et quitter le sol.<br/>2. Dasher
 * deux fois en l'air, en laissant chaque ruee se terminer.<br/>3. Tenter une troisieme ruee.<br/>
 * \tattendu Les deux premieres ruees atteignent la vitesse de dash, la troisieme est refusee : le
 * nombre de charges est bien celui du tableau et non le reglage par defaut du moteur.
 * }
 */
TEST(PhysiquePersonnageIntegration, DeuxChargesDeDashAutorisentDeuxRueesEnLAir) {
    core::World world;
    core::TileMap tiles(30, 60);
    for (int column = 0; column <= 4; ++column) {  // plateforme a gauche, le vide a droite
        tiles.setTile(column, 30, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 1.0f, 28.0f);
    core::PhysicsConfig config;
    config.dashCharges = 2;  // ce que ferait un niveau portant "dashCharges": 2
    core::CharacterPhysicsSystem system(config);
    for (int i = 0; i < 400; ++i) {  // se poser (charges rechargees au sol)
        system.update(world, tiles, core::PlayerInput{}, STEP);
    }
    const core::PlayerInput right{1.0f};
    int guard = 0;
    while (world.getComponent<core::Player>(player).grounded && guard < 600) {
        system.update(world, tiles, right, STEP);
        ++guard;
    }
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);
    ASSERT_EQ(world.getComponent<core::Player>(player).dashChargesRemaining, 2);

    core::PlayerInput dash;
    dash.dashPressed = true;
    dash.moveX = 1.0f;

    // Deux ruees successives, chacune suivie de la fin de son minuteur.
    for (int attempt = 0; attempt < 2; ++attempt) {
        system.update(world, tiles, dash, STEP);
        EXPECT_GT(world.getComponent<core::Velocity>(player).value.x, config.dashSpeed * 0.9f)
            << "ruee n" << attempt + 1;
        for (int i = 0; i < 15; ++i) {
            system.update(world, tiles, core::PlayerInput{}, STEP);
        }
    }
    ASSERT_EQ(world.getComponent<core::Player>(player).dashChargesRemaining, 0);
    ASSERT_FALSE(world.getComponent<core::Player>(player).grounded);

    // Troisieme ruee refusee : les deux charges du tableau sont epuisees.
    system.update(world, tiles, dash, STEP);
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
 * @brief Le niveau saut **exige** le saut : en avançant seulement (sans sauter), la marche bloque.
 * \castest{<b>Le niveau saut **exige** le saut : en avançant seulement (sans sauter), la marche
 * bloque.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau saut **exige** le saut : en avançant seulement (sans sauter), la marche
 * bloque.
 * }
 */
TEST(PhysiquePersonnageIntegration, Niveau2RequiertLeSaut) {
    const auto rightOnly = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        return in;
    };
    EXPECT_NE(playLevelFile("demo-saut.json", rightOnly),
              core::LevelOutcome::Won);  // bloqué à la marche
}

/**
 * @brief Le niveau dash **exige** le dash : en avançant seulement, on tombe dans la fosse de
 * danger. \castest{<b>Le niveau dash **exige** le dash : en avançant seulement, on tombe dans la
 * fosse de danger.</b><br/> \tcat Integration · Physique Personnage<br/> \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau dash **exige** le dash : en avançant seulement, on tombe dans la fosse de
 * danger.
 * }
 */
TEST(PhysiquePersonnageIntegration, Niveau3RequiertLeDash) {
    const auto rightOnly = [](int) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        return in;
    };
    EXPECT_NE(playLevelFile("demo-dash.json", rightOnly),
              core::LevelOutcome::Won);  // tombe dans la fosse
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
    for (int column = 0; column < 12; ++column) {
        map.setTile(column, 4, core::TileType::Solid);
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
    for (int column = 0; column <= 1; ++column) {  // sol bas, avant la pente
        tiles.setTile(column, 6, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::SlopeUpRight);  // monte de gauche a droite
    for (int column = 3; column <= 7; ++column) {       // sol haut, apres la pente
        tiles.setTile(column, 5, core::TileType::Solid);
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
        // Fenêtre d'échantillonnage élargie à [1.5, 3.5) plutôt que [2.0, 3.0) : la boîte du
        // personnage (1×1, une case entière) peut chevaucher la case de la pente dès que son bord
        // AVANT l'atteint (x ≥ 1.0, soit centerX ≥ 1.5) — la case de suivi la plus haute touchée
        // par l'empreinte de la boîte gouverne (voir SlopeGeometry.cpp), pas seulement son centre.
        if (centerX >= 1.5f && centerX < 3.5f) {  // au-dessus ou chevauchant la case de la pente
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
    } while (centerX < 4.0f && guard < 600);

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
    for (int column = 0; column <= 1; ++column) {  // sol haut, avant la pente
        tiles.setTile(column, 5, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::SlopeUpLeft);  // descend de gauche a droite
    for (int column = 3; column <= 7; ++column) {      // sol bas, apres la pente
        tiles.setTile(column, 6, core::TileType::Solid);
    }
    const core::Entity player =
        spawnPlayer(world, 0.3f, 4.0f);  // bord bas = 5.0 : posé au sol haut
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
        // Seuil à 4.0 plutôt que 3.5 (juste après la pente) : la boîte 1×1 chevauche encore
        // brièvement la pente et le sol bas juste après la transition, un « rattrapage » de contact
        // (grounded oscille sol/chute sur quelques pas, comme `TransitionPenteSolPlatSansAACoup`)
        // le temps que la boîte tienne entièrement dans une colonne de sol plat — laisser un peu
        // plus de marge avant de vérifier l'état final évite un pas malchanceux pris en plein
        // rattrapage.
    } while (centerX < 4.0f && guard < 600);

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
    tiles.setTile(1, 50, core::TileType::SlopeUpRight);          // localX = 0.8 → hauteur = 0.2
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
    for (int column = 0; column <= 1; ++column) {
        tiles.setTile(column, 6, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::SlopeUpRight);
    for (int column = 3; column <= 7; ++column) {
        tiles.setTile(column, 5, core::TileType::Solid);
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
    // large (une demi-case) pour couvrir ce rattrapage sans laisser passer un vrai à-coup (≥ 1
    // case).
    EXPECT_LT(maxStep, 0.5f);
}

/**
 * @brief Marcher sur un arrondi ascendant fait monter le personnage en suivant une courbe
 * distincte d'une pente linéaire, sans le traverser (`EX-GP-004`).
 * \castest{<b>Marcher sur un arrondi ascendant fait monter le personnage en suivant une courbe
 * distincte d'une pente linéaire, sans le traverser.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Marcher sur un arrondi ascendant fait monter le personnage en suivant une courbe
 * distincte d'une pente linéaire, sans le traverser.
 * }
 */
TEST(PhysiquePersonnageIntegration, SuitUnArrondiAscendantEnMarchant) {
    core::World world;
    core::TileMap tiles(8, 8);
    for (int column = 0; column <= 1;
         ++column) {  // sol bas, avant l'arrondi (meme disposition que la pente)
        tiles.setTile(column, 6, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::RoundedUpRight);  // quart de cercle, meme orientation
    for (int column = 3; column <= 7; ++column) {         // sol haut, apres l'arrondi
        tiles.setTile(column, 5, core::TileType::Solid);
    }
    const core::Entity player = spawnPlayer(world, 0.3f, 5.0f);  // bord bas = 6.0 : pose au sol bas
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{1.0f};

    bool sawMidSample = false;
    float midBottom = 0.0f;
    int guard = 0;
    float centerX = 0.0f;
    do {
        system.update(world, tiles, input, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        centerX = transform.position.x + 0.5f;
        if (centerX >= 2.0f && centerX < 3.0f) {  // au-dessus de la case de l'arrondi
            const float bottom = transform.position.y + 1.0f;
            EXPECT_LE(bottom, 6.0f + TOLERANCE);  // jamais sous la surface basse
            EXPECT_GE(bottom, 5.0f - TOLERANCE);  // jamais au-dessus de la surface haute
            if (!sawMidSample && centerX >= 2.5f) {
                midBottom = bottom;  // localX ~ 0.5 : premier echantillon a mi-case
                sawMidSample = true;
            }
        }
        ++guard;
    } while (centerX < 3.5f && guard < 600);

    ASSERT_TRUE(sawMidSample);
    // Valeur EXACTE d'une pente lineaire a mi-case : bas + 0,5 = 5,5. La courbe (quart de cercle,
    // concave, tangente verticale cote bas) est deja BEAUCOUP plus haute a ce stade (hauteur
    // theorique ~0,134, soit bord bas ~5,134) : un ecart net avec 5,5 prouve que la formule courbe
    // est reellement utilisee, pas une pente lineaire deguisee.
    EXPECT_LT(midBottom, 5.3f);

    ASSERT_LT(guard, 600);
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
    EXPECT_NEAR(world.getComponent<core::Transform>(player).position.y, 4.0f,
                0.05f);  // pose sur le sol haut (bord bas = 5.0), comme pour une pente
}

/**
 * @brief Le niveau plaque de pression **exige** de sauter par-dessus le mur : marcher seul
 * (sans jamais sauter) reste bloqué contre le mur, même une fois la plaque activée (`EX-GP-025`).
 * \castest{<b>Le niveau plaque de pression exige un saut : marcher sans sauter reste bloqué contre
 * le mur.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau plaque de pression exige un saut : marcher sans sauter reste bloqué contre
 * le mur.
 * }
 */
TEST(PhysiquePersonnageIntegration, NiveauPlaquePressionExigeUnSaut) {
    const auto rightOnly = [](int) { return core::PlayerInput{1.0f}; };
    EXPECT_NE(playLevelFile("demo-plaque-pression.json", rightOnly), core::LevelOutcome::Won);
}

/**
 * @brief Une pente de **plafond** (`SlopeDownRight`, `EX-GP-006`) bloque un saut selon sa
 * silhouette réelle, pas comme un carré plein uniforme : sous son bord fin (silhouette quasi
 * vide), le personnage monte bien plus haut que sous son bord épais (silhouette quasi pleine).
 * \castest{<b>Une pente de plafond bloque un saut selon sa silhouette réelle, pas comme un carré
 * plein uniforme.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage monte nettement plus haut sous le bord fin de la pente de plafond que
 * sous son bord épais — la collision suit sa silhouette inclinée, pas une case pleine.
 * }
 */
TEST(PhysiquePersonnageIntegration, PlafondInclineBloqueSelonSaSilhouette) {
    constexpr int CEILING_COLUMN = 3;
    constexpr int CEILING_ROW = 3;
    constexpr int FLOOR_ROW = 5;

    const auto minYReached = [](float startX) {
        core::TileMap map(10, 6);
        for (int column = 0; column < 10; ++column) {
            map.setTile(column, FLOOR_ROW, core::TileType::Solid);
        }
        map.setTile(CEILING_COLUMN, CEILING_ROW, core::TileType::SlopeDownRight);

        core::World world;
        const core::Entity player =
            spawnHumanoid(world, core::GridPosition{CEILING_COLUMN, FLOOR_ROW - 1});
        world.getComponent<core::Transform>(player).position.x = startX;
        core::CharacterPhysicsSystem system;

        bool jumped = false;
        float minY = 100.0f;
        for (int step = 0; step < 90; ++step) {
            const core::Player& pl = world.getComponent<core::Player>(player);
            core::PlayerInput in;
            if (!jumped && pl.grounded) {
                in.jumpPressed = true;
                jumped = true;
            }
            if (jumped) {
                in.jumpHeld = true;
            }
            system.update(world, map, in, STEP);
            minY = std::min(minY, world.getComponent<core::Transform>(player).position.y);
        }
        return minY;
    };

    // core::ceilingSlopeHeight(SlopeDownRight, x) = x : silhouette quasi vide (h~0) près du bord
    // GAUCHE de la case, quasi pleine (h~1) près du bord DROIT.
    const float thinSideMinY = minYReached(static_cast<float>(CEILING_COLUMN) + 0.05f);
    const float thickSideMinY = minYReached(static_cast<float>(CEILING_COLUMN) + 0.55f);

    EXPECT_LT(thinSideMinY, thickSideMinY - 0.3f);
}

/**
 * @brief La **face du haut** d'une pente de plafond (`SlopeDownRight`, `EX-GP-006`) est plate et
 * supporte le personnage qui tombe dessus **par le dessus** — il ne tombe pas au travers jusqu'au
 * sol lointain en dessous.
 * \castest{<b>La face du haut d'une pente de plafond supporte le personnage qui tombe dessus par
 * le dessus.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage se pose sur la face du haut de la tuile (au sommet de sa case),
 * `grounded` devient vrai — il ne tombe pas au travers jusqu'au sol lointain en dessous.
 * }
 */
TEST(PhysiquePersonnageIntegration, PlafondInclineSupportePersonnageParLeDessus) {
    constexpr int CEILING_COLUMN = 3;
    constexpr int CEILING_ROW = 3;
    constexpr int FAR_FLOOR_ROW = 9;  // tres eloigne : prouve qu'il ne s'agit pas d'une coincidence

    core::TileMap map(10, 10);
    for (int column = 0; column < 10; ++column) {
        map.setTile(column, FAR_FLOOR_ROW, core::TileType::Solid);
    }
    map.setTile(CEILING_COLUMN, CEILING_ROW, core::TileType::SlopeDownRight);

    core::World world;
    const core::Entity player = spawnHumanoid(world, core::GridPosition{CEILING_COLUMN, 0});
    core::CharacterPhysicsSystem system;

    for (int step = 0; step < 150; ++step) {
        system.update(world, map, core::PlayerInput{}, STEP);  // pas d'entree : chute libre
    }

    const core::Player& player_ = world.getComponent<core::Player>(player);
    const core::Transform& transform = world.getComponent<core::Transform>(player);
    EXPECT_TRUE(player_.grounded);
    EXPECT_NEAR(transform.position.y, static_cast<float>(CEILING_ROW) - core::PLAYER_HEIGHT, 0.05f);
}

/**
 * @brief Marcher sur un arrondi **concave** ascendant (`ConcaveUpRight`, `EX-GP-007`) fait monter
 * le personnage en suivant une courbe **inversée** par rapport à l'arrondi convexe : la hauteur
 * reste proche du palier bas sur l'essentiel de la case (tangente horizontale côté creux), puis
 * grimpe brutalement près du bord droit (tangente verticale côté plein) — l'inverse exact de
 * `SuitUnArrondiAscendantEnMarchant`.
 * \castest{<b>Marcher sur un arrondi concave ascendant suit une courbe inversée par rapport à
 * l'arrondi convexe, sans le traverser.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu À mi-case, le personnage reste bien plus proche du palier bas qu'avec l'arrondi
 * convexe (qui y est déjà bien plus haut) ; il se pose au final sur le palier haut, sans jamais
 * traverser la surface.
 * }
 */
TEST(PhysiquePersonnageIntegration, SuitUnArrondiConcaveAscendantEnMarchant) {
    core::World world;
    core::TileMap tiles(8, 8);
    for (int column = 0; column <= 1;
         ++column) {  // sol bas, avant l'arrondi (meme disposition que RoundedUpRight)
        tiles.setTile(column, 6, core::TileType::Solid);
    }
    tiles.setTile(2, 5,
                  core::TileType::ConcaveUpRight);  // quart de cercle concave, meme orientation
    for (int column = 3; column <= 7; ++column) {   // sol haut, apres l'arrondi
        tiles.setTile(column, 5, core::TileType::Solid);
    }
    // Taille RÉELLE du personnage (0,4×0,8, comme `TransitionPenteSolPlatSansAACoup`), pas la boîte
    // 1×1 des autres tests de ce fichier : une boîte pleine case chevauche déjà le sol haut
    // adjacent (colonne 3, plein) au moment même de l'échantillon à mi-case (x=2,5, bord droit de
    // la boîte atteignant x=3,0), déclenchant le rattrapage de raccord pente→sol documenté par
    // `TransitionPenteSolPlatSansAACoup` avant que le suivi de la courbe concave n'ait eu la chance
    // de s'exprimer — non spécifique à cette formule, mais bien plus visible ici (courbe concave
    // restant proche du palier bas jusque tard) qu'avec la formule convexe (déjà proche du palier
    // haut à ce stade, `SuitUnArrondiAscendantEnMarchant`).
    const core::Vector2 size = core::playerSize();
    const core::Entity player = world.createEntity();
    world.addComponent(player, core::Transform{core::Vector2{0.3f, 6.0f - size.y}, size, 0.0f});
    world.addComponent(player, core::Velocity{});
    world.addComponent(player, core::Collider{size});
    world.addComponent(player, core::Player{});
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{1.0f};

    bool sawMidSample = false;
    float midBottom = 0.0f;
    int guard = 0;
    float centerX = 0.0f;
    do {
        system.update(world, tiles, input, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        centerX = transform.position.x + size.x * 0.5f;
        if (centerX >= 2.0f && centerX < 3.0f) {  // au-dessus de la case de l'arrondi
            const float bottom = transform.position.y + size.y;
            EXPECT_LE(bottom, 6.0f + TOLERANCE);  // jamais sous la surface basse
            EXPECT_GE(bottom, 5.0f - TOLERANCE);  // jamais au-dessus de la surface haute
            if (!sawMidSample && centerX >= 2.5f) {
                midBottom = bottom;  // localX ~ 0.5 : premier echantillon a mi-case
                sawMidSample = true;
            }
        }
        ++guard;
    } while (centerX < 3.5f && guard < 600);

    ASSERT_TRUE(sawMidSample);
    // Valeur theorique a mi-case : hauteur ~0,866 (sqrt(0,75)), soit bord bas ~5,866 — bien plus
    // proche du palier BAS (6,0) que l'arrondi convexe a ce meme stade (~5,134, voir
    // SuitUnArrondiAscendantEnMarchant) : un ecart net avec cette valeur convexe prouve que la
    // courbe CONCAVE est reellement utilisee, pas la formule convexe deguisee.
    EXPECT_GT(midBottom, 5.7f);

    ASSERT_LT(guard, 600);
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
    EXPECT_NEAR(world.getComponent<core::Transform>(player).position.y, 5.0f - size.y,
                0.05f);  // pose sur le sol haut (bord bas = 5.0), comme pour l'arrondi convexe
}

/**
 * @brief Un arrondi concave de **plafond** (`ConcaveDownRight`, `EX-GP-007`) bloque un saut selon
 * sa silhouette réelle, pas comme un carré plein uniforme : sous son bord fin (silhouette quasi
 * vide), le personnage monte bien plus haut que sous son bord épais (silhouette quasi pleine) —
 * même principe que `PlafondInclineBloqueSelonSaSilhouette`, formule concave.
 * \castest{<b>Un arrondi concave de plafond bloque un saut selon sa silhouette réelle, pas comme un
 * carré plein uniforme.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage monte nettement plus haut sous le bord fin de l'arrondi concave de
 * plafond que sous son bord épais — la collision suit sa silhouette courbe, pas une case pleine.
 * }
 */
TEST(PhysiquePersonnageIntegration, ConcaveDePlafondBloqueSelonSaSilhouette) {
    constexpr int CEILING_COLUMN = 3;
    constexpr int CEILING_ROW = 3;
    constexpr int FLOOR_ROW = 5;

    const auto minYReached = [](float startX) {
        core::TileMap map(10, 6);
        for (int column = 0; column < 10; ++column) {
            map.setTile(column, FLOOR_ROW, core::TileType::Solid);
        }
        map.setTile(CEILING_COLUMN, CEILING_ROW, core::TileType::ConcaveDownRight);

        core::World world;
        const core::Entity player =
            spawnHumanoid(world, core::GridPosition{CEILING_COLUMN, FLOOR_ROW - 1});
        world.getComponent<core::Transform>(player).position.x = startX;
        core::CharacterPhysicsSystem system;

        bool jumped = false;
        float minY = 100.0f;
        for (int step = 0; step < 90; ++step) {
            const core::Player& pl = world.getComponent<core::Player>(player);
            core::PlayerInput in;
            if (!jumped && pl.grounded) {
                in.jumpPressed = true;
                jumped = true;
            }
            if (jumped) {
                in.jumpHeld = true;
            }
            system.update(world, map, in, STEP);
            minY = std::min(minY, world.getComponent<core::Transform>(player).position.y);
        }
        return minY;
    };

    // core::ceilingSlopeHeight(ConcaveDownRight, x) = 1 - sqrt(1 - x^2) : silhouette quasi vide
    // (h~0, tangente horizontale) sur l'essentiel du bord GAUCHE, quasi pleine (h~1, tangente
    // verticale) tres pres du bord DROIT. Decalage (comme `PlafondInclineBloqueSelonSaSilhouette`)
    // : `startX` est le bord GAUCHE de la boite (largeur 0,4, `spawnHumanoid`), pas son centre —
    // +0.95 placerait le CENTRE (+0,2 de plus) a x=3,95+0,2=4,15, hors de la colonne du plafond
    // (colonne 4, vide) plutot que pres de son bord droit ; +0.55 garde le centre a 3,75, bien
    // dans la colonne 3.
    const float thinSideMinY = minYReached(static_cast<float>(CEILING_COLUMN) + 0.05f);
    const float thickSideMinY = minYReached(static_cast<float>(CEILING_COLUMN) + 0.55f);

    EXPECT_LT(thinSideMinY, thickSideMinY - 0.3f);
}

/**
 * @brief La **face du haut** d'un arrondi concave de plafond (`ConcaveDownRight`, `EX-GP-007`) est
 * plate et supporte le personnage qui tombe dessus **par le dessus** — il ne tombe pas au travers
 * jusqu'au sol lointain en dessous (même principe que
 * `PlafondInclineSupportePersonnageParLeDessus`).
 * \castest{<b>La face du haut d'un arrondi concave de plafond supporte le personnage qui tombe
 * dessus par le dessus.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage se pose sur la face du haut de la tuile (au sommet de sa case),
 * `grounded` devient vrai — il ne tombe pas au travers jusqu'au sol lointain en dessous.
 * }
 */
TEST(PhysiquePersonnageIntegration, ConcaveDePlafondSupportePersonnageParLeDessus) {
    constexpr int CEILING_COLUMN = 3;
    constexpr int CEILING_ROW = 3;
    constexpr int FAR_FLOOR_ROW = 9;  // tres eloigne : prouve qu'il ne s'agit pas d'une coincidence

    core::TileMap map(10, 10);
    for (int column = 0; column < 10; ++column) {
        map.setTile(column, FAR_FLOOR_ROW, core::TileType::Solid);
    }
    map.setTile(CEILING_COLUMN, CEILING_ROW, core::TileType::ConcaveDownRight);

    core::World world;
    const core::Entity player = spawnHumanoid(world, core::GridPosition{CEILING_COLUMN, 0});
    core::CharacterPhysicsSystem system;

    for (int step = 0; step < 150; ++step) {
        system.update(world, map, core::PlayerInput{}, STEP);  // pas d'entree : chute libre
    }

    const core::Player& player_ = world.getComponent<core::Player>(player);
    const core::Transform& transform = world.getComponent<core::Transform>(player);
    EXPECT_TRUE(player_.grounded);
    EXPECT_NEAR(transform.position.y, static_cast<float>(CEILING_ROW) - core::PLAYER_HEIGHT, 0.05f);
}

/**
 * @brief Deux arrondis concaves de **sol** posés côte à côte, orientations opposées
 * (`ConcaveUpRight` puis `ConcaveUpLeft`), forment un pic continu à leur jointure (les deux bords
 * « pleins » se touchent) : le personnage la franchit en marchant sans jamais chuter jusqu'au fond
 * de la vallée voisine (`EX-GP-007`). La jointure entre deux cases non solides doit être vérifiée
 * sur toute la **largeur** de la boîte du personnage, pas seulement son centre — sinon la boîte
 * glisse dans la case voisine (vide de tout appui à cet endroit précis) avant même que son bord
 * n'ait fini de traverser le bord raide (tangente quasi verticale du côté plein) de la case
 * courante, ce qui provoquerait une chute jusqu'au fond de la case suivante.
 * \castest{<b>Deux arrondis concaves de sol adjacents restent praticables à leur jointure, sans
 * chute jusqu'au fond de la case voisine.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage franchit la jointure entre les deux arrondis concaves sans jamais
 * chuter jusqu'au fond de la case voisine (à peine à mi-hauteur, jamais au-delà) et se pose
 * normalement sur le palier bas de l'autre côté.
 * }
 */
TEST(PhysiquePersonnageIntegration, ArrondisConcavesDeSolAdjacentsSansChuteALaJointure) {
    core::World world;
    core::TileMap tiles(8, 8);
    for (int column = 0; column <= 1; ++column) {
        tiles.setTile(column, 6, core::TileType::Solid);
    }
    tiles.setTile(2, 5, core::TileType::ConcaveUpRight);
    tiles.setTile(3, 5, core::TileType::ConcaveUpLeft);
    for (int column = 4; column <= 7; ++column) {
        tiles.setTile(column, 6, core::TileType::Solid);
    }
    const core::Vector2 size = core::playerSize();
    const core::Entity player = world.createEntity();
    world.addComponent(player, core::Transform{core::Vector2{0.3f, 6.0f - size.y}, size, 0.0f});
    world.addComponent(player, core::Velocity{});
    world.addComponent(player, core::Collider{size});
    world.addComponent(player, core::Player{});
    core::CharacterPhysicsSystem system;
    const core::PlayerInput input{1.0f};

    float centerX = 0.0f;
    float maxBottomNearPeak = 0.0f;
    int guard = 0;
    do {
        system.update(world, tiles, input, STEP);
        const core::Transform& t = world.getComponent<core::Transform>(player);
        centerX = t.position.x + size.x * 0.5f;
        const float bottom = t.position.y + size.y;
        // Fenêtre resserrée autour du pic (x = 3, jointure des deux arrondis) : le fond de la case
        // voisine (6,0) est la valeur NORMALE en dehors de cette fenêtre — l'arrondi de droite
        // rejoint par construction le sol bas continuant après lui (colonnes 4-7, même hauteur),
        // donc `bottom` s'en approche légitimement en la quittant. Seule la zone proche du pic doit
        // rester proche de sa hauteur théorique — une chute jusqu'au fond dès la jointure y serait
        // anormale.
        if (centerX >= 2.8f && centerX < 3.2f) {
            maxBottomNearPeak = (std::max)(maxBottomNearPeak, bottom);
        }
        ++guard;
    } while (centerX < 5.0f && guard < 600);

    ASSERT_LT(guard, 600);
    // Valeur théorique au pic (x=3, jointure) : 5,0 pile. Une légère « décroche » près du pic
    // (tangente quasi verticale, courante pour toute courbe aussi raide, voir
    // `TransitionPenteSolPlatSansAACoup`) reste ici sous 5,4 — bien avant le fond de la case
    // voisine (6,0, qui signalerait une chute complète).
    EXPECT_LT(maxBottomNearPeak, 5.4f);
    EXPECT_TRUE(world.getComponent<core::Player>(player).grounded);
    EXPECT_NEAR(world.getComponent<core::Transform>(player).position.y, 6.0f - size.y, 0.05f);
}

/**
 * @brief Un saut bloqué près du bord **fin** (silhouette quasi vide) d'un arrondi concave de
 * plafond reste bloqué DURABLEMENT — le personnage retombe normalement ensuite, il ne se
 * téléporte jamais au-dessus du plafond. Près du bord fin, le blocage a lieu tout près du sommet
 * de la case (silhouette peu épaisse) : le bord bas du personnage reste alors, du fait de sa propre
 * hauteur, encore DANS la même case après le blocage. `core::resolveSlopeFollow` doit donc, sur le
 * pas suivant, distinguer ce chevauchement résiduel d'un véritable atterrissage sur la face du haut
 * de la tuile de plafond (qui ne devrait porter un personnage que tombant dessus **par
 * au-dessus**) — sans cette distinction, le personnage se retrouverait téléporté tout AU-DESSUS du
 * plafond. Vérifie l'invariant à CHAQUE pas (pas seulement au minimum global), puisque ce cas ne se
 * manifeste qu'un pas après le blocage, contrairement à `minY`/`minTopY`.
 * \castest{<b>Un saut bloqué près du bord fin d'un arrondi concave de plafond reste bloqué
 * durablement, sans téléportation au-dessus du plafond au pas suivant.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Après le blocage, le personnage retombe normalement vers le sol lointain en dessous ;
 * sa position ne remonte jamais au-dessus de la ligne de la tuile de plafond.
 * }
 */
TEST(PhysiquePersonnageIntegration, ConcaveDePlafondBordFinResteBloqueSansTeleportation) {
    constexpr int CEILING_COLUMN = 3;
    constexpr int CEILING_ROW = 3;
    constexpr int FLOOR_ROW = 5;

    core::TileMap map(10, 6);
    for (int column = 0; column < 10; ++column) {
        map.setTile(column, FLOOR_ROW, core::TileType::Solid);
    }
    map.setTile(CEILING_COLUMN, CEILING_ROW, core::TileType::ConcaveDownRight);

    core::World world;
    const core::Entity player =
        spawnHumanoid(world, core::GridPosition{CEILING_COLUMN, FLOOR_ROW - 1});
    // offset=0.20 : bord FIN (silhouette quasi vide), le blocage a lieu tout pres du sommet de la
    // case — c'est justement la ou le bord bas du personnage (hauteur 0,8) reste encore dans la
    // meme case apres le blocage (voir la doc du test).
    world.getComponent<core::Transform>(player).position.x =
        static_cast<float>(CEILING_COLUMN) + 0.20f;
    core::CharacterPhysicsSystem system;

    for (int i = 0; i < 10; ++i) {  // se poser au sol avant de sauter
        system.update(world, map, core::PlayerInput{}, STEP);
    }

    bool jumped = false;
    for (int step = 0; step < 30; ++step) {
        const core::Player& pl = world.getComponent<core::Player>(player);
        core::PlayerInput in;
        if (!jumped && pl.grounded) {
            in.jumpPressed = true;
            jumped = true;
        }
        if (jumped) {
            in.jumpHeld = true;
        }
        system.update(world, map, in, STEP);
        // Invariant a chaque pas, pas seulement au minimum global : le personnage ne doit JAMAIS
        // se retrouver au-dessus de la ligne de plafond (row=3) une fois le saut lance.
        const float y = world.getComponent<core::Transform>(player).position.y;
        ASSERT_GT(y, static_cast<float>(CEILING_ROW) - 0.5f) << "step=" << step;
    }
    SUCCEED();
}

/**
 * @brief Sauter tout en marchant (les deux à la fois, comme un joueur réel) **vers son propre bord
 * épais** sous un arrondi concave de plafond bloque toujours le saut, y compris quand marcher
 * pendant la montée déplace la boîte plus vite que le seuil vertical de blocage n'est atteint : la
 * colonne pertinente peut alors « disparaître » d'un pas à l'autre — voire sur PLUSIEURS pas
 * consécutifs quand le seuil est manqué de peu à chaque fois (une mémoire limitée au seul pas
 * précédent ne suffit pas). `core::CharacterPhysicsSystem` mémorise l'étendue horizontale couverte
 * par la boîte depuis le **début de la montée courante** (`Player::ascentSweepMinX/MaxX`), pas
 * seulement le pas précédent. Testé dans les deux orientations (`ConcaveDownRight` en marchant vers
 * la droite, son bord épais ; `ConcaveDownLeft` en marchant vers la gauche, symétrique) — marcher
 * vers son propre bord **fin** n'est volontairement PAS testé ici : s'en éloigner en marchant y
 * ramène légitimement vers une silhouette quasi vide (`EX-GP-007`).
 * \castest{<b>Sauter tout en marchant vers le bord épais d'un arrondi concave de plafond bloque
 * toujours le saut, en combinant les deux mouvements.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le saut reste bloqué par la silhouette épaisse en marchant vers elle pendant la montée
 * — jamais un passage complet jusqu'à l'apogée libre d'un saut non bloqué.
 * }
 */
TEST(PhysiquePersonnageIntegration, ConcaveDePlafondBloqueMemeEnMarchantPendantLeSaut) {
    constexpr int CEILING_COLUMN = 3;
    constexpr int CEILING_ROW = 3;
    constexpr int FLOOR_ROW = 5;

    const auto minTopYReached = [](core::TileType ceilingType, float startX, float moveX) {
        core::TileMap map(10, 6);
        for (int column = 0; column < 10; ++column) {
            map.setTile(column, FLOOR_ROW, core::TileType::Solid);
        }
        map.setTile(CEILING_COLUMN, CEILING_ROW, ceilingType);

        core::World world;
        const core::Entity player =
            spawnHumanoid(world, core::GridPosition{CEILING_COLUMN, FLOOR_ROW - 1});
        world.getComponent<core::Transform>(player).position.x = startX;
        core::CharacterPhysicsSystem system;

        // Se poser au sol AVANT de marcher/sauter : sinon la chute initiale (spawn en l'air, voir
        // GridPosition ci-dessus) déplace déjà le personnage horizontalement avant même le premier
        // saut, faussant la position de départ voulue par ce test.
        for (int i = 0; i < 10; ++i) {
            system.update(world, map, core::PlayerInput{}, STEP);
        }

        bool jumped = false;
        float minTopY = 100.0f;
        for (int step = 0; step < 90; ++step) {
            const core::Player& pl = world.getComponent<core::Player>(player);
            core::PlayerInput in;
            in.moveX = moveX;  // marche PENDANT le saut, comme un joueur réel
            if (!jumped && pl.grounded) {
                in.jumpPressed = true;
                jumped = true;
            }
            if (jumped) {
                in.jumpHeld = true;
            }
            system.update(world, map, in, STEP);
            minTopY = (std::min)(minTopY, world.getComponent<core::Transform>(player).position.y);
        }
        return minTopY;
    };

    // Un saut totalement non bloqué atteindrait ~1,96 à 2,2 (apogée libre) ; bloqué par la
    // silhouette épaisse (h ≳ 0,55 sur ce sous-intervalle), minTopY doit rester nettement en
    // dessous de cette hauteur libre (valeur numériquement plus grande = moins haut).
    // `ConcaveDownRight` (épais à DROITE, x=1) en marchant vers la droite : offsets proches de 1.
    for (float offset = 0.60f; offset <= 0.95f; offset += 0.05f) {
        const float startX = static_cast<float>(CEILING_COLUMN) + offset;
        const float minTop = minTopYReached(core::TileType::ConcaveDownRight, startX, 1.0f);
        EXPECT_GT(minTop, 2.9f) << "ConcaveDownRight, offset=" << offset << " (marche a droite)";
    }
    // `ConcaveDownLeft` (épais à GAUCHE, x=0) en marchant vers la gauche : symétrique.
    for (float offset = 0.05f; offset <= 0.40f; offset += 0.05f) {
        const float startX = static_cast<float>(CEILING_COLUMN) + offset;
        const float minTop = minTopYReached(core::TileType::ConcaveDownLeft, startX, -1.0f);
        EXPECT_GT(minTop, 2.9f) << "ConcaveDownLeft, offset=" << offset << " (marche a gauche)";
    }
    // Même SILHOUETTE, donc même règle : les pentes LINÉAIRES de plafond (`EX-GP-006`, `LOT-26`)
    // sont épaisses d'un côté et fines de l'autre (`h = x` / `h = 1 - x`), comme les arrondis
    // concaves ci-dessus. La règle ne dépend pas du type de tuile mais de cette silhouette, d'où
    // la vérification sur les deux familles.
    for (float offset = 0.60f; offset <= 0.95f; offset += 0.05f) {
        const float startX = static_cast<float>(CEILING_COLUMN) + offset;
        const float minTop = minTopYReached(core::TileType::SlopeDownRight, startX, 1.0f);
        EXPECT_GT(minTop, 2.9f) << "SlopeDownRight, offset=" << offset << " (marche a droite)";
    }
    for (float offset = 0.05f; offset <= 0.40f; offset += 0.05f) {
        const float startX = static_cast<float>(CEILING_COLUMN) + offset;
        const float minTop = minTopYReached(core::TileType::SlopeDownLeft, startX, -1.0f);
        EXPECT_GT(minTop, 2.9f) << "SlopeDownLeft, offset=" << offset << " (marche a gauche)";
    }
}

/**
 * @brief Un arrondi concave de **plafond**, seul sur sa case (voisine vide), bloque toujours un
 * saut visant son bord « plein » (silhouette la plus épaisse, tout près de son propre bord de
 * case). Même exigence que pour le sol (largeur complète de la boîte, pas seulement son centre) :
 * sinon le centre de la boîte glisse dans la case voisine vide (aucune silhouette à vérifier là)
 * avant que son bord n'ait fini de traverser la portion la plus épaisse, laissant un saut passer
 * au travers par en dessous sans être bloqué du tout.
 * \castest{<b>Un arrondi concave de plafond bloque un saut visant son bord plein, même tout près
 * du bord de sa propre case.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le saut est bloqué par la silhouette épaisse près du bord de case, bien avant
 * d'atteindre l'apogée libre d'un saut totalement non bloqué.
 * }
 */
TEST(PhysiquePersonnageIntegration, ConcaveDePlafondBloqueMemePresDuBordDeSaPropreCase) {
    constexpr int CEILING_COLUMN = 3;
    constexpr int CEILING_ROW = 3;
    constexpr int FLOOR_ROW = 5;

    core::TileMap map(10, 6);
    for (int column = 0; column < 10; ++column) {
        map.setTile(column, FLOOR_ROW, core::TileType::Solid);
    }
    map.setTile(CEILING_COLUMN, CEILING_ROW, core::TileType::ConcaveDownRight);
    // Colonne 4 (au-delà du bord droit, le plus epais) volontairement VIDE : sans appui voisin,
    // rien ne peut compenser une colonne mal choisie.

    core::World world;
    const core::Entity player =
        spawnHumanoid(world, core::GridPosition{CEILING_COLUMN, FLOOR_ROW - 1});
    // Bord droit de la boîte (largeur 0,4) à x=4,2 : centre à x=4,0, PILE sur la frontière de case
    // — la colonne qui doit bloquer le saut est la colonne 3 (la vraie tuile, dont le bord PLEIN,
    // silhouette la plus épaisse, touche justement cette frontière), pas la colonne 4 (vide).
    world.getComponent<core::Transform>(player).position.x =
        static_cast<float>(CEILING_COLUMN) + 0.8f;
    core::CharacterPhysicsSystem system;

    bool jumped = false;
    float minY = 100.0f;
    for (int step = 0; step < 90; ++step) {
        const core::Player& pl = world.getComponent<core::Player>(player);
        core::PlayerInput in;
        if (!jumped && pl.grounded) {
            in.jumpPressed = true;
            jumped = true;
        }
        if (jumped) {
            in.jumpHeld = true;
        }
        system.update(world, map, in, STEP);
        minY = (std::min)(minY, world.getComponent<core::Transform>(player).position.y);
    }

    // Un saut totalement non bloqué atteindrait ~2,2 (apogée libre, mesuré sans aucun plafond) ;
    // bloqué par la silhouette épaisse de ce bord (h ≈ 0,4, donc autour de y ≈ 3,4), minY doit
    // rester nettement au-dessus (valeur numériquement plus grande = moins haut) de cet apogée
    // libre.
    EXPECT_GT(minY, 3.0f);
}

/**
 * @brief Le niveau double saut **exige** le saut aérien : un seul saut (au sol) ne suffit pas à
 * franchir le mur (`EX-GP-015`).
 * \castest{<b>Le niveau double saut exige le saut aérien : un seul saut ne suffit pas.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau double saut exige le saut aérien : un seul saut ne suffit pas.
 * }
 */
TEST(PhysiquePersonnageIntegration, NiveauDoubleSautRequiertLeSautAerien) {
    const auto script = [](int, const core::Player& player, float, float) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        in.jumpPressed = player.grounded;  // un seul saut, jamais aerien
        in.jumpHeld = true;
        return in;
    };
    EXPECT_NE(playReactivePhysicsOnly("demo-double-saut.json", script), core::LevelOutcome::Won);
}

/**
 * @brief Le niveau budget **exige** son budget de deux sauts : avec un seul saut autorisé, la
 * seconde marche reste hors d'atteinte (`EX-GP-024`).
 * \castest{<b>Le niveau budget exige ses deux sauts : un seul saut autorisé ne suffit pas.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau budget exige ses deux sauts : un seul saut autorisé ne suffit pas.
 * }
 */
TEST(PhysiquePersonnageIntegration, NiveauBudgetRequiertLesDeuxSauts) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "demo-budget.json";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawnHumanoid(world, level.entry());
    world.getComponent<core::Player>(player).jumpsRemaining = 1;  // budget reduit a un seul saut
    core::CharacterPhysicsSystem system;

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < 3000 && outcome == core::LevelOutcome::Playing; ++step) {
        core::PlayerInput in;
        in.moveX = 1.0f;
        in.jumpPressed = true;
        in.jumpHeld = true;
        system.update(world, level.tileMap(), in, STEP);
        const core::Transform& transform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        outcome = core::evaluateOutcome(
            core::Aabb::fromTopLeftSize(transform.position, collider.size), level);
    }
    EXPECT_NE(outcome, core::LevelOutcome::Won);  // bloque a la seconde marche, budget epuise
}

namespace {

// Niveau minimal (grille vide) portant une seule plateforme mobile de (startCol,startRow) a
// (endCol,endRow), a la vitesse donnee (cases/s).
core::Level makePlatformLevel(int startCol, int startRow, int endCol, int endRow,
                              float speed = 2.0f) {
    core::TileMap map(20, 20);
    map.setTile(startCol, startRow, core::TileType::MovingPlatform);
    std::vector<core::MovingPlatformConfig> platformConfigs{
        core::MovingPlatformConfig{.startPosition = core::GridPosition{startCol, startRow},
                                   .waypoints = {core::GridPosition{endCol, endRow}},
                                   .speed = speed,
                                   .phase = 0}};
    return core::Level("plateforme-integration", std::move(map), core::GridPosition{0, 0},
                       core::GridPosition{19, 19}, {}, -1, -1, {}, {}, {}, std::nullopt,
                       std::nullopt, {}, std::move(platformConfigs));
}

}  // namespace

/**
 * @brief Un personnage au sol sur une plateforme mobile horizontale est porté avec elle : son
 * décalage par rapport à la plateforme reste nul, sans glissement cumulé sur cent pas
 * (`EX-GP-026`).
 * \castest{<b>Une plateforme mobile horizontale porte le personnage sans glissement.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage reste exactement aligné avec la plateforme à chaque pas, sans dérive.
 * }
 */
TEST(PhysiquePersonnageIntegration, PlateformeHorizontalePorteSansGlissement) {
    core::World world;
    const core::Level level = makePlatformLevel(2, 5, 10, 5);
    core::PlatformController platforms(level);
    core::TileMap tiles(20, 20);  // grille vide : seule la plateforme (hors grille) porte
    core::CharacterPhysicsSystem system;

    // Personnage pose exactement sur le dessus de la plateforme a sa position de depart (2,5),
    // deja marque au sol (scenario : le portage etait deja engage avant le debut du test).
    const core::Entity player = spawnPlayer(world, 2.0f, 4.0f);
    world.getComponent<core::Player>(player).grounded = true;

    for (int i = 0; i < 100; ++i) {
        platforms.update();  // la plateforme bouge D'ABORD (ordre de resolution du pas, TACHE-03)
        system.update(world, tiles, core::PlayerInput{}, STEP, platforms.samples());

        const float playerX = world.getComponent<core::Transform>(player).position.x;
        const float platformX = platforms.boxAt(0).min.x;
        EXPECT_NEAR(playerX, platformX, TOLERANCE) << "pas " << i;
        EXPECT_TRUE(world.getComponent<core::Player>(player).grounded) << "pas " << i;
    }
}

/**
 * @brief Un personnage au sol sur une plateforme mobile verticale la suit sans s'enfoncer ni s'en
 * décoller, en montée comme en descente (`EX-GP-026`).
 * \castest{<b>Une plateforme mobile verticale porte le personnage sans enfoncement ni
 * décollement.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le bord bas du personnage reste exactement au niveau du dessus de la plateforme.
 * }
 */
TEST(PhysiquePersonnageIntegration, PlateformeVerticalePorteSansEnfoncementNiDecollement) {
    core::World world;
    const core::Level level = makePlatformLevel(3, 5, 3, 1);  // monte de 4 cases
    core::PlatformController platforms(level);
    core::TileMap tiles(20, 20);
    core::CharacterPhysicsSystem system;

    const core::Entity player = spawnPlayer(world, 3.0f, 4.0f);  // pose sur le dessus (y=5)
    world.getComponent<core::Player>(player).grounded = true;

    for (int i = 0; i < 150; ++i) {  // couvre montee ET descente (cycle = 2s = 120 pas a 2 cases/s)
        platforms.update();
        system.update(world, tiles, core::PlayerInput{}, STEP, platforms.samples());

        const float playerBottom = world.getComponent<core::Transform>(player).position.y + 1.0f;
        const float platformTop = platforms.boxAt(0).min.y;
        EXPECT_NEAR(playerBottom, platformTop, TOLERANCE) << "pas " << i;
    }
}

/**
 * @brief Aucune traversée : à la vitesse de plateforme la plus élevée retenue, le personnage n'est
 * jamais traversé par la plateforme (`EX-GP-014`, préservé pour un obstacle mobile).
 * \castest{<b>Aucune traversée à vitesse de plateforme maximale.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le personnage n'est jamais retrouvé chevauchant la plateforme.
 * }
 */
TEST(PhysiquePersonnageIntegration, AucuneTraverseeAVitesseMaximale) {
    core::World world;
    // Plateforme rapide fonçant droit sur un personnage immobile a proximite de son parcours.
    const core::Level level = makePlatformLevel(0, 5, 12, 5, /*speed=*/8.0f);
    core::PlatformController platforms(level);
    core::TileMap tiles(20, 20);
    core::CharacterPhysicsSystem system;

    const core::Entity player = spawnPlayer(world, 6.0f, 5.0f);  // sur le trajet de la plateforme

    for (int i = 0; i < 200; ++i) {
        platforms.update();
        system.update(world, tiles, core::PlayerInput{}, STEP, platforms.samples());

        const core::Aabb playerBox = core::Aabb::fromTopLeftSize(
            world.getComponent<core::Transform>(player).position, core::Vector2{1.0f, 1.0f});
        const core::Aabb platformBox = platforms.boxAt(0);
        const bool overlapping =
            playerBox.min.x < platformBox.max.x && playerBox.max.x > platformBox.min.x &&
            playerBox.min.y < platformBox.max.y && playerBox.max.y > platformBox.min.y;
        EXPECT_FALSE(overlapping) << "pas " << i;
    }
}

/**
 * @brief Écrasement : une plateforme montante contre un plafond avec le personnage entre les deux
 * est mortelle (`Player::squished`) — décision de cadrage retenue (`TACHE-03`).
 * \castest{<b>Une plateforme montante contre un plafond écrase le personnage.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `Player::squished` devient vrai avant que la plateforme n'atteigne le plafond.
 * }
 */
TEST(PhysiquePersonnageIntegration, EcrasementContreUnPlafondEstMortel) {
    core::World world;
    // Plateforme montant de la ligne 8 vers la ligne 0 ; plafond solide en ligne 1 (juste au-dessus
    // du point d'arrivee) pour laisser au personnage porte le temps de se faire ecraser.
    const core::Level level = makePlatformLevel(3, 8, 3, 0, /*speed=*/2.0f);
    core::PlatformController platforms(level);
    core::TileMap tiles(20, 20);
    for (int column = 0; column < 20; ++column) {
        tiles.setTile(column, 1, core::TileType::Solid);  // plafond bas
    }
    core::CharacterPhysicsSystem system;

    const core::Entity player = spawnPlayer(world, 3.0f, 7.0f);  // pose sur le dessus (y=8)
    world.getComponent<core::Player>(player).grounded = true;

    bool squished = false;
    for (int i = 0; i < 300 && !squished; ++i) {
        platforms.update();
        system.update(world, tiles, core::PlayerInput{}, STEP, platforms.samples());
        squished = world.getComponent<core::Player>(player).squished;
    }
    EXPECT_TRUE(squished);
}

/**
 * @brief Un bloc poussable posé sur une plateforme mobile horizontale est porté avec elle
 * (`core::BlockController`, `EX-GP-026`/`EX-GP-022`).
 * \castest{<b>Un bloc poussable posé sur une plateforme est porté.</b><br/>
 * \tcat Integration · Physique Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La colonne du bloc progresse dans le sens de la plateforme, sans jamais tomber.
 * }
 */
TEST(PhysiquePersonnageIntegration, BlocPousseSurPlateformeEstPorte) {
    core::TileMap map(20, 20);
    // Bloc pose sur le DESSUS de la plateforme (ligne 4, une case au-dessus de la plateforme en
    // ligne 5) -- pas a la meme case (la plateforme n'est pas dans la grille, EX-GP-005).
    map.setTile(4, 4, core::TileType::Block);
    const core::Level level("bloc-plateforme", map, core::GridPosition{0, 0},
                            core::GridPosition{19, 19}, {});
    const core::Level platformLevel = makePlatformLevel(4, 5, 12, 5);

    core::PlatformController platforms(platformLevel);
    core::BlockController blocks(level);
    const core::Aabb noPlayerContact =
        core::Aabb::fromTopLeftSize(core::Vector2{-5.0f, -5.0f}, core::Vector2{1.0f, 1.0f});

    const int startColumn = blocks.positions().front().column;
    for (int i = 0; i < 100; ++i) {
        platforms.update();
        blocks.update(noPlayerContact, /*moveIntentX=*/0.0f, map, platforms.samples());
    }
    // La plateforme a parcouru 8 cases en 100 pas (2 cases/s, 1.67s) : le bloc doit avoir progresse
    // dans le meme sens, jamais etre tombe (reste sur la meme ligne).
    EXPECT_GT(blocks.positions().front().column, startColumn);
    EXPECT_EQ(blocks.positions().front().row, 4);
}
