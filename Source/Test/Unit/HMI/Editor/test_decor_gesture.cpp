/**
 * @file test_decor_gesture.cpp
 * @brief Tests unitaires de la machine à états du geste de manipulation de décors (LOT-50
 *        TACHE-02, `EX-DEC-010`).
 */

#include <cmath>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

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

/**
 * @brief En cas de superposition, le clic désigne le décor **le plus au-dessus** : c'est celui que
 * l'auteur voit, donc celui qu'il croit viser.
 * \castest{<b>Un clic sur des décors superposés désigne le plus au-dessus.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Un clic dans le vide ne désigne rien : l'appelant en déduit la désélection, sans index
 * sentinelle à interpréter.
 * \castest{<b>Un clic dans le vide ne désigne aucun décor.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorGestureTest, DesignateDecorAtDansLeVideRenvoieNullopt) {
    const std::vector<Rect> bounds{Rect{Vector2{0.0f, 0.0f}, Vector2{1.0f, 1.0f}}};

    const std::optional<DecorHit> hit =
        hmi::designateDecorAt(Vector2{99.0f, 99.0f}, bounds, std::nullopt, std::nullopt);

    EXPECT_FALSE(hit.has_value());
}

/**
 * @brief Les poignées du décor **sélectionné** l'emportent sur le corps d'un autre décor : sans
 * cette priorité, une poignée posée au-dessus d'un voisin deviendrait inutilisable et le clic
 * sélectionnerait le voisin au lieu de redimensionner.
 * \castest{<b>Les poignées du décor sélectionné priment sur le corps d'un autre décor.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorGestureTest, DesignateDecorAtPrivilegieLesPoigneesDuDecorSelectionne) {
    // Decor 0 selectionne, ses poignees couvrent (0,0). Decor 1 (non selectionne) a son corps la
    // aussi -- sans priorite aux poignees, le clic viserait le corps du decor 1.
    const std::vector<Rect> bounds{
        Rect{Vector2{0.0f, 0.0f}, Vector2{4.0f, 4.0f}},
        Rect{Vector2{-1.0f, -1.0f}, Vector2{4.0f, 4.0f}},
    };
    const hmi::DecorHandleLayout handles = hmi::decorHandleLayout(bounds[0], 0.1f, 0.0f);

    const std::optional<DecorHit> hit =
        hmi::designateDecorAt(Vector2{0.0f, 0.0f}, bounds, 0u, handles);

    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(hit->index, 0u);
    EXPECT_EQ(hit->handle, DecorHandle::TopLeft);
}

// ---------------------------------------------------------------------------------------------
// Clic contre glisser (seuil)
// ---------------------------------------------------------------------------------------------

/**
 * @brief L'appui sélectionne **immédiatement**, avant tout glisser : un simple clic doit suffire à
 * sélectionner, sans obliger à bouger la souris.
 * \castest{<b>L'appui sélectionne immédiatement, même sans glisser.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorGestureTest, BeginSelectionneImmediatementMemeSansGlisser) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};

    hmi::beginDecorGesture(state, DecorHit{2, DecorHandle::Body}, Vector2{1.5f, 1.5f}, decor,
                           bounds);

    ASSERT_TRUE(state.selectedIndex.has_value());
    EXPECT_EQ(*state.selectedIndex, 2u);
}

/**
 * @brief Sous le seuil de glisser, ni l'aperçu ni le relâchement ne produisent d'action : c'est ce
 * qui distingue un clic de sélection d'un déplacement, et qui évite d'empiler dans l'historique un
 * déplacement d'un demi-pixel à chaque clic.
 * \castest{<b>Un déplacement sous le seuil ne produit aucune action.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorGestureTest, UnDeplacementSousLeSeuilNeProduitAucuneAction) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction preview = hmi::updateDecorGesture(
        state, Vector2{2.0f + hmi::DECOR_DRAG_THRESHOLD * 0.5f, 2.0f}, false);
    EXPECT_EQ(preview.kind, DecorGestureActionKind::None);

    const hmi::DecorGestureAction committed =
        hmi::endDecorGesture(state, Vector2{2.0f + hmi::DECOR_DRAG_THRESHOLD * 0.5f, 2.0f}, false);
    EXPECT_EQ(committed.kind, DecorGestureActionKind::None);
}

/**
 * @brief Au-delà du seuil, l'aperçu produit un déplacement dont la position suit exactement le
 * curseur : l'aperçu est calculé par la même fonction que l'action finale, il ne peut pas en
 * diverger.
 * \castest{<b>Un déplacement au-delà du seuil produit un aperçu de déplacement suivant le
 * curseur.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorGestureTest, UnDeplacementAuDelaDuSeuilProduitUneActionDeplacer) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{1.0f, 1.0f}};
    const Rect bounds{Vector2{1.0f, 1.0f}, Vector2{2.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::Body}, Vector2{2.0f, 2.0f}, decor,
                           bounds);

    const hmi::DecorGestureAction preview = hmi::updateDecorGesture(
        state, Vector2{2.0f + hmi::DECOR_DRAG_THRESHOLD * 5.0f, 2.0f}, false);

    EXPECT_EQ(preview.kind, DecorGestureActionKind::Move);
    EXPECT_FLOAT_EQ(preview.position.x, 1.0f + hmi::DECOR_DRAG_THRESHOLD * 5.0f);
    EXPECT_FLOAT_EQ(preview.position.y, 1.0f);
}

// ---------------------------------------------------------------------------------------------
// Deplacer (corps)
// ---------------------------------------------------------------------------------------------

/**
 * @brief Le relâchement produit la position finale, calculée comme le décalage du curseur appliqué
 * à la position d'origine — jamais la position du curseur elle-même, qui ferait sauter le décor
 * sous le pointeur au premier pixel de glisser.
 * \castest{<b>Le relâchement produit la position finale, décalée depuis la position
 * d'origine.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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
    EXPECT_FLOAT_EQ(action.position.x, 4.0f);  // 1.0 + (5.0 - 2.0)
    EXPECT_FLOAT_EQ(action.position.y, 2.5f);  // 1.0 + (3.5 - 2.0)
}

/**
 * @brief Avec aimantation, la position finale est arrondie à la case entière : c'est ce qui permet
 * d'aligner des décors sur la grille sans viser au pixel.
 * \castest{<b>Avec aimantation, la position finale est arrondie à la case entière.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Sans aimantation, la position exacte est conservée : les décors sont libres, hors grille
 * (`EX-DEC-001`), et l'aimantation reste un choix ponctuel.
 * \castest{<b>Sans aimantation, la position exacte est conservée.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Tirer le coin bas-droit étire le décor depuis le coin haut-gauche, qui reste **fixe** :
 * l'ancre est toujours le coin opposé à la poignée, comme dans tout éditeur graphique.
 * \castest{<b>Tirer le coin bas-droit étire depuis le coin haut-gauche, resté fixe.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Tirer le coin haut-gauche déplace la position **et** l'échelle, l'ancre étant cette fois
 * le coin bas-droit : les quatre poignées suivent la même règle, sans cas particulier.
 * \castest{<b>Tirer le coin haut-gauche déplace la position et l'échelle, ancre au coin
 * bas-droit.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(DecorGestureTest, RedimensionnerDepuisLeCoinHautGaucheDeplaceLAncreOpposee) {
    DecorGestureState state;
    Decor decor{"tree.png", Vector2{0.0f, 0.0f}};
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 2.0f}};
    hmi::beginDecorGesture(state, DecorHit{0, DecorHandle::TopLeft}, Vector2{0.0f, 0.0f}, decor,
                           bounds);

    // Glisse le coin haut-gauche vers (-4, -2) : l'ancre (coin bas-droit, (4,2)) reste fixe.
    const hmi::DecorGestureAction action =
        hmi::endDecorGesture(state, Vector2{-4.0f, -2.0f}, false);

    EXPECT_EQ(action.kind, DecorGestureActionKind::Resize);
    EXPECT_FLOAT_EQ(action.position.x, -4.0f);
    EXPECT_FLOAT_EQ(action.position.y, -2.0f);
    EXPECT_FLOAT_EQ(action.scale.x, 2.0f);  // largeur 8 au lieu de 4
    EXPECT_FLOAT_EQ(action.scale.y, 2.0f);  // hauteur 4 au lieu de 2
}

/**
 * @brief Tirer une poignée au-delà de son ancre ne produit jamais une échelle nulle ou négative :
 * un décor d'échelle nulle serait invisible, donc impossible à rattraper à la souris.
 * \castest{<b>Le redimensionnement ne descend jamais sous une taille minimale strictement
 * positive.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief L'angle suit la direction du curseur **depuis le centre** du décor : curseur à droite du
 * centre, quart de tour. C'est ce qui rend la rotation prévisible quelle que soit la distance au
 * centre.
 * \castest{<b>Amener le curseur à droite du centre produit un quart de tour.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief L'angle produit reste dans [0, 2π) quel que soit le quadrant visé, cohérent avec la
 * normalisation faite côté modèle.
 * \castest{<b>L'angle produit par la rotation reste dans [0, 2π).</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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

/**
 * @brief Abandonner (Échap) ramène le geste au repos sans produire d'action, mais **conserve la
 * sélection** : on annule le glisser en cours, pas le fait d'avoir désigné le décor.
 * \castest{<b>Abandonner un geste ne produit aucune action et conserve la sélection.</b><br/>
 * \tcat Unitaire · Geste sur les décors<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
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
