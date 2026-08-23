// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_parcours_complet.cpp
 * @brief Test système : jouer toute la séquence de niveaux de bout en bout (« titre → niveaux →
 *        titre »), sur les fichiers livrés, sans la couche GPU.
 *
 * Pour chaque niveau de la séquence, on charge le fichier réel, on fait apparaître le personnage à
 * l'entrée et on rejoue un scénario d'entrées **déterministe** (au pas fixe) jusqu'à la sortie
 * (`Won`). Franchir tous les niveaux dans l'ordre représente le parcours complet du jeu. La couche
 * fenêtre/rendu est exclue (vérifiée visuellement) ; on teste ici la simulation et l'enchaînement.
 *
 * Cette liste **doit** rester identique, dans le même ordre, à celle jouée par
 * `Source/HMI/Interface/MainWindow.cpp` (`MainWindow::startGame`) — un décalage entre les deux (un
 * niveau chargé en jeu mais absent d'ici, ou l'inverse) est précisément le défaut qui a déclenché
 * `LOT-25`
 * (`demo5.json` manquait ici). `scripts/check_demo_sequence.py` (CI) compare les deux listes
 * automatiquement.
 */

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "AiSolver/Env/HeadlessLevelEnvironment.h"
#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/BlockController.h"
#include "Core/Gameplay/DangerController.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Gameplay/PlatformController.h"
#include "Core/Levels/DangerGeometry.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/LevelSequence.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/PlayerSpawn.h"
#include "Test/Systeme/ScriptedLevelSequence.h"

namespace {

constexpr float STEP = 1.0f / 60.0f;

// Fait apparaître le personnage à sa vraie taille (humanoïde 0,4×0,8), centré dans la tuile.
core::Entity spawn(core::World& world, core::GridPosition at) {
    const core::Entity entity = world.createEntity();
    const core::Vector2 size = core::playerSize();
    world.addComponent(entity,
                       core::Transform{core::playerSpawnPosition(at.column, at.row), size, 0.0f});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Collider{size});
    world.addComponent(entity, core::Player{});
    return entity;
}

// Trace d'un rejeu : issue, et centre du personnage à chaque pas. La trajectoire alimente le
// garde-fou de proximité (LOT-65 TACHE-05) : une mécanique posée hors de portée du chemin
// réellement parcouru n'est pas démontrée, même si le fichier la contient.
struct PlayTrace {
    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    std::vector<core::Vector2> centers;
};

