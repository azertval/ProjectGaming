/**
 * @file test_decor_mutations.cpp
 * @brief Tests unitaires des mutateurs de décors sur `LevelDraft` (LOT-50 TACHE-01,
 *        `EX-DEC-010`).
 */

#include <gtest/gtest.h>

#include <cmath>
#include <optional>

#include "Core/Levels/Decor.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Math/Vector2.h"

namespace {

using core::Decor;
using core::DecorLayer;
using core::LevelDraft;
using core::Vector2;

constexpr float TWO_PI = 6.28318530717958647692f;

}  // namespace

// ---------------------------------------------------------------------------------------------
// moveDecor
// ---------------------------------------------------------------------------------------------

TEST(DecorMutationsTest, MoveDecorDeplaceLaPosition) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    EXPECT_TRUE(draft.moveDecor(0, Vector2{4.5f, 2.25f}));

    EXPECT_EQ(draft.decors()[0].position, (Vector2{4.5f, 2.25f}));
}

TEST(DecorMutationsTest, MoveDecorHorsBornesRenvoieFauxSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.moveDecor(0, Vector2{4.5f, 2.25f}));
}

TEST(DecorMutationsTest, MoveDecorUndoRedo) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    draft.moveDecor(0, Vector2{4.5f, 2.25f});
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.decors()[0].position, (Vector2{1.0f, 1.0f}));
    ASSERT_TRUE(draft.redo());
    EXPECT_EQ(draft.decors()[0].position, (Vector2{4.5f, 2.25f}));
}

// ---------------------------------------------------------------------------------------------
// resizeDecor
// ---------------------------------------------------------------------------------------------

TEST(DecorMutationsTest, ResizeDecorChangeLEchelleEtLaPositionAtomiquement) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    EXPECT_TRUE(draft.resizeDecor(0, Vector2{0.5f, 0.5f}, Vector2{2.0f, 0.5f}));

    EXPECT_EQ(draft.decors()[0].position, (Vector2{0.5f, 0.5f}));
    EXPECT_EQ(draft.decors()[0].scale, (Vector2{2.0f, 0.5f}));
}

TEST(DecorMutationsTest, ResizeDecorHorsBornesRenvoieFauxSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.resizeDecor(0, Vector2{0.0f, 0.0f}, Vector2{2.0f, 2.0f}));
}

TEST(DecorMutationsTest, ResizeDecorEchelleNulleOuNegativeRejeteeSansToucherLaPosition) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    EXPECT_FALSE(draft.resizeDecor(0, Vector2{9.0f, 9.0f}, Vector2{0.0f, 1.0f}));
    EXPECT_FALSE(draft.resizeDecor(0, Vector2{9.0f, 9.0f}, Vector2{1.0f, 0.0f}));
    EXPECT_FALSE(draft.resizeDecor(0, Vector2{9.0f, 9.0f}, Vector2{-1.0f, 1.0f}));
    EXPECT_FALSE(draft.resizeDecor(0, Vector2{9.0f, 9.0f}, Vector2{1.0f, -1.0f}));
    // Aucune des tentatives rejetees n'a modifie l'echelle (1,1) ni la position (1,1) par defaut.
    EXPECT_EQ(draft.decors()[0].scale, (Vector2{1.0f, 1.0f}));
    EXPECT_EQ(draft.decors()[0].position, (Vector2{1.0f, 1.0f}));
}

TEST(DecorMutationsTest, ResizeDecorUndoRedo) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    draft.resizeDecor(0, Vector2{0.5f, 0.5f}, Vector2{3.0f, 3.0f});
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.decors()[0].scale, (Vector2{1.0f, 1.0f}));
    EXPECT_EQ(draft.decors()[0].position, (Vector2{1.0f, 1.0f}));
    ASSERT_TRUE(draft.redo());
    EXPECT_EQ(draft.decors()[0].scale, (Vector2{3.0f, 3.0f}));
    EXPECT_EQ(draft.decors()[0].position, (Vector2{0.5f, 0.5f}));
}

// ---------------------------------------------------------------------------------------------
// rotateDecor
// ---------------------------------------------------------------------------------------------

TEST(DecorMutationsTest, RotateDecorNominal) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    EXPECT_TRUE(draft.rotateDecor(0, 1.0f));

    EXPECT_FLOAT_EQ(draft.decors()[0].rotation, 1.0f);
}

TEST(DecorMutationsTest, RotateDecorHorsBornesRenvoieFauxSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.rotateDecor(0, 1.0f));
}

