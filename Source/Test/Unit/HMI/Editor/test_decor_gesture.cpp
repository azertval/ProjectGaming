/**
 * @file test_decor_gesture.cpp
 * @brief Tests unitaires de la machine à états du geste de manipulation de décors (LOT-50
 *        TACHE-02, `EX-DEC-010`).
 */

#include <gtest/gtest.h>

#include <cmath>
#include <optional>
#include <vector>

#include "Core/Levels/Decor.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/DecorGesture.h"

namespace {

using core::Decor;
using core::Rect;
using core::Vector2;
using hmi::DecorGestureActionKind;
using hmi::DecorGestureState;
using hmi::DecorHandle;
using hmi::DecorHit;

constexpr float TWO_PI = 6.28318530717958647692f;

}  // namespace

// ---------------------------------------------------------------------------------------------
// designateDecorAt
// ---------------------------------------------------------------------------------------------

TEST(DecorGestureTest, DesignateDecorAtChoisitLePlusAuDessusEnCasDeSuperposition) {
    const std::vector<Rect> bounds{
        Rect{Vector2{0.0f, 0.0f}, Vector2{4.0f, 4.0f}},  // dessous (rang 0)
        Rect{Vector2{1.0f, 1.0f}, Vector2{4.0f, 4.0f}},  // dessus (rang 1, superpose)
    };

    const std::optional<DecorHit> hit =
        hmi::designateDecorAt(Vector2{2.0f, 2.0f}, bounds, std::nullopt, std::nullopt);

    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(hit->index, 1u);
    EXPECT_EQ(hit->handle, DecorHandle::Body);
}

TEST(DecorGestureTest, DesignateDecorAtDansLeVideRenvoieNullopt) {
    const std::vector<Rect> bounds{Rect{Vector2{0.0f, 0.0f}, Vector2{1.0f, 1.0f}}};

    const std::optional<DecorHit> hit =
        hmi::designateDecorAt(Vector2{99.0f, 99.0f}, bounds, std::nullopt, std::nullopt);

    EXPECT_FALSE(hit.has_value());
}

TEST(DecorGestureTest, DesignateDecorAtPrivilegieLesPoigneesDuDecorSelectionne) {
    // Decor 0 selectionne, ses poignees couvrent (0,0). Decor 1 (non selectionne) a son corps la
    // aussi -- sans priorite aux poignees, le clic viserait le corps du decor 1.
    const std::vector<Rect> bounds{
        Rect{Vector2{0.0f, 0.0f}, Vector2{4.0f, 4.0f}},
        Rect{Vector2{-1.0f, -1.0f}, Vector2{4.0f, 4.0f}},
    };
    const hmi::DecorHandleLayout handles = hmi::decorHandleLayout(bounds[0], 0.1f);

    const std::optional<DecorHit> hit =
        hmi::designateDecorAt(Vector2{0.0f, 0.0f}, bounds, 0u, handles);

    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(hit->index, 0u);
    EXPECT_EQ(hit->handle, DecorHandle::TopLeft);
}

// ---------------------------------------------------------------------------------------------
// Clic contre glisser (seuil)
// ---------------------------------------------------------------------------------------------

TEST(DecorGestureTest, BeginSelectionneImmediatementMemeSansGlisser) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};

    hmi::beginDecorGesture(state, DecorHit{2, DecorHandle::Body}, Vector2{1.5f, 1.5f}, decor,
                           bounds);

    ASSERT_TRUE(state.selectedIndex.has_value());
    EXPECT_EQ(*state.selectedIndex, 2u);
}

TEST(DecorGestureTest, UnDeplacementSousLeSeuilNeProduitAucuneAction) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction preview = hmi::updateDecorGesture(
        state, Vector2{2.0f + hmi::DECOR_DRAG_THRESHOLD * 0.5f, 2.0f}, false);
    EXPECT_EQ(preview.kind, DecorGestureActionKind::None);

    const hmi::DecorGestureAction committed = hmi::endDecorGesture(
        state, Vector2{2.0f + hmi::DECOR_DRAG_THRESHOLD * 0.5f, 2.0f}, false);
    EXPECT_EQ(committed.kind, DecorGestureActionKind::None);
}

TEST(DecorGestureTest, UnDeplacementAuDelaDuSeuilProduitUneActionDeplacer) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction preview =
        hmi::updateDecorGesture(state, Vector2{2.0f + hmi::DECOR_DRAG_THRESHOLD * 5.0f, 2.0f}, false);

    EXPECT_EQ(preview.kind, DecorGestureActionKind::Move);
    EXPECT_FLOAT_EQ(preview.position.x, 1.0f + hmi::DECOR_DRAG_THRESHOLD * 5.0f);
    EXPECT_FLOAT_EQ(preview.position.y, 1.0f);
}

// ---------------------------------------------------------------------------------------------
// Deplacer (corps)
// ---------------------------------------------------------------------------------------------

TEST(DecorGestureTest, DeplacerProduitLaPositionFinaleAuRelachement) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);
    // Fait passer le geste en Dragging (au-dela du seuil) ; seule l'action finale nous interesse.
    static_cast<void>(hmi::updateDecorGesture(state, Vector2{5.0f, 2.0f}, false));

    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{5.0f, 3.5f}, false);

    EXPECT_EQ(action.kind, DecorGestureActionKind::Move);
    EXPECT_EQ(action.index, 0u);
    EXPECT_FLOAT_EQ(action.position.x, 4.0f);   // 1.0 + (5.0 - 2.0)
    EXPECT_FLOAT_EQ(action.position.y, 2.5f);   // 1.0 + (3.5 - 2.0)
}

