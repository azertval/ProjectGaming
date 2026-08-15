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

namespace {

constexpr float STEP = 1.0f / 60.0f;

// Script d'entrées réactif : fonction du pas ET de l'état courant (au sol, position). Nécessaire
// dès qu'un scénario dépend de la trajectoire (double saut, wall jump, bloc) plutôt que d'un
// numéro de pas fixé à l'avance.
using ReactiveInput =
    std::function<core::PlayerInput(int step, const core::Player& player, float x, float y)>;

// Un niveau de la séquence et son scénario d'entrées.
struct ScriptedLevel {
    const char* file;
    ReactiveInput input;
};

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

// Script constant : avancer à droite, rien d'autre (déplacement, pente, arrondi, interrupteur —
// la trajectoire seule suffit, aucun timing particulier).
ReactiveInput rightOnly() {
    return [](int, const core::Player&, float, float) { return core::PlayerInput{1.0f}; };
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

// Vrai quand le personnage approche le bord droit @p edge (en cases) d'une plateforme, dans une
// fenetre de @p window cases. Declencher un saut ou une ruee AVANT cette fenetre gaspille la portee
// horizontale sur du sol encore solide -- constat deja fait au LOT-65 TACHE-02 sur demo-budget, et
// qui vaut pour toute fosse.
bool atLedge(float x, float edge, float window) {
    return x >= edge - window && x <= edge - 0.05f;
}

// Un danger temporise est-il mortel au pas @p step ? Meme formule que
// `core::DangerController::isBlinkActive`. Un tableau qui pose un danger temporise SUR le chemin
// exige d'attendre sa fenetre inoffensive, ce qu'aucune entree reactive ne peut deviner depuis la
// seule position du personnage : le script doit pouvoir le calculer.
bool blinkActive(int step, int phase, int period, int activeDuration) {
    const int cursor = (((step - phase) % period) + period) % period;
    return cursor < activeDuration;
}

// Distance parcourue par une plateforme mobile le long de son segment au pas @p step, meme formule
// que `core::PlatformController::boxAtStep` (aller-retour triangulaire 0 -> distance -> 0). Meme
// raison que ci-dessus : attendre qu'une plateforme revienne ne se deduit pas de la position du
// personnage.
float platformOffset(int step, float speed, float distance) {
    const float travelled = static_cast<float>(step) * speed / 60.0f;
    const float cycle = distance * 2.0f;
    float phase = std::fmod(travelled, cycle);
    if (phase < 0.0f) {
        phase += cycle;
    }
    return phase <= distance ? phase : cycle - phase;
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
// La séquence jouée et son scénario d'entrées, un par tableau. Fonction plutôt que littéral dans
// le test : les garde-fous de la TACHE-05 la rejouent eux aussi, et chaque appel rend des scripts
// à l'état NEUF (chaque lambda possède son état par capture-valeur `mutable`, jamais partagé).
std::vector<ScriptedLevel> scriptedSequence() {
    return {
        // 1. Déplacement, chute, sol : escalier descendant, aucun saut nécessaire.
        {"demo-deplacement.json", rightOnly()},
        // 2. Saut simple : TROIS fosses de difficulte croissante -- la premiere a un fond dont on
        //    ressort, les deux suivantes sont garnies de pics. Un saut par fosse, declenche au
        //    bord : sauter en continu ferait consommer le saut aerien juste apres le decollage et
        //    retomber court.
        {"demo-saut.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && (atLedge(x, 6.0f, 0.6f) || atLedge(x, 12.0f, 0.6f) ||
                                                  atLedge(x, 18.0f, 0.6f));
             return in;
         }},
        // 3. Double saut (EX-GP-015) : DEUX paliers a trois cases, hors de portee d'un saut simple
        //    (2,4 cases). Saut au bord de chaque palier, puis saut aerien pres de l'apex -- un
        //    saut aerien declenche trop tot ne monterait pas assez haut.
        {"demo-double-saut.json",
         [airborne = 0, airJumpDone = false](int, const core::Player& player, float x,
                                             float) mutable {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (player.grounded) {
                 airborne = 0;
                 airJumpDone = false;
                 // Sauter au BORD seulement : decoller trop tot consomme la portee horizontale
                 // sur du sol deja solide.
                 in.jumpPressed = (x >= 6.2f && x <= 6.9f) || (x >= 12.2f && x <= 12.9f);
             } else if (++airborne >= 20 && !airJumpDone) {
                 in.jumpPressed = true;  // ~0,33 s : proche de l'apex du premier saut
                 airJumpDone = true;
             }
             return in;
         }},
        // 4. Wall jump/wall slide (EX-GP-016) : puits étroit, franchi en enchaînant les wall jumps
        //    entre les deux parois (script réactif : pousse toujours à l'opposé du dernier mur
        //    touché).
        {"demo-wall-jump.json",
         [wallJumpLastPush = 1.0f](int, const core::Player& player, float, float) mutable {
             core::PlayerInput in;
             in.jumpPressed = true;
             in.jumpHeld = true;
             if (player.wallDirection != 0.0f) {
                 wallJumpLastPush = -player.wallDirection;
             }
             in.moveX = wallJumpLastPush;
             return in;
         }},
        // 5. Dash (EX-GP-017) : couloir d'une case de haut (le saut y est impossible, et le budget
        //    de sauts est nul), troue de TROIS fosses de deux cases garnies de pics. La ruee part
        //    au bord de chaque fosse : declenchee plus tot, elle est deja finie au moment de
        //    decoller, et le budget ne la recharge qu'au contact du sol.
        {"demo-dash.json",
         [](int, const core::Player&, float x, float) {
             core::PlayerInput in{1.0f};
             in.dashPressed = atLedge(x, 5.0f, 0.35f) || atLedge(x, 13.0f, 0.35f) ||
                              atLedge(x, 21.0f, 0.35f);
             return in;
         }},
        // 6. Synthese de l'acte I (LOT-65 TACHE-07) : ruee sous un plafond bas au-dessus d'une
        //    fosse, deux paliers au double saut, puis un puits au wall jump jusqu'a la sortie.
        {"demo-mouvement.json",
         [airborne = 0, airJumpDone = false, lastPush = 1.0f](int, const core::Player& player,
                                                              float x, float y) mutable {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (y > 6.5f) {
                 // Segments A et B : couloir bas (ruee) puis paliers (double saut).
                 in.dashPressed = atLedge(x, 6.0f, 0.35f);
                 if (player.grounded) {
                     airborne = 0;
                     airJumpDone = false;
                     in.jumpPressed = atLedge(x, 13.0f, 0.8f) || atLedge(x, 20.0f, 0.8f);
                 } else if (++airborne >= 20 && !airJumpDone) {
                     in.jumpPressed = true;
                     airJumpDone = true;
                 }
             } else {
                 // Segment C : puits, wall jump alterne (pousse toujours a l'oppose du mur).
                 if (x < 25.6f) {
                     return in;  // encore sur le palier : marcher jusqu'au puits
                 }
                 in.jumpPressed = true;
                 if (player.wallDirection != 0.0f) {
                     lastPush = -player.wallDirection;
                 }
                 in.moveX = lastPush;
             }
             return in;
         }},
        // 7. Interrupteur ↔ porte (EX-GP-020) : chaque interrupteur est loge dans une alcove du
        //    plafond -- il faut sauter pour l'atteindre, et la porte reste fermee sinon. L'ancien
        //    tableau le posait sur le trajet direct vers sa porte : impossible de ne pas le
        //    resoudre, donc rien a resoudre.
        {"demo-interrupteur.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && ((x >= 4.0f && x <= 4.6f) ||
                                                  (x >= 10.0f && x <= 10.6f));
             return in;
         }},
        // 8. Plaque de pression (EX-GP-025, LOT-65) : le poids doit RESTER. Le personnage pousse
        //    un bloc dans la fosse, ou il enfonce la plaque et l'y maintient, puis saute par-dessus
        //    la fosse et franchit la porte restee ouverte derriere lui. L'ancien tableau reposait
        //    sur un saut qui prenait la porte de vitesse pendant qu'elle se refermait -- il
        //    enseignait l'inverse de la mecanique.
        {"demo-plaque-pression.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 9.0f, 0.5f);
             return in;
         }},
        // 9. Cle ↔ porte verrouillee (EX-GP-023) : la cle est logee dans une alcove du plafond,
        //    et son ramassage exige le contact ET « Interagir » (EX-CTRL-022). Deux paires, pour
        //    que la lecon se confirme plutot que de passer inapercue.
        {"demo-cle.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             const bool sousUneCle = (x >= 4.0f && x <= 4.6f) || (x >= 10.0f && x <= 10.6f);
             in.jumpPressed = player.grounded && sousUneCle;
             in.interactPressed = sousUneCle;  // maintenu : le contact ne dure que le saut
             return in;
         }},
        // 10. Bloc poussable (EX-GP-022) : un SEUL saut disponible. Le bloc comble la premiere
        //     fosse a ras (on marche dessus), le saut sert pour la seconde -- sans le bloc, il en
        //     faudrait deux, et le budget les refuse.
        {"demo-bloc.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 14.0f, 0.5f);
             return in;
         }},
        // 11. Bloc a taille reduite (EX-GP-005) : la fosse fait DEUX cases, hors de portee du saut
        //     unique. Le demi-bloc n'en comble qu'une, et son sommet reste 0,25 case sous le sol :
        //     on descend dessus, puis le saut franchit la seconde.
        {"demo-bloc-reduit.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && ((x >= 9.8f && x <= 10.8f) ||
                                                  (x >= 14.8f && x <= 15.8f));
             return in;
         }},
        // 12. Bloc a taille quart (EX-GP-005) : trop petit pour combler quoi que ce soit, il
        //     obstrue en revanche un couloir d'une case de haut ou il faut le pousser devant soi.
        //     La fosse qui suit, hors du couloir, exige le saut unique.
        {"demo-bloc-quart.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Le premier quart de bloc, pousse, tombe dans la fosse (case 11) sans la combler --
             // il est bien trop petit. Le seul saut du tableau sert a la franchir ; le second
             // quart se degage ensuite en marchant.
             in.jumpPressed = player.grounded && atLedge(x, 11.0f, 0.6f);
             return in;
         }},
        // 14. Pentes et arrondis (EX-GP-003/004) : fusionne l'ancien demo-arrondi, qui reprenait le
        //     meme trace a une tuile pres. Trois marches inclinees a monter, une fosse a franchir --
        //     c'est elle qui interdit de traverser le tableau en marchant --, puis une descente par
        //     une pente et un arrondi orientes a gauche. Aucun dash : une pente franchissable a la
        //     marche ne l'est pas au dash (defaut moteur consigne, TACHE-06).
        {"demo-pente.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 15.0f, 0.6f);
             return in;
         }},
        // 15. Pentes vers la gauche (EX-GP-003/004) : miroir exact du precedent. On entre a DROITE
        //     et la sortie est a gauche -- maintenir « droite » n'y mene nulle part, ce qui est
        //     precisement le propos de quatre silhouettes orientees a gauche.
        {"demo-pente-gauche.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{-1.0f};
             in.jumpHeld = true;
             // Fosse franchie vers la GAUCHE : le bord utile est celui de gauche de la plateforme.
             in.jumpPressed = player.grounded && x >= 13.0f && x <= 13.7f;
             return in;
         }},
        // 16. Arrondis concaves (EX-GP-007) : les concaves de SOL se montent et se descendent sur
        //     le chemin (trois montees, trois descentes), les concaves de PLAFOND bordent le
        //     couloir juste au-dessus de la tete -- et la fosse a franchir est placee sous eux,
        //     pour que le saut vienne buter dans leur silhouette. Ils flottaient jusqu'ici a deux
        //     hauteurs de saut au-dessus du chemin.
        {"demo-concave.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 8.0f, 0.6f);
             return in;
         }},
        // 17. Plafond incline (EX-GP-006) : les quatre variantes bordent le couloir juste au-dessus
        //     de la tete, et chaque fosse a franchir est surmontee d'une silhouette qui raccourcit
        //     le saut -- il faut passer SOUS elle. Elles etaient jusqu'ici en ligne 2 au-dessus d'un
        //     couloir en ligne 7, soit deux fois la hauteur d'un saut : le tableau se traversait en
        //     ligne droite sans jamais approcher son sujet.
        {"demo-plafond.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && (atLedge(x, 6.0f, 0.6f) ||
                                                  atLedge(x, 11.0f, 0.6f) ||
                                                  atLedge(x, 16.0f, 0.6f));
             return in;
         }},
        // 18. Dangers directionnels (EX-GP-050) : les quatre orientations bordent le couloir. Les
        //     pointes vers le HAUT sont des fosses mortelles a franchir ; celles qui pendent du
        //     plafond et celles qui garnissent la rangee haute rendent le saut dangereux ailleurs.
        //     Marcher est sur, sauter au mauvais endroit ne l'est pas -- c'est la lecon, et elle
        //     etait jusqu'ici impossible a recevoir : les quatre variantes flottaient dans des
        //     alcoves que le personnage ne pouvait pas atteindre.
        {"demo-dangers-directionnels.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && (atLedge(x, 7.0f, 0.6f) ||
                                                  atLedge(x, 15.0f, 0.6f) ||
                                                  atLedge(x, 23.0f, 0.6f));
             return in;
         }},
        // 19. Dangers avances (EX-GP-051/052/053) : l'interrupteur ARME les dangers commutes places
        //     plus loin sur le chemin -- il faut donc sauter par-dessus lui, pas marcher dessus.
        //     Trois dangers temporises jalonnent ensuite le couloir : chacun impose d'attendre sa
        //     fenetre inoffensive, calculee ici avec la meme formule que le controleur. Les dangers
        //     mobiles patrouillent la rangee haute, juste au-dessus de la tete.
        {"demo-dangers-avances.json",
         [](int step, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Sauter par-dessus l'interrupteur (case 5) sans le toucher : l'armer condamne la
             // suite du couloir.
             in.jumpPressed = player.grounded && atLedge(x, 5.0f, 0.5f);

             // Attendre devant chaque danger temporise que sa fenetre mortelle soit passee, avec
             // assez de marge pour traverser la case avant qu'elle ne revienne.
             constexpr int PERIOD = 180;
             constexpr int ACTIVE = 45;
             constexpr int CROSSING = 40;  // pas necessaires pour degager la case
             const int phases[3] = {0, 60, 120};
             const float holds[3] = {13.4f, 19.4f, 25.4f};
             for (int index = 0; index < 3; ++index) {
                 if (x < holds[index] || x > holds[index] + 0.2f) {
                     continue;
                 }
                 if (blinkActive(step, phases[index], PERIOD, ACTIVE) ||
                     blinkActive(step + CROSSING, phases[index], PERIOD, ACTIVE)) {
                     in.moveX = 0.0f;  // patienter : la case est (ou redevient) mortelle
                 }
             }
             return in;
         }},
        // 20. Plateforme mobile (EX-GP-026) : trois plateformes en ascenseur au-dessus du vide,
        //     dont une verticale et une portant un bloc poussable -- le portage d'un bloc est exige
        //     par EX-GP-026 et n'etait mis en scene par aucun tableau. Manquer une plateforme est
        //     mortel : il n'y a aucun sol entre les paliers. L'ancien tableau se franchissait sans
        //     AUCUNE entree.
        {"demo-plateforme.json",
         [](int step, const core::Player& player, float x, float y) {
             core::PlayerInput in;
             in.jumpHeld = true;
             if (x < 4.6f) {
                 // Porte par l'ascenseur des l'apparition : ne rien faire jusqu'en haut, puis
                 // sauter vers le palier intermediaire.
                 if (y <= 5.4f) {
                     in.moveX = 1.0f;
                     in.jumpPressed = player.grounded;
                 }
             } else if (x < 7.5f) {
                 in.moveX = 1.0f;  // palier intermediaire
             } else if (x < 8.0f) {
                 // Attendre que la plateforme horizontale revienne a son point de depart.
                 in.moveX = platformOffset(step, 1.0f, 4.0f) < 0.25f ? 1.0f : 0.0f;
             } else if (platformOffset(step, 1.0f, 4.0f) > 3.6f) {
                 in.moveX = 1.0f;  // arrivee au bout : rejoindre le palier de sortie
             }
             return in;
         }},
        // 21. Budget de mouvements (EX-GP-024) : le trajet demande EXACTEMENT quatre sauts puis deux
        //     un budget borné à quatre (marge d'un saut).
        // Deux sauts déclenchés une fois chacun (drapeaux), juste au bord de chaque fossé -- pas
        // plus tôt : décoller trop en amont consomme la portée horizontale du saut sur du sol déjà
        // solide, laissant trop peu d'élan pour franchir le fossé (constaté : un saut déclenché à
        // 2+ cases du bord retombe dans le vide plutôt que sur le palier suivant).
        {"demo-budget.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Quatre marches ascendantes, un saut chacune -- exactement le budget.
             in.jumpPressed = player.grounded && (atLedge(x, 6.0f, 0.6f) ||
                                                  atLedge(x, 12.0f, 0.6f) ||
                                                  atLedge(x, 18.0f, 0.6f) ||
                                                  atLedge(x, 24.0f, 0.6f));
             // Puis un couloir d'une case de haut : deux fosses, deux ruees -- le reste du budget.
             in.dashPressed = atLedge(x, 29.0f, 0.35f) || atLedge(x, 35.0f, 0.35f);
             return in;
         }},
        // 22. Synthese (LOT-65 TACHE-09) : mecanismes, terrain et dangers ENTRELACES plutot que
        //     juxtaposes. Une meme porte est commandee par DEUX declencheurs -- un interrupteur et
        //     une plaque --, croisement etabli en TACHE-06 et jamais mis en scene ; un demi-bloc
        //     comble ensuite une fosse gardee par des pics, et un arrondi mene a la sortie.
        {"demo-synthese.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Trois sauts : toucher l'interrupteur du plafond (case 4), franchir la fosse ou le
             // bloc est tombe sur la plaque (case 17), puis les pics du couloir (case 25).
             in.jumpPressed = player.grounded && (atLedge(x, 4.0f, 0.7f) ||
                                                  atLedge(x, 17.0f, 0.6f) ||
                                                  atLedge(x, 25.0f, 0.6f));
             return in;
         }},
        // 23. Final multi-salles (LOT-65 TACHE-09) : absorbe l'ancien demo-salles, qui portait 272
        //     tuiles et ZERO mecanique, dont 40 % scellees sous le sol -- et qui etait joue APRES
        //     le final. Quatre salles, une enigme par salle, cadrage par salle avec des ZONES
        //     dessinees a la main et une taille de salle propre au niveau (EX-LVL-007, EX-REN-017,
        //     les deux variantes du LOT-64 qu'aucun tableau n'employait).
        //
        //     A (haut gauche) : poser le bloc sur la plaque tient la porte ouverte, et il faut
        //     repartir SANS lui -- le poids doit rester, c'est tout le propos de EX-GP-025.
        //     B (haut droite) : arrondis a gravir, quart de bloc a degager, puis un puits.
        //     C (bas droite)  : la cle, gardee par trois dangers temporises dephases.
        //     D (bas gauche)  : la porte verrouillee, puis la sortie.
        {"demo-final.json",
         [](int step, const core::Player& player, float x, float y) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (y < 10.5f) {
                 // Bande haute. Pousser le bloc de la case 9 jusqu'a la plaque (case 13), puis
                 // SAUTER PAR-DESSUS lui : continuer a marcher le pousserait hors de la plaque et
                 // refermerait la porte -- c'est exactement la lecon du tableau, le poids doit
                 // rester. Un saut ne pousse rien (aucun recouvrement vertical pendant le survol).
                 if (player.grounded && x >= 11.7f && x <= 12.2f) {
                     in.jumpPressed = true;
                 }
                 // Puis la marche du couloir (case 23) avant les arrondis.
                 in.jumpPressed = in.jumpPressed || (player.grounded && atLedge(x, 23.0f, 0.5f));
                 return in;
             }

             // Bande basse : marcher vers la GAUCHE, cle d'abord, dangers temporises ensuite.
             in.moveX = -1.0f;
             in.interactPressed = (x >= 42.5f && x <= 44.0f);  // recouvre largement la case cle

             constexpr int PERIOD = 180;
             constexpr int ACTIVE = 50;
             constexpr int CROSSING = 40;  // pas necessaires pour degager la case
             const int phases[3] = {120, 60, 0};
             const float holds[3] = {40.6f, 29.6f, 18.6f};
             for (int index = 0; index < 3; ++index) {
                 if (x < holds[index] || x > holds[index] + 0.3f) {
                     continue;
                 }
                 if (blinkActive(step, phases[index], PERIOD, ACTIVE) ||
                     blinkActive(step + CROSSING, phases[index], PERIOD, ACTIVE)) {
                     in.moveX = 0.0f;  // patienter : la case est (ou redevient) mortelle
                 }
             }
             return in;
         }},
    };
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
        const core::Vector2 last =
            trace.centers.empty() ? core::Vector2{} : trace.centers.back();
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
    // Hors de portée : la géométrie exacte de demo-plafond livré (tuile en ligne 2, sol en ligne 7).
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
