#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/LinkGesture.h"

namespace {

using core::GridPosition;
using core::TileType;
using hmi::LinkGestureAction;
using hmi::PendingLink;
using hmi::resolveLinkClick;

}  // namespace

// Case cliquee ni declencheur ni cible : ignoree, quelle que soit l'attente.
TEST(LinkGesture, ClicHorsCategorieIgnore) {
    const auto decision =
        resolveLinkClick(std::nullopt, GridPosition{1, 1}, TileType::Solid, false);
    EXPECT_EQ(decision.action, LinkGestureAction::Ignore);
}

// Aucune attente, clic sur un declencheur : la case devient l'attente.
TEST(LinkGesture, PremierClicSurDeclencheurPoseLAttente) {
    const auto decision =
        resolveLinkClick(std::nullopt, GridPosition{2, 3}, TileType::Switch, false);
    EXPECT_EQ(decision.action, LinkGestureAction::SetPending);
    EXPECT_EQ(decision.cell, (GridPosition{2, 3}));
}

// Aucune attente, clic sur une cible : la case devient aussi l'attente (ordre indifferent).
TEST(LinkGesture, PremierClicSurCiblePoseLAttente) {
    const auto decision = resolveLinkClick(std::nullopt, GridPosition{4, 0}, TileType::Door, false);
    EXPECT_EQ(decision.action, LinkGestureAction::SetPending);
    EXPECT_EQ(decision.cell, (GridPosition{4, 0}));
}

// Attente sur un declencheur, clic sur un second declencheur : remplace l'attente.
TEST(LinkGesture, DeuxDeclencheursRemplacentLAttente) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Switch};
    const auto decision =
        resolveLinkClick(pending, GridPosition{1, 0}, TileType::PressurePlate, false);
    EXPECT_EQ(decision.action, LinkGestureAction::ReplacePending);
    EXPECT_EQ(decision.cell, (GridPosition{1, 0}));
}

// Attente sur une cible, clic sur une autre cible : remplace l'attente.
TEST(LinkGesture, DeuxCiblesRemplacentLAttente) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Door};
    const auto decision =
        resolveLinkClick(pending, GridPosition{1, 0}, TileType::DangerSwitched, false);
    EXPECT_EQ(decision.action, LinkGestureAction::ReplacePending);
    EXPECT_EQ(decision.cell, (GridPosition{1, 0}));
}

// Declencheur en attente puis clic sur une cible, pas encore liee : cree la liaison.
TEST(LinkGesture, DeclencheurPuisCibleNonLieeCreeLaLiaison) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Switch};
    const auto decision = resolveLinkClick(pending, GridPosition{3, 3}, TileType::Door, false);
    EXPECT_EQ(decision.action, LinkGestureAction::Link);
    EXPECT_EQ(decision.switchPosition, (GridPosition{0, 0}));
    EXPECT_EQ(decision.targetPosition, (GridPosition{3, 3}));
}

// Cible en attente puis clic sur un declencheur, deja liee : supprime la liaison (bascule).
TEST(LinkGesture, CiblePuisDeclencheurDejaLieSupprimeLaLiaison) {
    const PendingLink pending{GridPosition{3, 3}, TileType::DangerSwitched};
    const auto decision =
        resolveLinkClick(pending, GridPosition{0, 0}, TileType::PressurePlate, true);
    EXPECT_EQ(decision.action, LinkGestureAction::Unlink);
    EXPECT_EQ(decision.switchPosition, (GridPosition{0, 0}));
    EXPECT_EQ(decision.targetPosition, (GridPosition{3, 3}));
}

// Attente perimee (la case en attente ne porte plus un type de liaison, ex. repeinte) : traitee
// comme aucune attente, le clic courant devient la nouvelle attente.
TEST(LinkGesture, AttentePerimeeEstIgnoreeEtRemplacee) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Solid};  // repeint depuis
    const auto decision = resolveLinkClick(pending, GridPosition{3, 3}, TileType::Door, false);
    EXPECT_EQ(decision.action, LinkGestureAction::SetPending);
    EXPECT_EQ(decision.cell, (GridPosition{3, 3}));
}

// Clic hors categorie avec une attente en cours : ignore sans toucher a l'attente.
TEST(LinkGesture, ClicHorsCategorieAvecAttenteIgnore) {
    const PendingLink pending{GridPosition{0, 0}, TileType::Switch};
    const auto decision = resolveLinkClick(pending, GridPosition{1, 1}, TileType::Solid, false);
    EXPECT_EQ(decision.action, LinkGestureAction::Ignore);
}