TEST(DecorGestureTest, DeplacerAvecAimantationArrondiALaGrilleEntiere) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{5.4f, 2.4f}, true);

    EXPECT_EQ(action.kind, DecorGestureActionKind::Move);
    EXPECT_FLOAT_EQ(action.position.x, 4.0f);  // 1.0 + (5.4-2.0) = 4.4 -> arrondi a 4.0
    EXPECT_FLOAT_EQ(action.position.y, 1.0f);  // 1.0 + (2.4-2.0) = 1.4 -> arrondi a 1.0
}

TEST(DecorGestureTest, DeplacerSansAimantationConserveLaPositionExacte) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{5.4f, 2.4f}, false);

    EXPECT_FLOAT_EQ(action.position.x, 4.4f);
    EXPECT_FLOAT_EQ(action.position.y, 1.4f);
}

// ---------------------------------------------------------------------------------------------
// Redimensionner (coins)
// ---------------------------------------------------------------------------------------------

TEST(DecorGestureTest, RedimensionnerDepuisLeCoinBasDroitEtireDepuisLeCoinHautGaucheFixe) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{0.0f, 0.0f}};
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::BottomRight}, Vector2{4.0f, 2.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{8.0f, 6.0f}, false);

    EXPECT_EQ(action.kind, DecorGestureActionKind::Resize);
    // Ancre = coin haut-gauche (0,0), inchangee ; nouvelle taille (8,6) -> echelle x2, x3.
    EXPECT_FLOAT_EQ(action.position.x, 0.0f);
    EXPECT_FLOAT_EQ(action.position.y, 0.0f);
    EXPECT_FLOAT_EQ(action.scale.x, 2.0f);
    EXPECT_FLOAT_EQ(action.scale.y, 3.0f);
}

TEST(DecorGestureTest, RedimensionnerDepuisLeCoinHautGaucheDeplaceLAncreOpposee) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{0.0f, 0.0f}};
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::TopLeft}, Vector2{0.0f, 0.0f}, decor,
                           bounds);

    // Glisse le coin haut-gauche vers (-4, -2) : l'ancre (coin bas-droit, (4,2)) reste fixe.
    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{-4.0f, -2.0f}, false);

    EXPECT_EQ(action.kind, DecorGestureActionKind::Resize);
    EXPECT_FLOAT_EQ(action.position.x, -4.0f);
    EXPECT_FLOAT_EQ(action.position.y, -2.0f);
    EXPECT_FLOAT_EQ(action.scale.x, 2.0f);  // largeur 8 au lieu de 4
    EXPECT_FLOAT_EQ(action.scale.y, 2.0f);  // hauteur 4 au lieu de 2
}

TEST(DecorGestureTest, RedimensionnerNeDescendJamaisSousUneTailleMinimale) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{0.0f, 0.0f}};
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::BottomRight}, Vector2{4.0f, 2.0f}, decor,
                           bounds);

    // Glisse le coin bas-droit AU-DELA de l'ancre (0,0) : taille nulle/negative evitee.
    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{0.0f, 0.0f}, false);

    EXPECT_EQ(action.kind, DecorGestureActionKind::Resize);
    EXPECT_GT(action.scale.x, 0.0f);
    EXPECT_GT(action.scale.y, 0.0f);
}

// ---------------------------------------------------------------------------------------------
// Pivoter (poignee de rotation)
// ---------------------------------------------------------------------------------------------

TEST(DecorGestureTest, PivoterVersLaDroiteProduitUnQuartDeTour) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{0.0f, 0.0f}};
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{2.0f, 2.0f}};  // centre (1,1)
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Rotation}, Vector2{1.0f, -1.0f}, decor,
                           bounds);

    // Curseur directement a droite du centre (1,1) -> (3,1).
    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{3.0f, 1.0f}, false);

    EXPECT_EQ(action.kind, DecorGestureActionKind::Rotate);
    EXPECT_NEAR(action.rotation, TWO_PI * 0.25f, 1e-4f);
}

TEST(DecorGestureTest, PivoterNormaliseDansZeroDeuxPi) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{0.0f, 0.0f}};
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Rotation}, Vector2{1.0f, -1.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction action = hmi::endDecorGesture(state, Vector2{1.0f, 3.0f}, false);

    EXPECT_GE(action.rotation, 0.0f);
    EXPECT_LT(action.rotation, TWO_PI);
}

// ---------------------------------------------------------------------------------------------
// Abandon (Echap)
// ---------------------------------------------------------------------------------------------

TEST(DecorGestureTest, AbandonNeProduitAucuneAction) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);
    static_cast<void>(hmi::updateDecorGesture(state, Vector2{9.0f, 9.0f}, false));

    hmi::cancelDecorGesture(state);

    EXPECT_EQ(state.phase, hmi::DecorGesturePhase::Idle);
    ASSERT_TRUE(state.selectedIndex.has_value());
    EXPECT_EQ(*state.selectedIndex, 0u);  // la selection persiste, seul le glisser est abandonne
}
