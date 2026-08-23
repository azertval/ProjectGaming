// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_game_events.cpp
 * @brief Tests unitaires de la détection d'événements de jeu (LOT-60, EX-REN-047).
 */

#include <algorithm>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Game/GameEvents.h"

namespace {

using hmi::GameEvent;
using hmi::MechanismEventState;
using hmi::PlayerEventState;

bool contains(const std::vector<GameEvent>& events, GameEvent event) {
    return std::find(events.begin(), events.end(), event) != events.end();
}

}  // namespace

/**
 * @brief Un personnage dont rien ne change ne produit aucun evenement.
 * \castest{<b>Un etat stable du personnage ne produit aucun evenement.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Detecter les transitions entre deux etats identiques.<br/>
 * \tattendu La liste d'evenements est vide.
 * }
 */
TEST(GameEventsTest, EtatStableNeProduitAucunEvenement) {
    const PlayerEventState state{
        .grounded = true, .dashTimer = 0.0F, .wallDirection = 0.0F, .justJumped = false};
    EXPECT_TRUE(hmi::detectPlayerEvents(state, state).empty());
}

/**
 * @brief Le front justJumped produit exactement l'evenement Jumped.
 * \castest{<b>Un saut produit exactement un evenement Jumped.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. current.justJumped = true, tout le reste inchange.<br/>
 * \tattendu La liste contient exactement un evenement : Jumped.
 * }
 */
TEST(GameEventsTest, SautProduitJumped) {
    const PlayerEventState previous{.grounded = true, .justJumped = false};
    const PlayerEventState current{.grounded = true, .justJumped = true};

    const std::vector<GameEvent> events = hmi::detectPlayerEvents(previous, current);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::Jumped);
}

/**
 * @brief Le front sol (etait en l'air, est au sol) produit exactement l'evenement Landed.
 * \castest{<b>Un atterrissage produit exactement un evenement Landed.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. previous.grounded = false, current.grounded = true.<br/>
 * \tattendu La liste contient exactement un evenement : Landed.
 * }
 */
TEST(GameEventsTest, AtterrissageProduitLanded) {
    const PlayerEventState previous{.grounded = false};
    const PlayerEventState current{.grounded = true};

    const std::vector<GameEvent> events = hmi::detectPlayerEvents(previous, current);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::Landed);
}

/**
 * @brief Decoller du sol (grounded true -> false) ne produit pas Landed.
 * \castest{<b>Decoller du sol ne produit pas l'evenement Landed.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. previous.grounded = true, current.grounded = false.<br/>
 * \tattendu Aucun evenement Landed dans la liste (Landed est un front d'entree au sol, pas de
 * sortie).
 * }
 */
TEST(GameEventsTest, DecollerNeProduitPasLanded) {
    const PlayerEventState previous{.grounded = true};
    const PlayerEventState current{.grounded = false};

    EXPECT_FALSE(contains(hmi::detectPlayerEvents(previous, current), GameEvent::Landed));
}

/**
 * @brief Le front du minuteur de dash (<=0 -> >0) produit exactement l'evenement Dashed.
 * \castest{<b>Un dash produit exactement un evenement Dashed.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. previous.dashTimer = 0, current.dashTimer = 0.2 (dash qui vient de se
 * declencher).<br/>
 * \tattendu La liste contient exactement un evenement : Dashed.
 * }
 */
TEST(GameEventsTest, DeclenchementDashProduitDashed) {
    const PlayerEventState previous{.dashTimer = 0.0F};
    const PlayerEventState current{.dashTimer = 0.2F};

    const std::vector<GameEvent> events = hmi::detectPlayerEvents(previous, current);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::Dashed);
}

/**
 * @brief Le decompte du minuteur de dash (>0 -> >0 plus petit) ne re-produit pas Dashed.
 * \castest{<b>Le decompte d'un dash en cours ne re-produit pas Dashed.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. previous.dashTimer = 0.2, current.dashTimer = 0.1 (dash toujours en cours).<br/>
 * \tattendu Aucun evenement Dashed : le front n'a lieu qu'au declenchement, pas a chaque pas du
 * dash.
 * }
 */
TEST(GameEventsTest, DecompteDashNeReproduitPasDashed) {
    const PlayerEventState previous{.dashTimer = 0.2F};
    const PlayerEventState current{.dashTimer = 0.1F};

    EXPECT_TRUE(hmi::detectPlayerEvents(previous, current).empty());
}

/**
 * @brief Le contact avec un mur (wallDirection 0 -> non nul) produit WallContactEnter.
 * \castest{<b>Un contact mural produit exactement un evenement WallContactEnter.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. previous.wallDirection = 0, current.wallDirection = 1.<br/>
 * \tattendu La liste contient exactement un evenement : WallContactEnter.
 * }
 */
TEST(GameEventsTest, ContactMuralProduitWallContactEnter) {
    const PlayerEventState previous{.wallDirection = 0.0F};
    const PlayerEventState current{.wallDirection = 1.0F};

    const std::vector<GameEvent> events = hmi::detectPlayerEvents(previous, current);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::WallContactEnter);
}

/**
 * @brief Un dash au contact d'un mur produit deux evenements distincts, jamais fusionnes.
 * \castest{<b>Un dash au contact d'un mur produit Dashed ET WallContactEnter.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Faire transitionner dashTimer et wallDirection dans le meme pas.<br/>
 * \tattendu La liste contient les deux evenements (priorite laissee a l'appelant, TACHE-03).
 * }
 */
