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
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelOutcome.h"
#include "Core/Levels/TileMap.h"
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

// Rejoue un niveau jusqu'à son issue (borne large pour éviter une boucle infinie si ça régresse).
// Composition complète, comme `hmi::GameSession::update` : mécanismes (interrupteurs/portes) et
// blocs poussables (pleins ET réduits, `EX-GP-005` — balayage boîte-boîte après la grille) sont
// résolus à chaque pas, que le niveau les utilise ou non (no-op sans mécanisme/bloc).
core::LevelOutcome playLevel(const ScriptedLevel& scripted, int maxSteps = 3000) {
    const std::filesystem::path path =
        std::filesystem::path(PROJECTGAMING_LEVELS_DIR) / scripted.file;
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromFile(path);
    if (!loaded.ok()) {
        return core::LevelOutcome::Lost;  // fichier absent/invalide → échec du parcours
    }
    const core::Level& level = *loaded.level;

    core::World world;
    const core::Entity player = spawn(world, level.entry());
    world.getComponent<core::Player>(player).jumpsRemaining = level.jumpBudget();
    world.getComponent<core::Player>(player).dashesRemaining = level.dashBudget();
    core::CharacterPhysicsSystem system;
    core::BlockController blocks(level);
    core::MechanismController mechanisms(level);

    core::LevelOutcome outcome = core::LevelOutcome::Playing;
    for (int step = 0; step < maxSteps && outcome == core::LevelOutcome::Playing; ++step) {
        const core::Transform& previousTransform = world.getComponent<core::Transform>(player);
        const core::Collider& collider = world.getComponent<core::Collider>(player);
        const core::Aabb previousBox =
            core::Aabb::fromTopLeftSize(previousTransform.position, collider.size);
        const core::PlayerInput in =
            scripted.input(step, world.getComponent<core::Player>(player),
                           previousTransform.position.x, previousTransform.position.y);

        const core::TileMap mechanismMap = mechanisms.collisionMap();
        blocks.update(previousBox, in.moveX, mechanismMap);
        const core::TileMap collision = blocks.collisionMap(mechanismMap);
        system.update(world, collision, in, STEP);

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
        mechanisms.update(box);
        outcome = core::evaluateOutcome(box, level);
    }
    return outcome;
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

// Script constant : avancer et dasher en continu (couloir bas + fosse).
ReactiveInput rightAndDash() {
    return [](int, const core::Player&, float, float) {
        core::PlayerInput in{1.0f};
        in.dashPressed = true;
        return in;
    };
}

}  // namespace

/**
 * @brief Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, puis «
 * retour au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → … → titre du jeu, sur
 * l'intégralité des mécaniques livrées (`LOT-01` à `LOT-24`).
 * \castest{<b>Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre,
 * puis « retour au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → titre du
 * jeu.</b><br/>
 * \tcat Systeme · Parcours Complet<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Parcours complet : chaque niveau de la séquence est franchi (`Won`) dans l'ordre, puis
 * « retour au titre ». Reproduit la boucle titre → niveau 1 → niveau 2 → titre du jeu.
 * }
 */
