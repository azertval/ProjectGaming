// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <functional>
#include <vector>

#include "Core/Ecs/Components/Player.h"
#include "Core/Physics/PlayerInput.h"

/**
 * @file ScriptedLevelSequence.h
 * @brief Séquence des niveaux `demo-*.json` livrés et leur scénario d'entrées déterministe,
 *        extraite de `test_parcours_complet.cpp` (LOT-ANNEXE-08) pour être rejouée à l'identique
 *        par un autre test (`Source/Test/Integration/test_recompense_demo_niveaux.cpp`) sans
 *        dupliquer ces scripts.
 *
 * Header inclus directement (anonymous namespace, comme tout autre fichier de test) : chaque
 * inclusion obtient sa propre copie à liaison interne, aucune violation de l'ODR.
 */

namespace {

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

// Script constant : avancer à droite, rien d'autre (déplacement, pente, arrondi, interrupteur —
// la trajectoire seule suffit, aucun timing particulier).
ReactiveInput rightOnly() {
    return [](int, const core::Player&, float, float) { return core::PlayerInput{1.0f}; };
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

// La séquence jouée et son scénario d'entrées, un par tableau. Fonction plutôt que littéral : les
// appelants (garde de fidélité pas-à-pas, non-régression de récompense) la rejouent tous, et chaque
// appel rend des scripts à l'état NEUF (chaque lambda possède son état par capture-valeur
// `mutable`, jamais partagé).
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
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 6.0f, 0.6f) || atLedge(x, 12.0f, 0.6f) || atLedge(x, 18.0f, 0.6f));
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
             in.dashPressed =
                 atLedge(x, 5.0f, 0.35f) || atLedge(x, 13.0f, 0.35f) || atLedge(x, 21.0f, 0.35f);
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
             in.jumpPressed =
                 player.grounded && ((x >= 4.0f && x <= 4.6f) || (x >= 10.0f && x <= 10.6f));
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
             in.jumpPressed =
                 player.grounded && ((x >= 9.8f && x <= 10.8f) || (x >= 14.8f && x <= 15.8f));
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
        //     meme trace a une tuile pres. Trois marches inclinees a monter, une fosse a franchir
        //     -- c'est elle qui interdit de traverser le tableau en marchant --, puis une descente
        //     par une pente et un arrondi orientes a gauche. Aucun dash : une pente franchissable
        //     a la marche ne l'est pas au dash (defaut moteur consigne, TACHE-06).
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
        //     le saut -- il faut passer SOUS elle. Elles etaient jusqu'ici en ligne 2 au-dessus
        //     d'un couloir en ligne 7, soit deux fois la hauteur d'un saut : le tableau se
        //     traversait en ligne droite sans jamais approcher son sujet.
        {"demo-plafond.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 6.0f, 0.6f) || atLedge(x, 11.0f, 0.6f) || atLedge(x, 16.0f, 0.6f));
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
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 7.0f, 0.6f) || atLedge(x, 15.0f, 0.6f) || atLedge(x, 23.0f, 0.6f));
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
        // 21. Budget de mouvements (EX-GP-024) : le trajet demande EXACTEMENT quatre sauts puis
        //     deux ruees -- un budget borné à quatre (marge d'un saut).
        {"demo-budget.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Quatre marches ascendantes, un saut chacune -- exactement le budget.
             in.jumpPressed =
                 player.grounded && (atLedge(x, 6.0f, 0.6f) || atLedge(x, 12.0f, 0.6f) ||
                                     atLedge(x, 18.0f, 0.6f) || atLedge(x, 24.0f, 0.6f));
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
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 4.0f, 0.7f) || atLedge(x, 17.0f, 0.6f) || atLedge(x, 25.0f, 0.6f));
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