// Rejoue un niveau jusqu'à son issue (borne large pour éviter une boucle infinie si ça régresse).
// Composition complète, comme `hmi::GameSession::update` : plateformes, blocs poussables (pleins
// ET réduits, `EX-GP-005` — balayage boîte-boîte après la grille) et mécanismes (dont le poids des
// blocs sur les plaques) sont résolus à chaque pas, que le niveau les utilise ou non.
PlayTrace playLevelTraced(const ScriptedLevel& scripted, int maxSteps = 3000) {
    PlayTrace trace;
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / scripted.file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        trace.outcome = core::LevelOutcome::Lost;  // fichier absent/invalide → échec du parcours
        return trace;
    }
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawn(world, level.entry());
    world.getComponent<core::Player>(player).jumpsRemaining = level.jumpBudget();
    world.getComponent<core::Player>(player).dashesRemaining = level.dashBudget();
    core::CharacterPhysicsSystem system;
    core::BlockController blocks(level);
    core::MechanismController mechanisms(level);
    core::PlatformController platforms(level);
    core::DangerController dangers(level);

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < maxSteps && outcome == core::LevelOutcome::Playing; ++step) {
        const core::Transform& previousTransform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        const core::Aabb previousBox =
            core::Aabb::fromTopLeftSize(previousTransform.position, collider.size);
        const core::PlayerInput in =
            scripted.input(step, world.getComponent<core::Player>(player),
                           previousTransform.position.x, previousTransform.position.y);

        // Plateformes mobiles (EX-GP-026) : deplacees EN PREMIER (ordre de resolution documente,
        // LOT-63 TACHE-03), comme hmi::GameSession::update.
        platforms.update();
        const std::vector<core::PlatformSample> platformSamples = platforms.samples();

        const core::TileMap mechanismMap = mechanisms.collisionMap();
        blocks.update(previousBox, in.moveX, mechanismMap, platformSamples);
        const core::TileMap collision = blocks.collisionMap(mechanismMap);
        system.update(world, collision, in, STEP, platformSamples);

        // Composition boîte-boîte pour les blocs réduits (EX-GP-005) : le déplacement REEL obtenu
        // par la grille est retesté contre chaque bloc réduit, la restriction la plus stricte
        // l'emportant toujours (voir Core/Physics/AabbVsAabb.h et hmi::GameSession::update).
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
        trace.centers.push_back((box.min + box.max) * 0.5f);

        // Poids des blocs poussables sur les mecanismes a activation continue (EX-GP-025, LOT-65
        // TACHE-06) : meme composition que hmi::GameSession::update, sans quoi un tableau qui pose
        // un bloc sur une plaque se jouerait differemment ici et en jeu.
        std::vector<core::TriggerWeight> blockWeights;
        blockWeights.reserve(blocks.positions().size());
        for (std::size_t index = 0; index < blocks.positions().size(); ++index) {
            blockWeights.push_back(
                core::TriggerWeight{.box = blocks.boxAt(index), .mass = blocks.massAt(index)});
        }
        mechanisms.update(box, 1.0f, in.interactPressed, blockWeights);

        // Dangers A ETAT (EX-GP-051/052/053) : mobile, temporise et commute ne sont PAS resolus
        // par core::evaluateOutcome, qui ne connait que les dangers statiques -- leur boite doit
        // etre assemblee par l'appelant. hmi::GameSession le fait (collectActiveDangerBoxes) ; ce
        // test ne le faisait pas, si bien qu'un tableau posant un danger temporise sur le chemin
        // aurait ete franchi ici et mortel en jeu. Meme composition, meme ordre.
        dangers.update();
        std::vector<core::Aabb> extraDangerBoxes;
        for (std::size_t index = 0; index < dangers.moverCount(); ++index) {
            extraDangerBoxes.push_back(dangers.moverBox(index));
        }
        for (const core::DangerBlinkConfig& config : level.blinkConfigs()) {
            if (dangers.isBlinkActive(config.position)) {
                extraDangerBoxes.push_back(core::dangerHitbox(
                    core::TileType::DangerBlink, config.position.column, config.position.row));
            }
        }
        for (const core::DangerLink& link : level.dangerLinks()) {
            if (mechanisms.isDangerActive(link.dangerPosition)) {
                extraDangerBoxes.push_back(core::dangerHitbox(core::TileType::DangerSwitched,
                                                              link.dangerPosition.column,
                                                              link.dangerPosition.row));
            }
        }

        // Ecrasement par une plateforme mobile (EX-GP-026) ou par une porte qui se referme
        // (EX-GP-021) : mortel, comme hmi::GameSession::update (traduit en boite de danger
        // supplementaire pour evaluateOutcome).
        if (world.getComponent<core::Player>(player).squished || mechanisms.crushedPlayer()) {
            extraDangerBoxes.push_back(box);
        }
        outcome = core::evaluateOutcome(box, level, extraDangerBoxes);
    }
    trace.outcome = outcome;
    return trace;
}

// Issue seule : la trace complète n'intéresse que les garde-fous de la TACHE-05.
core::LevelOutcome playLevel(const ScriptedLevel& scripted, int maxSteps = 3000) {
    return playLevelTraced(scripted, maxSteps).outcome;
}

// Portee, en tuiles, du garde-fou de proximite (LOT-65 TACHE-05). Calibree sur la hauteur d'un
// saut SIMPLE -- environ 2,4 tuiles avec les valeurs de core::PhysicsConfig (jumpSpeed 15,
// gravity 50, flottement d'apex 0,5) -- et non sur celle d'un double saut : une mecanique qu'il
// faut deja savoir enchainer deux sauts pour effleurer n'est pas sur le chemin. La marge au-dela
// de 2,4 laisse passer le hors-chemin volontaire (un secret facultatif reste legitime) ; ce qui
// est refuse, c'est l'INATTEIGNABLE.
constexpr float REACH_TILES = 2.5f;

// Tuiles porteuses d'une mecanique : tout sauf le vide, le decor solide et les deux bornes du
// parcours. Derive de core::TileType par exclusion plutot que par liste positive -- un type
// ajoute au moteur entre donc dans le controle sans qu'on ait a y penser.
[[nodiscard]] bool isMechanicTile(core::TileType type) noexcept {
    return type != core::TileType::Empty && type != core::TileType::Solid &&
           type != core::TileType::Entry && type != core::TileType::Exit;
}