TEST(DecorMutationsTest, RotateDecorNormaliseHorsDeZeroDeuxPi) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    ASSERT_TRUE(draft.rotateDecor(0, TWO_PI + 0.5f));
    EXPECT_NEAR(draft.decors()[0].rotation, 0.5f, 1e-4f);
    EXPECT_GE(draft.decors()[0].rotation, 0.0f);
    EXPECT_LT(draft.decors()[0].rotation, TWO_PI);

    ASSERT_TRUE(draft.rotateDecor(0, -0.5f));
    EXPECT_NEAR(draft.decors()[0].rotation, TWO_PI - 0.5f, 1e-4f);
    EXPECT_GE(draft.decors()[0].rotation, 0.0f);
    EXPECT_LT(draft.decors()[0].rotation, TWO_PI);
}

TEST(DecorMutationsTest, RotateDecorUndoRedo) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    draft.rotateDecor(0, 1.5f);
    ASSERT_TRUE(draft.undo());
    EXPECT_FLOAT_EQ(draft.decors()[0].rotation, 0.0f);
    ASSERT_TRUE(draft.redo());
    EXPECT_FLOAT_EQ(draft.decors()[0].rotation, 1.5f);
}

// ---------------------------------------------------------------------------------------------
// setDecorLayer
// ---------------------------------------------------------------------------------------------

TEST(DecorMutationsTest, SetDecorLayerEnvoieEnFinDeVecteurDeSaNouvelleCouche) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    Decor first{"a.png", Vector2{0.0f, 0.0f}};
    first.layer = DecorLayer::Background;
    Decor second{"b.png", Vector2{1.0f, 1.0f}};
    second.layer = DecorLayer::Foreground;
    draft.addDecor(first);
    draft.addDecor(second);

    const std::optional<std::size_t> newIndex = draft.setDecorLayer(0, DecorLayer::Foreground);

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 1u);
    ASSERT_EQ(draft.decors().size(), 2u);
    EXPECT_EQ(draft.decors()[0].assetName, "b.png");
    EXPECT_EQ(draft.decors()[1].assetName, "a.png");
    EXPECT_EQ(draft.decors()[1].layer, DecorLayer::Foreground);
}

TEST(DecorMutationsTest, SetDecorLayerVersLaMemeCoucheEstUnNoOpReussi) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"a.png", Vector2{0.0f, 0.0f}});  // couche par defaut : Decor

    const std::optional<std::size_t> newIndex = draft.setDecorLayer(0, DecorLayer::Decor);

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 0u);
    EXPECT_EQ(draft.decors()[0].assetName, "a.png");
}

TEST(DecorMutationsTest, SetDecorLayerHorsBornesRenvoieNullopt) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.setDecorLayer(0, DecorLayer::Foreground).has_value());
}

TEST(DecorMutationsTest, SetDecorLayerUndoRedo) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"a.png", Vector2{0.0f, 0.0f}});

    draft.setDecorLayer(0, DecorLayer::Foreground);
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.decors()[0].layer, DecorLayer::Decor);
    ASSERT_TRUE(draft.redo());
    EXPECT_EQ(draft.decors()[0].layer, DecorLayer::Foreground);
}

// ---------------------------------------------------------------------------------------------
// Reordonnancement intra-couche : bringDecorForward / sendDecorBackward / bringDecorToFront /
// sendDecorToBack. Decors intercales de couches differentes pour verifier que le
// reordonnancement saute par-dessus les decors d'une AUTRE couche sans les toucher.
// ---------------------------------------------------------------------------------------------

namespace {

// Construit A(Decor), B(Foreground), C(Decor), D(Decor) : B est une couche differente,
// intercalee entre les decors de couche Decor pour verifier que le reordonnancement de ceux-ci
// la traverse sans la perturber.
LevelDraft fourDecorsWithInterleavedLayer() {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    Decor a{"A.png", Vector2{0.0f, 0.0f}};
    a.layer = DecorLayer::Decor;
    Decor b{"B.png", Vector2{1.0f, 1.0f}};
    b.layer = DecorLayer::Foreground;
    Decor c{"C.png", Vector2{2.0f, 2.0f}};
    c.layer = DecorLayer::Decor;
    Decor d{"D.png", Vector2{3.0f, 3.0f}};
    d.layer = DecorLayer::Decor;
    draft.addDecor(a);
    draft.addDecor(b);
    draft.addDecor(c);
    draft.addDecor(d);
    return draft;
}

}  // namespace

