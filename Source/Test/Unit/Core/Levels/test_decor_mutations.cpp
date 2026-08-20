// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_decor_mutations.cpp
 * @brief Tests unitaires des mutateurs de décors sur `LevelDraft` (LOT-50 TACHE-01,
 *        `EX-DEC-010`).
 */

#include <cmath>
#include <optional>

#include <gtest/gtest.h>

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

/**
 * @brief Déplacer un décor change sa position et signale la réussite : le mutateur le plus simple,
 * socle du glisser-déposer dans l'éditeur.
 * \castest{<b>Déplacer un décor change sa position.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, MoveDecorDeplaceLaPosition) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    EXPECT_TRUE(draft.moveDecor(0, Vector2{4.5f, 2.25f}));

    EXPECT_EQ(draft.decors()[0].position, (Vector2{4.5f, 2.25f}));
}

/**
 * @brief Un index hors bornes rend faux plutôt que d'accéder au vecteur : l'index vient de la
 * sélection de l'éditeur, qui peut avoir été invalidée par une suppression concurrente.
 * \castest{<b>Déplacer un décor hors bornes rend faux, sans effet.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, MoveDecorHorsBornesRenvoieFauxSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.moveDecor(0, Vector2{4.5f, 2.25f}));
}

/**
 * @brief Le déplacement s'annule et se refait comme toute autre modification : les décors partagent
 * l'historique du brouillon, ils n'ont pas leur pile à part.
 * \castest{<b>Le déplacement d'un décor s'annule et se refait.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Le redimensionnement écrit la position **et** l'échelle en une seule opération : tirer une
 * poignée d'angle bouge les deux à la fois, et les séparer produirait deux entrées d'historique
 * pour un seul geste.
 * \castest{<b>Redimensionner un décor change l'échelle et la position atomiquement.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, ResizeDecorChangeLEchelleEtLaPositionAtomiquement) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    EXPECT_TRUE(draft.resizeDecor(0, Vector2{0.5f, 0.5f}, Vector2{2.0f, 0.5f}));

    EXPECT_EQ(draft.decors()[0].position, (Vector2{0.5f, 0.5f}));
    EXPECT_EQ(draft.decors()[0].scale, (Vector2{2.0f, 0.5f}));
}

/**
 * @brief Un index hors bornes rend faux sans rien écrire, comme pour le déplacement.
 * \castest{<b>Redimensionner un décor hors bornes rend faux, sans effet.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, ResizeDecorHorsBornesRenvoieFauxSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.resizeDecor(0, Vector2{0.0f, 0.0f}, Vector2{2.0f, 2.0f}));
}

/**
 * @brief Une échelle nulle ou négative est rejetée **sans** appliquer non plus la position : un
 * décor d'échelle nulle serait invisible et irrécupérable à la souris, une échelle négative le
 * retournerait à l'insu de l'auteur. Le rejet est total, jamais partiel.
 * \castest{<b>Une échelle nulle ou négative est rejetée sans toucher à la position.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief L'annulation d'un redimensionnement restaure les deux grandeurs ensemble — position et
 * échelle — cohérente avec l'atomicité de l'opération.
 * \castest{<b>Le redimensionnement d'un décor s'annule et se refait, position et échelle
 * ensemble.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Faire pivoter un décor écrit son angle et signale la réussite.
 * \castest{<b>Faire pivoter un décor écrit son angle.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, RotateDecorNominal) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"bush.png", Vector2{1.0f, 1.0f}});

    EXPECT_TRUE(draft.rotateDecor(0, 1.0f));

    EXPECT_FLOAT_EQ(draft.decors()[0].rotation, 1.0f);
}

/**
 * @brief Un index hors bornes rend faux sans rien écrire, comme les autres mutateurs.
 * \castest{<b>Faire pivoter un décor hors bornes rend faux, sans effet.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, RotateDecorHorsBornesRenvoieFauxSansEffet) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.rotateDecor(0, 1.0f));
}

/**
 * @brief L'angle est ramené dans l'intervalle [0, 2π), y compris depuis une valeur négative : sans
 * normalisation, une rotation répétée à la molette ferait croître l'angle indéfiniment et finirait
 * par perdre en précision flottante.
 * \castest{<b>L'angle d'un décor est normalisé dans [0, 2π), même depuis une valeur
 * négative.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief La rotation s'annule et se refait, en revenant à l'angle nul d'origine.
 * \castest{<b>La rotation d'un décor s'annule et se refait.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Changer un décor de couche le renvoie en **fin** du vecteur : il devient le plus en avant
 * de sa nouvelle couche, et le nouvel index est rendu à l'appelant pour que la sélection le suive
 * au lieu de désigner un voisin.
 * \castest{<b>Changer un décor de couche l'envoie en fin de vecteur et rend son nouvel
 * index.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Renvoyer un décor vers sa couche actuelle réussit sans rien déplacer : l'opération est
 * idempotente, un clic redondant ne doit pas réordonner la scène.
 * \castest{<b>Changer un décor vers sa couche actuelle réussit sans rien déplacer.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, SetDecorLayerVersLaMemeCoucheEstUnNoOpReussi) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);
    draft.addDecor(Decor{"a.png", Vector2{0.0f, 0.0f}});  // couche par defaut : Decor

    const std::optional<std::size_t> newIndex = draft.setDecorLayer(0, DecorLayer::Decor);

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 0u);
    EXPECT_EQ(draft.decors()[0].assetName, "a.png");
}

/**
 * @brief Un index hors bornes ne rend aucun index : l'appelant distingue ainsi l'échec du succès
 * sans code d'erreur séparé.
 * \castest{<b>Changer de couche hors bornes ne rend aucun index.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, SetDecorLayerHorsBornesRenvoieNullopt) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.setDecorLayer(0, DecorLayer::Foreground).has_value());
}

/**
 * @brief Le changement de couche s'annule et se refait comme les autres mutations.
 * \castest{<b>Le changement de couche d'un décor s'annule et se refait.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Avancer un décor l'échange avec le suivant **de sa couche**, en sautant par-dessus les
 * décors d'une autre couche sans les toucher : l'ordre de dessin est déjà tranché par la couche,
 * réordonner à l'intérieur ne doit pas la traverser.
 * \castest{<b>Avancer un décor l'échange avec le suivant de sa couche, sautant les
 * autres.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, BringDecorForwardEchangeAvecLeProchainDeMemeCouche) {
    LevelDraft draft =
        fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    const std::optional<std::size_t> newIndex = draft.bringDecorForward(0);  // avance A

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 2u);  // A saute par-dessus B (autre couche) pour s'echanger avec C
    ASSERT_EQ(draft.decors().size(), 4u);
    EXPECT_EQ(draft.decors()[0].assetName, "C.png");
    EXPECT_EQ(draft.decors()[1].assetName, "B.png");  // intacte, jamais deplacee
    EXPECT_EQ(draft.decors()[2].assetName, "A.png");
    EXPECT_EQ(draft.decors()[3].assetName, "D.png");
}

/**
 * @brief Avancer un décor déjà le plus en avant de sa couche réussit sans rien changer — pas un
 * échec, qui afficherait une erreur pour un geste anodin.
 * \castest{<b>Avancer un décor déjà en tête de sa couche réussit sans rien changer.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, BringDecorForwardDejaEnTeteEstUnNoOpReussi) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // ... D est deja le plus en avant

    const std::optional<std::size_t> newIndex = draft.bringDecorForward(3);  // D, dernier decor

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 3u);
    EXPECT_EQ(draft.decors()[3].assetName, "D.png");
}

/**
 * @brief Reculer un décor l'échange avec le précédent de sa couche, en sautant lui aussi par-dessus
 * les décors des autres couches.
 * \castest{<b>Reculer un décor l'échange avec le précédent de sa couche.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, SendDecorBackwardEchangeAvecLePrecedentDeMemeCouche) {
    LevelDraft draft =
        fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    const std::optional<std::size_t> newIndex = draft.sendDecorBackward(2);  // recule C

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 0u);  // C saute par-dessus B pour s'echanger avec A
    EXPECT_EQ(draft.decors()[0].assetName, "C.png");
    EXPECT_EQ(draft.decors()[1].assetName, "B.png");
    EXPECT_EQ(draft.decors()[2].assetName, "A.png");
    EXPECT_EQ(draft.decors()[3].assetName, "D.png");
}

/**
 * @brief Reculer un décor déjà le plus en arrière de sa couche réussit sans rien changer.
 * \castest{<b>Reculer un décor déjà en queue de sa couche réussit sans rien changer.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, SendDecorBackwardDejaEnQueueEstUnNoOpReussi) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();  // A est deja le plus en arriere

    const std::optional<std::size_t> newIndex = draft.sendDecorBackward(0);

    ASSERT_TRUE(newIndex.has_value());
    EXPECT_EQ(*newIndex, 0u);
    EXPECT_EQ(draft.decors()[0].assetName, "A.png");
}

/**
 * @brief Mettre au premier plan place le décor au dernier rang **de sa couche** et laisse sa couche
 * inchangée : le geste réordonne, il ne promeut jamais un décor d'un plan à l'autre.
 * \castest{<b>Mettre au premier plan place le décor au dernier rang de sa couche, couche
 * inchangée.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, BringDecorToFrontAmeneAuDernierRangDeSaCouche) {
    LevelDraft draft =
        fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

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

/**
 * @brief Mettre à l'arrière-plan place le décor au premier rang de sa couche, couche inchangée elle
 * aussi.
 * \castest{<b>Mettre à l'arrière-plan place le décor au premier rang de sa couche, couche
 * inchangée.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, SendDecorToBackAmeneAuPremierRangDeSaCouche) {
    LevelDraft draft =
        fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

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

/**
 * @brief Les deux extrêmes appliqués à un décor déjà en place réussissent sans rien déplacer : même
 * idempotence que les échanges d'un rang.
 * \castest{<b>Premier plan et arrière-plan sur un décor déjà en place ne déplacent rien.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, BringDecorToFrontEtSendDecorToBackDejaEnPlaceSontDesNoOp) {
    LevelDraft draft =
        fourDecorsWithInterleavedLayer();  // A(Decor) B(Foreground) C(Decor) D(Decor)

    EXPECT_EQ(draft.bringDecorToFront(3), 3u);  // D deja au premier plan de sa couche
    EXPECT_EQ(draft.sendDecorToBack(0), 0u);    // A deja a l'arriere-plan de sa couche
}

/**
 * @brief Les quatre opérations de réordonnancement rendent toutes l'absence d'index sur un index
 * hors bornes : un contrat uniforme, qu'aucune des quatre ne doit trahir isolément.
 * \castest{<b>Les quatre opérations de réordonnancement hors bornes ne rendent aucun
 * index.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, ReordonnancementHorsBornesRenvoieNullopt) {
    LevelDraft draft = LevelDraft::empty("N", 10, 10);

    EXPECT_FALSE(draft.bringDecorForward(0).has_value());
    EXPECT_FALSE(draft.sendDecorBackward(0).has_value());
    EXPECT_FALSE(draft.bringDecorToFront(0).has_value());
    EXPECT_FALSE(draft.sendDecorToBack(0).has_value());
}

/**
 * @brief Le réordonnancement s'annule et se refait : l'ordre du vecteur fait partie de l'état
 * instantané du brouillon, au même titre que les tuiles.
 * \castest{<b>Le réordonnancement d'un décor s'annule et se refait.</b><br/>
 * \tcat Unitaire · Mutations de décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorMutationsTest, BringDecorToFrontUndoRedo) {
    LevelDraft draft = fourDecorsWithInterleavedLayer();

    draft.bringDecorToFront(0);
    ASSERT_TRUE(draft.undo());
    EXPECT_EQ(draft.decors()[0].assetName, "A.png");
    ASSERT_TRUE(draft.redo());
    EXPECT_EQ(draft.decors()[3].assetName, "A.png");
}