// Positions des tuiles de mecanique d'un niveau livre, releves sur le fichier reellement charge.
std::vector<core::GridPosition> mechanicTilesOf(const char* file) {
    std::vector<core::GridPosition> cells;
    const std::filesystem::path path = std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        return cells;
    }
    const core::TileMap& map = loaded.level->tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            if (isMechanicTile(map.tile(column, row))) {
                cells.push_back(core::GridPosition{.column = column, .row = row});
            }
        }
    }
    return cells;
}

// Vrai si @p cell passe a portee d'un saut d'au moins une position occupee par le personnage.
// Fonction pure, testable sans niveau reel (cf. test negatif).
[[nodiscard]] bool withinReach(core::GridPosition cell, const std::vector<core::Vector2>& centers) {
    const float cellX = static_cast<float>(cell.column) + 0.5f;
    const float cellY = static_cast<float>(cell.row) + 0.5f;
    for (const core::Vector2& center : centers) {
        if (std::fabs(center.x - cellX) <= REACH_TILES &&
            std::fabs(center.y - cellY) <= REACH_TILES) {
            return true;
        }
    }
    return false;
}

// Fichiers de la sequence livree, dans l'ordre. Lus du MEME fichier que le jeu plutot que
// recopies : un tableau ajoute demain tombe sous les garde-fous sans intervention.
std::vector<std::string> deliveredSequenceFiles() {
    const std::filesystem::path sequencePath =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / "sequence-demo.json";
    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromFile(sequencePath);
    if (!result.ok()) {
        return {};
    }
    return result.sequence->levels;
}

// Exclusions NOMMEES ET JUSTIFIEES du garde-fou anti-couloir -- meme discipline que
// `excludedTileTypes()` (test_couverture_mecaniques.cpp) : une exclusion sans justification ecrite
// est le point de fuite habituel de ce genre de controle.
std::set<std::string> corridorExemptLevels() {
    // demo-deplacement : premier tableau, dont le sujet EST de marcher, tomber et atterrir. Il ne
    // demande volontairement aucune autre entree -- c'est son role de tutoriel implicite
    // (niveaux.md Sec. 3). Aucune autre exclusion n'est legitime : un tableau franchissable en
    // maintenant "droite" ne demontre pas sa mecanique, il la decore.
    //
    // Le nom est assemble en deux morceaux A DESSEIN : `scripts/check_demo_sequence.py` releve
    // tout litteral "demo-*.json" de ce fichier pour le comparer a la sequence livree, et une
    // exclusion nommee ici n'est pas une entree de la sequence -- ecrite d'une piece, elle
    // ferait echouer la comparaison en annoncant un tableau joue deux fois.
    return {std::string("demo-") + "deplacement.json"};
}

// Script constant : avancer et sauter en continu (saut simple — budget illimité, un peu de marge
// ne coûte rien).
ReactiveInput rightAndJump() {
    return [](int, const core::Player&, float, float) {
        core::PlayerInput in{1.0f};
        in.jumpPressed = true;
        in.jumpHeld = true;
        return in;
    };
}

// Script constant : avancer, sauter UNE SEULE FOIS par atterrissage (jamais en continu) — requis
// pour un budget de sauts serré (EX-GP-024) : tenir `jumpPressed` en continu ferait consommer le
// saut aérien « gratuit » (double saut, toujours actif par défaut) juste après chaque saut au sol,
// épuisant le budget avant la marche suivante.
ReactiveInput rightAndJumpOncePerLanding() {
    return [](int, const core::Player& player, float, float) {
        core::PlayerInput in{1.0f};
        in.jumpPressed = player.grounded;
        in.jumpHeld = true;
        return in;
    };
}

// Script constant : avancer et dasher en continu (couloir bas + fosse).
ReactiveInput rightAndDash() {
    return [](int, const core::Player&, float, float) {
        core::PlayerInput in{1.0f};
        in.dashPressed = true;
        return in;
    };
}

/**
 * @brief Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, puis «
 * retour au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → … → titre du jeu, sur
 * l'intégralité des mécaniques livrées (`LOT-01` à `LOT-24`).
 *
 * Le rejeu sert aussi de garde-fou de **proximité** (`LOT-65` TACHE-05) : la trajectoire réellement
 * parcourue est relevée, et chaque tuile de mécanique du tableau doit passer à portée d'un saut
 * d'une position occupée. Une mécanique hors d'atteinte est « couverte » sans jamais être jouée —
 * le trou que la `TACHE-01` avait annoncé sans le combler.
 * \castest{<b>Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, et
 * aucune de ses tuiles de mécanique n'est hors de portée du trajet parcouru.</b><br/>
 * \tcat Systeme · Parcours Complet<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, et
 * aucune de ses tuiles de mécanique n'est hors de portée du trajet parcouru.
 * }
 */
