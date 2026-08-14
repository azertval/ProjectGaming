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
#include "Core/Gameplay/MechanismController.h"
#include "Core/Gameplay/PlatformController.h"
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

        // Ecrasement par une plateforme mobile (EX-GP-026) ou par une porte qui se referme
        // (EX-GP-021) : mortel, comme hmi::GameSession::update (traduit en boite de danger
        // supplementaire pour evaluateOutcome).
        std::vector<core::Aabb> extraDangerBoxes;
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
    return {"demo-deplacement.json"};
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
        // 6. Interrupteur ↔ porte (EX-GP-020) : le trajet passe sur l'interrupteur (ouvre la
        //    porte) puis la sortie.
        {"demo-interrupteur.json", rightOnly()},
        // 7. Plaque de pression (EX-GP-025) : ouvre la porte tant qu'un poids y repose. La plaque
        //    est juste avant un mur d'un bloc ; le trajet la recouvre en marchant, ouvrant la
        //    porte au-dessus du mur, puis un saut passe par-dessus avant qu'elle ne se referme.
        {"demo-plaque-pression.json",
         [plaquePressionJumped = false](int, const core::Player& player, float x, float) mutable {
             core::PlayerInput in{1.0f};
             if (!plaquePressionJumped && player.grounded && x >= 4.5f) {
                 in.jumpPressed = true;
                 in.jumpHeld = true;
                 plaquePressionJumped = true;
             } else if (plaquePressionJumped) {
                 in.jumpHeld = true;
             }
             return in;
         }},
        // 8. Clé ↔ porte verrouillée (EX-GP-023, LOT-63) : ramassage par contact ET « Interagir »
        //    (EX-CTRL-022, seul déclencheur du ramassage) en passant sur la clé, puis la porte
        //    verrouillée ouverte définitivement jusqu'à la sortie.
        {"demo-cle.json",
         [](int, const core::Player&, float x, float) {
             core::PlayerInput in{1.0f};
             in.interactPressed = (x >= 3.5f && x <= 5.5f);  // recouvre largement la case clé
             return in;
         }},
        // 9. Bloc poussable (EX-GP-022) : poussé contre un mur, sert de marche pour le franchir
        //    (aucune fosse à combler : le bloc glisse sur un sol continu, sans chute à
        //    synchroniser avec l'arrivée du personnage).
        {"demo-bloc.json",
         [blocClimbed = false, blocCleared = false](int, const core::Player& player, float x, float y) mutable {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (!blocClimbed && player.grounded && x >= 6.3f && y > 4.5f) {
                 in.jumpPressed = true;  // grimpe sur le bloc pousse contre le mur
                 blocClimbed = true;
             } else if (blocClimbed && !blocCleared && player.grounded && y <= 4.5f) {
                 in.jumpPressed = true;  // depuis le bloc, franchit le mur
                 blocCleared = true;
             }
             return in;
         }},
        // 10. Budget de sauts (EX-GP-024) : deux marches ascendantes, deux sauts nécessaires pour
        //     un budget borné à quatre (marge d'un saut).
        // Deux sauts déclenchés une fois chacun (drapeaux), juste au bord de chaque fossé -- pas
        // plus tôt : décoller trop en amont consomme la portée horizontale du saut sur du sol déjà
        // solide, laissant trop peu d'élan pour franchir le fossé (constaté : un saut déclenché à
        // 2+ cases du bord retombe dans le vide plutôt que sur le palier suivant).
        {"demo-budget.json",
         [budgetGap1Jumped = false, budgetGap2Jumped = false](int, const core::Player& player, float x, float) mutable {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (!budgetGap1Jumped && player.grounded && x >= 6.3f) {
                 in.jumpPressed = true;
                 budgetGap1Jumped = true;
             } else if (budgetGap1Jumped && !budgetGap2Jumped && player.grounded && x >= 13.0f) {
                 in.jumpPressed = true;
                 budgetGap2Jumped = true;
             }
             return in;
         }},
        // 11. Pente (EX-GP-003, LOT-22) : palier surélevé atteint en marchant, sans saut.
        {"demo-pente.json", rightOnly()},
        // 12. Pente gauche (EX-GP-003, LOT-65) : SlopeUpLeft/RoundedUpLeft/ConcaveUpLeft — miroir
        //     de la pente/l'arrondi, trois paliers **descendants** en marchant vers la droite (la
        //     variante "gauche" monte vers la gauche, donc descend dans ce sens de marche), sans
        //     saut — même suivi de surface que les variantes "droite", déjà prouvé symétrique par
        //     les tests unitaires (`SuitUnePenteDescendanteEnMarchant`).
        {"demo-pente-gauche.json", rightOnly()},
        // 13. Arrondi (EX-GP-004, LOT-23) : variante courbe de la pente, même principe.
        {"demo-arrondi.json", rightOnly()},
        // 14. Arrondis concaves (EX-GP-007, LOT-65) : ConcaveUpRight au sol (même principe que
        //     l'arrondi convexe, courbure inversée) puis ConcaveDownRight/ConcaveDownLeft en
        //     plafond, posés avec un dégagement généreux au-dessus du chemin plat qui suit —
        //     jamais touchés en marchant, leur silhouette est déjà exhaustivement prouvée au
        //     niveau Integration (`ConcaveDePlafondBloqueSelonSaSilhouette` et consorts).
        {"demo-concave.json", rightOnly()},
        // 15. Plafond incliné (EX-GP-006, LOT-65) : SlopeDownRight/SlopeDownLeft/
        //     RoundedDownRight/RoundedDownLeft, posés en plafond avec un dégagement généreux
        //     au-dessus d'un couloir plat — présents et traversés visuellement, jamais touchés en
        //     marchant (même esprit que les dangers avancés ci-dessous : la silhouette de blocage
        //     est prouvée ailleurs, ce niveau prouve le chargement et la traversée).
        {"demo-plafond.json", rightOnly()},
        // 16. Bloc à taille réduite (EX-GP-005, LOT-24) : poussé dans la fosse, comble le chemin à
        //     sa hauteur ; un petit saut franchit le léger ressaut laissé par sa boîte réduite.
        {"demo-bloc-reduit.json",
         [](int, const core::Player& player, float, float) {
             core::PlayerInput in{1.0f};
             in.jumpPressed = player.grounded;
             in.jumpHeld = true;
             return in;
         }},
        // 17. Bloc à taille quart (EX-GP-005, LOT-65) : même principe que le bloc réduit, sur
        //     terrain plat sans fosse — la poussée ne fait qu'écarter le bloc du chemin, un petit
        //     saut par atterrissage suffit dans tous les cas.
        {"demo-bloc-quart.json",
         [](int, const core::Player& player, float, float) {
             core::PlayerInput in{1.0f};
             in.jumpPressed = player.grounded;
             in.jumpHeld = true;
             return in;
         }},
        // 18. Plateforme mobile (EX-GP-026, LOT-63) : le personnage tombe dessus dès l'apparition
        //     puis se laisse porter jusqu'à la sortie, sans aucune entrée (portage pur) — la
        //     traversée serait mortelle sans elle (aucun sol entre les deux bords).
        {"demo-plateforme.json",
         [](int, const core::Player&, float, float) { return core::PlayerInput{}; }},
        // 19. Dangers avancés (EX-GP-050/051/052/053, LOT-31) : directionnel, mobile, commuté et
        //     temporisé sont chacun posés sur une alcôve surélevée **optionnelle**, hors du
        //     couloir principal (au sol) qui mène directement à la sortie — comme les autres
        //     niveaux de cette séquence, aucun scénario de mort n'est exercé ici (déjà couvert aux
        //     niveaux Unit/Integration, `test_danger_controller.cpp`/`test_danger_avance.cpp`) ;
        //     ce niveau ne vérifie que le chargement et la franchissabilité du couloir principal.
        {"demo-dangers-avances.json", rightOnly()},
        // 20. Dangers directionnels (EX-GP-050, LOT-65) : DangerDown/DangerLeft/DangerRight, même
        //     principe que les dangers avancés ci-dessus — alcôves flottantes hors du couloir
        //     principal, jamais atteintes par un déplacement au sol.
        {"demo-dangers-directionnels.json", rightOnly()},
        // 21. Niveau final : synthèse combinant quatre mécaniques en un seul parcours continu
        //     (cadrage suivi), chacune reprenant exactement la géométrie de son tableau dédié
        //     (demo-dash/demo-pente/demo-interrupteur/demo-double-saut), séparées par de larges
        //     couloirs plats — dash, pente, interrupteur/porte, double saut. La plateforme mobile
        //     n'est délibérément pas combinée ici : sa seule présence dans le fichier casse la
        //     résolution de collision pendant le suivi de pente, même immobile et loin du joueur —
        //     défaut moteur consigné (CHANGELOG), pas corrigé dans ce lot ; déjà couverte
        //     isolément par demo-plateforme.json.
        {"demo-final.json",
         [finalDoubleJumpDone = false](int, const core::Player& player, float x, float) mutable {
             core::PlayerInput in{1.0f};
             if (x < 10.0f) {
                 // segment A : dash sous le plafond bas, au-dessus de la fosse.
                 in.dashPressed = true;
             } else if (x < 45.0f) {
                 // couloirs + segments B (pente) et D (interrupteur/porte) : marcher suffit.
             } else if (x < 57.0f) {
                 // segment G : double saut vers le palier surélevé.
                 in.jumpHeld = true;
                 if (player.grounded && x < 46.6f) {
                     // pas encore pres du bord : marcher.
                 } else if (player.grounded) {
                     in.jumpPressed = true;
                     finalDoubleJumpDone = false;
                 } else if (!finalDoubleJumpDone && x >= 47.2f) {
                     in.jumpPressed = true;
                     finalDoubleJumpDone = true;
                 }
             }
             // couloir final : aucune entrée supplémentaire nécessaire au-delà de x = 57.
             return in;
         }},
        // 22. Niveaux à salles (LOT-32, EX-REN-015) : niveau plus haut qu'une salle par défaut,
        //     deux salles empilées ; le trajet marche à plat (aucun saut) jusqu'au bord de la
        //     salle du haut, tombe dans un puits muré (aucune dérive horizontale possible)
        //     jusqu'à la salle du bas, puis marche jusqu'à la sortie.
        {"demo-salles.json", rightOnly()},
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