TEST(GameEventsTest, DashEtMurDansLeMemePasProduisentDeuxEvenements) {
    const PlayerEventState previous{.dashTimer = 0.0F, .wallDirection = 0.0F};
    const PlayerEventState current{.dashTimer = 0.2F, .wallDirection = -1.0F};

    const std::vector<GameEvent> events = hmi::detectPlayerEvents(previous, current);
    EXPECT_TRUE(contains(events, GameEvent::Dashed));
    EXPECT_TRUE(contains(events, GameEvent::WallContactEnter));
}

/**
 * @brief Un interrupteur (non continu) qui bascule produit SwitchToggled.
 * \castest{<b>Un interrupteur bascule produit l'evenement SwitchToggled.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Un mecanisme non continu passe de ferme a ouvert.<br/>
 * \tattendu La liste contient exactement un evenement : SwitchToggled.
 * }
 */
TEST(GameEventsTest, InterrupteurBasculeProduitSwitchToggled) {
    const MechanismEventState previous{.doorOpen = {false}};
    const MechanismEventState current{.doorOpen = {true}};
    const std::vector<bool> continuous = {false};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::SwitchToggled);
}

/**
 * @brief Une plaque de pression enfoncee produit PressurePlatePressed, pas SwitchToggled.
 * \castest{<b>Une plaque de pression enfoncee produit PressurePlatePressed.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Un mecanisme continu passe de ferme a ouvert.<br/>
 * \tattendu La liste contient exactement un evenement : PressurePlatePressed.
 * }
 */
TEST(GameEventsTest, PlaquePresseeProduitPressurePlatePressed) {
    const MechanismEventState previous{.doorOpen = {false}};
    const MechanismEventState current{.doorOpen = {true}};
    const std::vector<bool> continuous = {true};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::PressurePlatePressed);
}

/**
 * @brief Une plaque de pression relachee produit PressurePlateReleased.
 * \castest{<b>Une plaque de pression relachee produit PressurePlateReleased.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Un mecanisme continu passe d'ouvert a ferme.<br/>
 * \tattendu La liste contient exactement un evenement : PressurePlateReleased.
 * }
 */
TEST(GameEventsTest, PlaqueRelacheeProduitPressurePlateReleased) {
    const MechanismEventState previous{.doorOpen = {true}};
    const MechanismEventState current{.doorOpen = {false}};
    const std::vector<bool> continuous = {true};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::PressurePlateReleased);
}

/**
 * @brief Un mecanisme dont l'etat ne change pas ne produit aucun evenement.
 * \castest{<b>Un mecanisme stable ne produit aucun evenement.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. previous et current identiques pour un jeu de mecanismes.<br/>
 * \tattendu La liste d'evenements est vide.
 * }
 */
TEST(GameEventsTest, MecanismeStableNeProduitRien) {
    const MechanismEventState state{.doorOpen = {true, false, true}};
    const std::vector<bool> continuous = {false, true, false};

    EXPECT_TRUE(hmi::detectMechanismEvents(state, state, continuous).empty());
}

/**
 * @brief Seul le mecanisme qui transitionne produit un evenement, les autres restent muets.
 * \castest{<b>Un seul mecanisme sur plusieurs qui change produit un seul evenement.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trois mecanismes, un seul change d'etat.<br/>
 * \tattendu Exactement un evenement, correspondant au mecanisme qui a change.
 * }
 */
TEST(GameEventsTest, SeulLeMecanismeQuiChangeProduitUnEvenement) {
    const MechanismEventState previous{.doorOpen = {false, false, false}};
    const MechanismEventState current{.doorOpen = {false, true, false}};
    const std::vector<bool> continuous = {false, false, false};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::SwitchToggled);
}

/**
 * @brief Une taille incoherente entre les vecteurs ignore l'index en trop sans planter.
 * \castest{<b>Des vecteurs de tailles differentes ne provoquent aucun acces hors bornes.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. previous a 2 elements, current en a 3, isContinuous en a 2.<br/>
 * \tattendu Aucun plantage ; seuls les index communs aux trois vecteurs sont consideres.
 * }
 */
TEST(GameEventsTest, TaillesIncoherentesNIgnorentSansPlanter) {
    const MechanismEventState previous{.doorOpen = {false, false}};
    const MechanismEventState current{.doorOpen = {false, false, true}};
    const std::vector<bool> continuous = {false, false};

    EXPECT_TRUE(hmi::detectMechanismEvents(previous, current, continuous).empty());
}

/**
 * @brief L'issue Playing ne produit aucun evenement.
 * \castest{<b>Une partie en cours ne produit aucun evenement d'issue.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Traduire core::LevelOutcome::Playing.<br/>
 * \tattendu std::nullopt.
 * }
 */
TEST(GameEventsTest, IssueEnCoursNeProduitRien) {
    EXPECT_EQ(hmi::detectOutcomeEvent(core::LevelOutcome::Playing), std::nullopt);
}

/**
 * @brief L'issue Won produit LevelCompleted.
 * \castest{<b>Une issue gagnee produit l'evenement LevelCompleted.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Traduire core::LevelOutcome::Won.<br/>
 * \tattendu GameEvent::LevelCompleted.
 * }
 */
TEST(GameEventsTest, IssueGagneeProduitLevelCompleted) {
    EXPECT_EQ(hmi::detectOutcomeEvent(core::LevelOutcome::Won), GameEvent::LevelCompleted);
}

/**
 * @brief L'issue Lost produit Died.
 * \castest{<b>Une issue perdue produit l'evenement Died.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Traduire core::LevelOutcome::Lost.<br/>
 * \tattendu GameEvent::Died.
 * }
 */
TEST(GameEventsTest, IssuePerdueProduitDied) {
    EXPECT_EQ(hmi::detectOutcomeEvent(core::LevelOutcome::Lost), GameEvent::Died);
}