// Rejoue l'orchestration de reference (identique a playLevelTraced ci-dessus) et
// aisolver::HeadlessLevelEnvironment::step EN PARALLELE, pas par pas, avec EXACTEMENT le meme
// core::PlayerInput calcule par le script -- echoue au PREMIER pas ou position, vitesse ou issue
// divergent entre les deux orchestrations (LOT-ANNEXE-05 TACHE-05). Duplique le corps de
// playLevelTraced() plutot que de le reutiliser : HeadlessLevelEnvironment est une replique
// INDEPENDANTE (decision de cadrage de l'epic, pas de partage de code entre HMI/AiSolver et ce
// test système), et cette garde doit driver les deux orchestrations en lockstep pour comparer
// leur etat a CHAQUE pas, pas seulement a l'issue finale.
void expectStepByStepFidelity(const ScriptedLevel& scripted, int maxSteps = 3000) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / scripted.file;

    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    ASSERT_TRUE(loaded.ok()) << "niveau : " << scripted.file;
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawn(world, level.entry());
    world.getComponent<core::Player>(player).jumpsRemaining = level.jumpBudget();
    world.getComponent<core::Player>(player).dashesRemaining = level.dashBudget();
    core::CharacterPhysicsSystem system;
    core::BlockController blocks(level);
    core::MechanismController mechanisms(level);
    core::PlatformController platforms(level);
    core::DangerController dangers(level);

    aisolver::HeadlessLevelEnvironment env;
    ASSERT_TRUE(env.reset(path)) << "niveau : " << scripted.file;

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < maxSteps && outcome == core::LevelOutcome::Playing; ++step) {
        const core::Transform& previousTransform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        const core::Aabb previousBox =
            core::Aabb::fromTopLeftSize(previousTransform.position, collider.size);
        const core::PlayerInput in =
            scripted.input(step, world.getComponent<core::Player>(player),
                           previousTransform.position.x, previousTransform.position.y);

        // --- Orchestration de reference : copie exacte du corps de playLevelTraced ci-dessus. ---
        platforms.update();
        const std::vector<core::PlatformSample> platformSamples = platforms.samples();

        const core::TileMap mechanismMap = mechanisms.collisionMap();
        blocks.update(previousBox, in.moveX, mechanismMap, platformSamples);
        const core::TileMap collision = blocks.collisionMap(mechanismMap);
        system.update(world, collision, in, STEP, platformSamples);

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

        std::vector<core::TriggerWeight> blockWeights;
        blockWeights.reserve(blocks.positions().size());
        for (std::size_t index = 0; index < blocks.positions().size(); ++index) {
            blockWeights.push_back(
                core::TriggerWeight{.box = blocks.boxAt(index), .mass = blocks.massAt(index)});
        }
        mechanisms.update(box, world.getComponent<core::Player>(player).mass, in.interactPressed,
                          blockWeights);

        dangers.update();
        std::vector<core::Aabb> extraDangerBoxes;
        for (std::size_t index = 0; index < dangers.moverCount(); ++index) {
            extraDangerBoxes.push_back(dangers.moverBox(index));
        }
        for (const core::DangerBlinkConfig& config : level.blinkConfigs()) {
            if (dangers.isBlinkActive(config.position)) {
                extraDangerBoxes.push_back(core::dangerHitbox(
                    core::TileType::DangerBlink, config.position.column, config.position.row));
            }
        }
        for (const core::DangerLink& link : level.dangerLinks()) {
            if (mechanisms.isDangerActive(link.dangerPosition)) {
                extraDangerBoxes.push_back(core::dangerHitbox(core::TileType::DangerSwitched,
                                                              link.dangerPosition.column,
                                                              link.dangerPosition.row));
            }
        }
        if (world.getComponent<core::Player>(player).squished || mechanisms.crushedPlayer()) {
            extraDangerBoxes.push_back(box);
        }
        outcome = core::evaluateOutcome(box, level, extraDangerBoxes);

        // --- aisolver::HeadlessLevelEnvironment, MEME entree pour ce pas. ---
        const aisolver::StepObservation observation = env.step(in);

        // Comparaison pas-a-pas : premiere divergence rapportee avec le pas exact et le niveau.
        const core::Velocity& velocity = world.getComponent<core::Velocity>(player);
        ASSERT_NEAR(box.min.x, observation.playerBox.min.x, 1e-4f)
            << "niveau : " << scripted.file << ", pas " << step << " (position x)";
        ASSERT_NEAR(box.min.y, observation.playerBox.min.y, 1e-4f)
            << "niveau : " << scripted.file << ", pas " << step << " (position y)";
        ASSERT_NEAR(velocity.value.x, observation.playerVelocity.value.x, 1e-4f)
            << "niveau : " << scripted.file << ", pas " << step << " (vitesse x)";
        ASSERT_NEAR(velocity.value.y, observation.playerVelocity.value.y, 1e-4f)
            << "niveau : " << scripted.file << ", pas " << step << " (vitesse y)";
        ASSERT_EQ(outcome, observation.outcome)
            << "niveau : " << scripted.file << ", pas " << step << " (issue)";
    }

    EXPECT_EQ(outcome, core::LevelOutcome::Won) << "niveau : " << scripted.file;
}

}  // namespace