TEST(ParcoursCompletSysteme, FranchitTouteLaSequence) {
    // États mutables des scripts réactifs (une variable par niveau qui en a besoin) : capturés par
    // référence ci-dessous, jamais par valeur statique, pour rester surs même si ce test était un
    // jour rejoué plusieurs fois dans le même processus.
    bool doubleSautSecondJumpDone = false;
    float wallJumpLastPush = 1.0f;
    bool plaquePressionJumped = false;
    bool blocClimbed = false;
    bool blocCleared = false;
    bool finalSecondJumpDone = false;

    const std::vector<ScriptedLevel> sequence = {
        // 1. Déplacement, chute, sol : escalier descendant, aucun saut nécessaire.
        {"demo-deplacement.json", rightOnly()},
        // 2. Saut simple : un fossé franchissable seulement en sautant.
        {"demo-saut.json", rightAndJump()},
        // 3. Double saut (EX-GP-015) : un mur trop haut pour un seul saut, franchi en enchaînant
        //    saut au sol puis saut aérien juste avant le mur.
        {"demo-double-saut.json",
         [&doubleSautSecondJumpDone](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (player.grounded && x < 6.6f) {
                 // pas encore pres du mur : marcher, pas de saut premature.
             } else if (player.grounded) {
                 in.jumpPressed = true;
                 doubleSautSecondJumpDone = false;
             } else if (!doubleSautSecondJumpDone && x >= 7.2f) {
                 in.jumpPressed = true;
                 doubleSautSecondJumpDone = true;
             }
             return in;
         }},
        // 4. Wall jump/wall slide (EX-GP-016) : puits étroit, franchi en enchaînant les wall jumps
        //    entre les deux parois (script réactif : pousse toujours à l'opposé du dernier mur
        //    touché).
        {"demo-wall-jump.json",
         [&wallJumpLastPush](int, const core::Player& player, float, float) {
             core::PlayerInput in;
             in.jumpPressed = true;
             in.jumpHeld = true;
             if (player.wallDirection != 0.0f) {
                 wallJumpLastPush = -player.wallDirection;
             }
             in.moveX = wallJumpLastPush;
             return in;
         }},
        // 5. Dash (EX-GP-017) : couloir bas (saut impossible) + fosse, franchi en avançant et
        //    dashant.
        {"demo-dash.json", rightAndDash()},
        // 6. Interrupteur ↔ porte (EX-GP-020) : le trajet passe sur l'interrupteur (ouvre la
        //    porte) puis la sortie.
        {"demo-interrupteur.json", rightOnly()},
        // 7. Plaque de pression (EX-GP-025) : ouvre la porte tant qu'un poids y repose. La plaque
        //    est juste avant un mur d'un bloc ; le trajet la recouvre en marchant, ouvrant la
        //    porte au-dessus du mur, puis un saut passe par-dessus avant qu'elle ne se referme.
        {"demo-plaque-pression.json",
         [&plaquePressionJumped](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             if (!plaquePressionJumped && player.grounded && x >= 5.5f) {
                 in.jumpPressed = true;
                 in.jumpHeld = true;
                 plaquePressionJumped = true;
             } else if (plaquePressionJumped) {
                 in.jumpHeld = true;
             }
             return in;
         }},
        // 8. Bloc poussable (EX-GP-022) : poussé contre le mur, sert de marche pour le franchir.
        {"demo-bloc.json",
         [&blocClimbed, &blocCleared](int, const core::Player& player, float x, float y) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (!blocClimbed && player.grounded && x >= 6.2f && y > 5.5f) {
                 in.jumpPressed = true;
                 blocClimbed = true;
             } else if (blocClimbed && !blocCleared && player.grounded && y <= 5.5f) {
                 in.jumpPressed = true;
                 blocCleared = true;
             }
             return in;
         }},
        // 9. Budget de sauts (EX-GP-024) : deux marches ascendantes, exactement deux sauts prévus.
        {"demo-budget.json", rightAndJumpOncePerLanding()},
        // 10. Pente (EX-GP-003, LOT-22) : palier surélevé atteint en marchant, sans saut.
        {"demo-pente.json", rightOnly()},
        // 11. Arrondi (EX-GP-004, LOT-23) : variante courbe de la pente, même principe.
        {"demo-arrondi.json", rightOnly()},
        // 12. Bloc à taille réduite (EX-GP-005, LOT-24) : poussé dans la fosse, comble le chemin à
        //     sa hauteur ; un petit saut franchit le léger ressaut laissé par sa boîte réduite.
        {"demo-bloc-reduit.json",
         [](int, const core::Player& player, float, float) {
             core::PlayerInput in{1.0f};
             in.jumpPressed = player.grounded;
             in.jumpHeld = true;
             return in;
         }},
        // 13. Dangers avancés (EX-GP-050/051/052/053, LOT-31) : directionnel, mobile, commuté et
        //     temporisé sont chacun posés sur une alcôve surélevée **optionnelle**, hors du
        //     couloir principal (au sol) qui mène directement à la sortie — comme les autres
        //     niveaux de cette séquence, aucun scénario de mort n'est exercé ici (déjà couvert aux
        //     niveaux Unit/Integration, `test_danger_controller.cpp`/`test_danger_avance.cpp`) ;
        //     ce niveau ne vérifie que le chargement et la franchissabilité du couloir principal.
        {"demo-dangers-avances.json", rightOnly()},
        // 14. Niveau final : combine dash, pente, bloc poussable, interrupteur/porte et double
        //     saut en un seul parcours cohérent.
        {"demo-final.json",
         [&finalSecondJumpDone](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             if (x < 10.0f) {
                 in.dashPressed = true;
             } else if (x < 13.0f) {
                 // transition + pente : marcher sans sauter (suivi de pente).
             } else if (x < 27.0f) {
                 in.jumpPressed = player.grounded;
                 in.jumpHeld = true;
             } else {
                 if (player.grounded && x < 27.6f) {
                     // pas encore pres du mur final : continuer a marcher.
                 } else if (player.grounded) {
                     in.jumpPressed = true;
                     finalSecondJumpDone = false;
                 } else if (!finalSecondJumpDone && x >= 28.2f) {
                     in.jumpPressed = true;
                     finalSecondJumpDone = true;
                 }
                 in.jumpHeld = true;
             }
             return in;
         }},
        // 15. Niveaux à salles (LOT-32, EX-REN-015) : niveau bien plus grand qu'une salle, en 2×2
        //     salles ; le trajet marche à plat (aucun saut) jusqu'au bord de la première salle,
        //     tombe dans un puits muré (aucune dérive horizontale possible) jusqu'à la salle du
        //     bas, puis marche jusqu'à la sortie — franchit deux frontières de salles.
        {"demo-salles.json", rightOnly()},
    };

    ASSERT_FALSE(sequence.empty());
    for (const ScriptedLevel& level : sequence) {
        EXPECT_EQ(playLevel(level), core::LevelOutcome::Won) << "niveau : " << level.file;
    }
    // Tous les niveaux franchis dans l'ordre → fin de séquence (retour au titre).
}