TEST(DecorMutationsTest, BringDecorForwardEchangeAvecLeProchainDeMemeCouche) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    const std::optional<std::size_t> newIndex = draft.bringDecorForward(0);  // avance A

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 2u);  // A saute par-dessus B (autre couche) pour s'echanger avec C
    ASSERT_EQ(draft.decors().size(), 4u);
    EXPECT_EQ(draft.decors()[0].assetName, "C.png");
    EXPECT_EQ(draft.decors()[1].assetName, "B.png");  // intacte, jamais deplacee
    EXPECT_EQ(draft.decors()[2].assetName, "A.png");
    EXPECT_EQ(draft.decors()[3].assetName, "D.png");
}

TEST(DecorMutationsTest, BringDecorForwardDejaEnTeteEstUnNoOpReussi) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // ... D est deja le plus en avant

    const std::optional<std::size_t> newIndex = draft.bringDecorForward(3);  // D, dernier decor

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 3u);
    EXPECT_EQ(draft.decors()[3].assetName, "D.png");
}

TEST(DecorMutationsTest, SendDecorBackwardEchangeAvecLePrecedentDeMemeCouche) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    const std::optional<std::size_t> newIndex = draft.sendDecorBackward(2);  // recule C

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 0u);  // C saute par-dessus B pour s'echanger avec A
    EXPECT_EQ(draft.decors()[0].assetName, "C.png");
    EXPECT_EQ(draft.decors()[1].assetName, "B.png");
    EXPECT_EQ(draft.decors()[2].assetName, "A.png");
    EXPECT_EQ(draft.decors()[3].assetName, "D.png");
}

TEST(DecorMutationsTest, SendDecorBackwardDejaEnQueueEstUnNoOpReussi) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // A est deja le plus en arriere

    const std::optional<std::size_t> newIndex = draft.sendDecorBackward(0);

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 0u);
    EXPECT_EQ(draft.decors()[0].assetName, "A.png");
}

TEST(DecorMutationsTest, BringDecorToFrontAmeneAuDernierRangDeSaCouche) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    const std::optional<std::size_t> newIndex = draft.bringDecorToFront(0);  // A au premier plan

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 3u);
    ASSERT_EQ(draft.decors().size(), 4u);
    EXPECT_EQ(draft.decors()[0].assetName, "B.png");
    EXPECT_EQ(draft.decors()[1].assetName, "C.png");
    EXPECT_EQ(draft.decors()[2].assetName, "D.png");
    EXPECT_EQ(draft.decors()[3].assetName, "A.png");
    EXPECT_EQ(draft.decors()[3].layer, DecorLayer::Decor);  // couche inchangee
}

TEST(DecorMutationsTest, SendDecorToBackAmeneAuPremierRangDeSaCouche) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    const std::optional<std::size_t> newIndex = draft.sendDecorToBack(3);  // D a l'arriere-plan

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 0u);
    ASSERT_EQ(draft.decors().size(), 4u);
    EXPECT_EQ(draft.decors()[0].assetName, "D.png");
    EXPECT_EQ(draft.decors()[1].assetName, "A.png");
    EXPECT_EQ(draft.decors()[2].assetName, "B.png");
    EXPECT_EQ(draft.decors()[3].assetName, "C.png");
    EXPECT_EQ(draft.decors()[0].layer, DecorLayer::Decor);  // couche inchangee
}

TEST(DecorMutationsTest, BringDecorToFrontEtSendDecorToBackDejaEnPlaceSontDesNoOp) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    EXPECT_EQ(draft.bringDecorToFront(3), 3u);  // D deja au premier plan de sa couche
    EXPECT_EQ(draft.sendDecorToBack(0), 0u);    // A deja a l'arriere-plan de sa couche
}

TEST(DecorMutationsTest, ReordonnancementHorsBornesRenvoieNullopt) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.bringDecorForward(0).has_value());
    EXPECT_FALSE(draft.sendDecorBackward(0).has_value());
    EXPECT_FALSE(draft.bringDecorToFront(0).has_value());
    EXPECT_FALSE(draft.sendDecorToBack(0).has_value());
}

TEST(DecorMutationsTest, BringDecorToFrontUndoRedo) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();

    draft.bringDecorToFront(0);
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.decors()[0].assetName, "A.png");
    ASSERT_TRUE(draft.redo());
    EXPECT_EQ(draft.decors()[3].assetName, "A.png");
}