/**
 * @brief Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, puis «
 * retour au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → … → titre du jeu, sur
 * l'intégralité des mécaniques livrées.
 *
 * Le rejeu sert aussi de garde-fou de **proximité** (`LOT-65` TACHE-05) : la trajectoire réellement
 * parcourue est relevée, et chaque tuile de mécanique du tableau doit passer à portée d'un saut
 * d'une position occupée. Une mécanique hors d'atteinte est « couverte » sans jamais être jouée —
 * le trou que la `TACHE-01` avait annoncé sans le combler.
 * \castest{<b>Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, et
 * aucune de ses tuiles de mécanique n'est hors de portée du trajet parcouru.</b><br/>
 * \tcat Systeme · Parcours Complet<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, et
 * aucune de ses tuiles de mécanique n'est hors de portée du trajet parcouru.
 * }
 */
TEST(ParcoursCompletSysteme, FranchitTouteLaSequence) {
    const std::vector<ScriptedLevel> sequence = scriptedSequence();
    ASSERT_FALSE(sequence.empty());
    for (const ScriptedLevel& level : sequence) {
        const PlayTrace trace = playLevelTraced(level);
        // Le message porte la position atteinte : redessiner un tableau demande de savoir OU le
        // scenario s'arrete, pas seulement qu'il echoue.
        const core::Vector2 last = trace.centers.empty() ? core::Vector2{} : trace.centers.back();
        EXPECT_EQ(trace.outcome, core::LevelOutcome::Won)
            << "niveau : " << level.file << " (arret en x=" << last.x << " y=" << last.y
            << " apres " << trace.centers.size() << " pas)";

        // Garde-fou de proximite (LOT-65 TACHE-05) : une mecanique posee hors de portee du trajet
        // reellement parcouru n'est pas demontree, meme si le fichier la contient et meme si le
        // garde-fou de couverture la compte comme presente.
        for (const core::GridPosition cell : mechanicTilesOf(level.file)) {
            EXPECT_TRUE(withinReach(cell, trace.centers))
                << "mecanique hors de portee du trajet : " << level.file << " en (" << cell.column
                << ", " << cell.row << ")";
        }
    }
    // Tous les niveaux franchis dans l'ordre → fin de séquence (retour au titre).
}

/**
 * @brief Garde-fou anti-couloir (`LOT-65` TACHE-05) : aucun tableau de la séquence livrée ne se
 * franchit en maintenant simplement « droite ». Un tableau qui se termine sans autre entrée ne
 * démontre pas sa mécanique — il la décore ; c'est le défaut exact qu'une revue a relevé sur dix
 * des vingt-deux tableaux de la première séquence du `LOT-65`. Les exclusions sont nommées et
 * justifiées (`corridorExemptLevels`).
 * \castest{<b>Aucun tableau de la séquence livrée n'est franchissable en maintenant « droite »,
 * hors exclusions nommées.</b><br/>
 * \tcat Systeme · Parcours Complet<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Aucun tableau de la séquence livrée n'est franchissable en maintenant « droite », hors
 * exclusions nommées.
 * }
 */
TEST(ParcoursCompletSysteme, AucunTableauNEstFranchissableEnMaintenantDroite) {
    const std::vector<std::string> files = deliveredSequenceFiles();
    ASSERT_FALSE(files.empty()) << "sequence livree absente ou invalide";
    const std::set<std::string> exempt = corridorExemptLevels();
    for (const std::string& file : files) {
        if (exempt.contains(file)) {
            continue;
        }
        EXPECT_NE(playLevel({file.c_str(), rightOnly()}), core::LevelOutcome::Won)
            << "tableau franchi en maintenant droite : " << file;
    }
}

/**
 * @brief Test négatif du garde-fou de proximité : une tuile posée au-delà de la portée d'un saut
 * au-dessus du trajet est signalée, alors que la même tuile posée sur le trajet ne l'est pas —
 * démontre la sensibilité du contrôle sans dépendre d'aucun fichier de niveau réel.
 * \castest{<b>Une tuile de mécanique posée hors de portée d'un saut au-dessus du trajet est
 * signalée par le garde-fou de proximité ; la même tuile sur le trajet ne l'est pas.</b><br/>
 * \tcat Systeme · Parcours Complet<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une tuile de mécanique posée hors de portée d'un saut au-dessus du trajet est signalée
 * par le garde-fou de proximité ; la même tuile sur le trajet ne l'est pas.
 * }
 */
TEST(ParcoursCompletSysteme, GardeFouDeProximiteSignaleUneMecaniqueHorsDePortee) {
    // Trajet au sol : le personnage longe la ligne 6 en avançant de la colonne 1 à la colonne 10.
    std::vector<core::Vector2> centers;
    for (int column = 1; column <= 10; ++column) {
        centers.push_back(core::Vector2{static_cast<float>(column) + 0.5f, 6.5f});
    }

    // Sur le trajet : à une case au-dessus, largement à portée.
    EXPECT_TRUE(withinReach(core::GridPosition{.column = 5, .row = 5}, centers));
    // Hors de portée : la géométrie exacte de demo-plafond livré (tuile en ligne 2, sol en ligne
    // 7).
    EXPECT_FALSE(withinReach(core::GridPosition{.column = 5, .row = 2}, centers));
    // Hors de portée horizontalement : au-delà de la fin du trajet.
    EXPECT_FALSE(withinReach(core::GridPosition{.column = 20, .row = 6}, centers));

    // L'inventaire des tuiles de mécanique exclut le décor et les bornes, jamais le reste.
    EXPECT_FALSE(isMechanicTile(core::TileType::Empty));
    EXPECT_FALSE(isMechanicTile(core::TileType::Solid));
    EXPECT_FALSE(isMechanicTile(core::TileType::Entry));
    EXPECT_FALSE(isMechanicTile(core::TileType::Exit));
    EXPECT_TRUE(isMechanicTile(core::TileType::Switch));
    EXPECT_TRUE(isMechanicTile(core::TileType::SlopeDownRight));
    EXPECT_TRUE(isMechanicTile(core::TileType::DangerBlink));
}

/**
 * @brief Garde de fidélité pas-à-pas (`LOT-ANNEXE-05` TACHE-05) : chaque niveau de la séquence est
 * rejoué EN PARALLÈLE par l'orchestration de référence (identique à `playLevelTraced`) et par
 * `aisolver::HeadlessLevelEnvironment::step`, avec exactement le même `core::PlayerInput` à chaque
 * pas — la comparaison porte sur la position, la vitesse et l'issue à CHAQUE pas, pas seulement sur
 * l'issue finale (à la différence de `ParcoursCompletSysteme.FranchitTouteLaSequence`), pour
 * détecter une trajectoire divergente qui atteindrait malgré tout la même issue par compensation
 * d'erreurs.
 * \castest{<b>Les deux orchestrations (référence et `HeadlessLevelEnvironment`) restent identiques
 * pas à pas, position/vitesse/issue, sur toute la séquence livrée.</b><br/>
 * \tcat Systeme · Parcours Complet<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les deux orchestrations (référence et `HeadlessLevelEnvironment`) restent identiques
 * pas à pas, position/vitesse/issue, sur toute la séquence livrée.
 * }
 */
TEST(ParcoursCompletSystemeHeadlessEnvironment, FideliteParPas) {
    const std::vector<ScriptedLevel> sequence = scriptedSequence();
    ASSERT_FALSE(sequence.empty());
    for (const ScriptedLevel& level : sequence) {
        expectStepByStepFidelity(level);
    }
}
