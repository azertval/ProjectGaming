/**
 * @file test_decor_geometry.cpp
 * @brief Tests unitaires de la géométrie partagée corps/poignées d'un décor (LOT-50 TACHE-02).
 */

#include <gtest/gtest.h>

#include "Core/Levels/Decor.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/DecorGeometry.h"

namespace {

using core::Decor;
using core::Rect;
using core::Vector2;

}  // namespace

/**
 * @brief decorWorldBounds place le rectangle au coin haut-gauche du décor, à sa taille réelle en
 * unités monde (pixels de l'asset convertis à 16 px/unité, `EX-ARCH-021`, puis multipliés par
 * l'échelle) — jamais la taille en pixels telle quelle.
 * \castest{<b>decorWorldBounds calcule le rectangle englobant en unites monde.</b><br/>
 * \tcat Unitaire · Decor Geometry<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le rectangle d'un decor a echelle non uniforme.<br/>
 * \tattendu Position = position du decor ; taille = (taille pixel / 16) * echelle sur chaque axe.
 * }
 */
TEST(DecorGeometryTest, DecorWorldBoundsCalculeLeRectangleEnglobantEnUnitesMonde) {
    Decor decor{"tree.png", Vector2{3.0f, 4.0f}};
    decor.scale = Vector2{2.0f, 0.5f};

    // 32x16 px, a 16 px/unite (Camera2D::PIXELS_PER_UNIT) : 2x1 unite avant echelle.
    const Rect bounds = hmi::decorWorldBounds(decor, Vector2{32.0f, 16.0f});

    EXPECT_EQ(bounds.position, (Vector2{3.0f, 4.0f}));
    EXPECT_EQ(bounds.size, (Vector2{4.0f, 0.5f}));  // (2*2.0, 1*0.5)
}

/**
 * @brief Les poignées gardent une taille écran constante : à zoom double (moitié moins d'unités
 * monde par pixel écran), leur taille en unités monde est divisée par deux.
 * \castest{<b>Les poignees gardent une taille ecran constante quel que soit le zoom.</b><br/>
 * \tcat Unitaire · Decor Geometry<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer les poignees d'un decor a deux echelles ecran differentes.<br/>
 * \tattendu La taille des poignees en unites monde est proportionnelle a
 * worldUnitsPerScreenPixel.
 * }
 */
TEST(DecorGeometryTest, PoigneesTailleEcranConstante) {
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 2.0f}};

    const hmi::DecorHandleLayout wide = hmi::decorHandleLayout(bounds, 0.1f, 0.0f);
    const hmi::DecorHandleLayout narrow = hmi::decorHandleLayout(bounds, 0.05f, 0.0f);

    EXPECT_FLOAT_EQ(wide.topLeft.size.x, narrow.topLeft.size.x * 2.0f);
    EXPECT_FLOAT_EQ(wide.topLeft.size.y, narrow.topLeft.size.y * 2.0f);
}

/**
 * @brief Les quatre poignées de coin sont centrées sur les quatre coins du rectangle englobant, et
 * la poignée de rotation au-dessus du centre du bord supérieur.
 * \castest{<b>Les poignees sont positionnees sur les coins et au-dessus du bord superieur.</b><br/>
 * \tcat Unitaire · Decor Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer les poignees d'un decor connu.<br/>
 * \tattendu Chaque poignee de coin contient le coin correspondant ; la poignee de rotation est
 * au-dessus du centre du bord superieur.
 * }
 */
TEST(DecorGeometryTest, PoigneesPositionneesAuxCoinsEtAuDessusDuBordSuperieur) {
    const Rect bounds{Vector2{10.0f, 10.0f}, Vector2{4.0f, 2.0f}};
    const hmi::DecorHandleLayout layout = hmi::decorHandleLayout(bounds, 0.1f, 0.0f);

    EXPECT_TRUE(layout.topLeft.contains(Vector2{10.0f, 10.0f}));
    EXPECT_TRUE(layout.topRight.contains(Vector2{14.0f, 10.0f}));
    EXPECT_TRUE(layout.bottomLeft.contains(Vector2{10.0f, 12.0f}));
    EXPECT_TRUE(layout.bottomRight.contains(Vector2{14.0f, 12.0f}));
    // Poignee de rotation : au-dessus du centre du bord superieur (x=12), y < 10.
    EXPECT_NEAR(layout.rotation.position.x + layout.rotation.size.x * 0.5f, 12.0f, 1e-3f);
    EXPECT_LT(layout.rotation.position.y + layout.rotation.size.y, 10.0f);
}

/**
 * @brief hitTestDecorHandles renvoie la poignée touchée, ou `None` en dehors de toutes.
 * \castest{<b>hitTestDecorHandles identifie la poignee touchee.</b><br/>
 * \tcat Unitaire · Decor Geometry<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tester un point sur chaque poignee, puis un point hors de toutes.<br/>
 * \tattendu Chaque poignee est identifiee correctement ; le point exterieur renvoie None.
 * }
 */
TEST(DecorGeometryTest, HitTestDecorHandlesIdentifieLaPoigneeTouchee) {
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 2.0f}};
    const hmi::DecorHandleLayout layout = hmi::decorHandleLayout(bounds, 0.1f, 0.0f);

    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{0.0f, 0.0f}, layout), hmi::DecorHandle::TopLeft);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{4.0f, 0.0f}, layout), hmi::DecorHandle::TopRight);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{0.0f, 2.0f}, layout), hmi::DecorHandle::BottomLeft);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{4.0f, 2.0f}, layout), hmi::DecorHandle::BottomRight);
    // Centre de la poignee de rotation : x = centre du bord superieur (2.0), y = -rotationOffset
    // (24 * 0.1 = 2.4 unites monde au-dessus du bord superieur, en y=0).
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{2.0f, -2.4f}, layout), hmi::DecorHandle::Rotation);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{2.0f, 1.0f}, layout), hmi::DecorHandle::None);
}

/**
 * @brief `decorRotatedPoint` à rotation nulle renvoie exactement `centre + localOffset` (identité)
 * : base de comparaison avant de vérifier une rotation non nulle. \castest{<b>decorRotatedPoint a
 * rotation nulle est l'identite.</b><br/> \tcat Unitaire · Decor Geometry<br/> \tcrit Majeur<br/>
 * \tetapes 1. Calculer decorRotatedPoint a rotation 0.<br/>
 * \tattendu Le point renvoye est centre + localOffset, sans deformation.
 * }
 */
TEST(DecorGeometryTest, DecorRotatedPointARotationNulleEstLIdentite) {
    const Rect bounds{Vector2{10.0f, 10.0f}, Vector2{4.0f, 2.0f}};  // centre (12, 11)

    const Vector2 point = hmi::decorRotatedPoint(bounds, Vector2{2.0f, -1.0f}, 0.0f);

    EXPECT_NEAR(point.x, 14.0f, 1e-4f);
    EXPECT_NEAR(point.y, 10.0f, 1e-4f);
}

/**
 * @brief `decorRotatedPoint` tourné d'un quart de tour (90°) déplace un point du bord droit vers le
 * bord bas, sens horaire (même convention que `hmi::SpriteBatch::draw` et `hmi::DecorGesture` — axe
 * Y vers le bas).
 * \castest{<b>decorRotatedPoint a 90 degres tourne dans le sens horaire.</b><br/>
 * \tcat Unitaire · Decor Geometry<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer decorRotatedPoint pour le coin superieur droit, tourne de 90 degres.<br/>
 * \tattendu Le point tourne coincide avec le coin inferieur droit non tourne.
 * }
 */
TEST(DecorGeometryTest, DecorRotatedPointA90DegresTourneDansLeSensHoraire) {
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 4.0f}};  // carre, centre (2, 2)
    constexpr float HALF_TURN_QUARTER = 1.57079632679f;           // pi/2

    // Coin superieur droit (2, -2) relatif au centre, tourne de 90 degres horaire.
    const Vector2 point = hmi::decorRotatedPoint(bounds, Vector2{2.0f, -2.0f}, HALF_TURN_QUARTER);

    // Coincide avec le coin inferieur droit NON tourne : (2, 2) relatif au centre -> (4, 4) absolu.
    EXPECT_NEAR(point.x, 4.0f, 1e-3f);
    EXPECT_NEAR(point.y, 4.0f, 1e-3f);
}

/**
 * @brief Les poignées de coin de `decorHandleLayout` tournent avec le décor (`LOT-50`, révision
 * post-livraison) : le cadre de sélection ne doit pas rester droit quand le décor pivote.
 * \castest{<b>Les poignees de decorHandleLayout tournent avec le decor.</b><br/>
 * \tcat Unitaire · Decor Geometry<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer les poignees d'un decor carre a rotation nulle, puis a 90 degres.<br/>
 * \tattendu Le coin haut-gauche a 90 degres coincide avec le coin haut-droit a rotation nulle.
 * }
 */
TEST(DecorGeometryTest, PoigneesDeCoinTournentAvecLeDecor) {
    const Rect bounds{Vector2{0.0f, 0.0f}, Vector2{4.0f, 4.0f}};  // carre, centre (2, 2)
    constexpr float HALF_TURN_QUARTER = 1.57079632679f;           // pi/2

    const hmi::DecorHandleLayout upright = hmi::decorHandleLayout(bounds, 0.1f, 0.0f);
    const hmi::DecorHandleLayout rotated = hmi::decorHandleLayout(bounds, 0.1f, HALF_TURN_QUARTER);

    const Vector2 uprightTopRightCenter{
        upright.topRight.position.x + upright.topRight.size.x * 0.5f,
        upright.topRight.position.y + upright.topRight.size.y * 0.5f};
    const Vector2 rotatedTopLeftCenter{rotated.topLeft.position.x + rotated.topLeft.size.x * 0.5f,
                                       rotated.topLeft.position.y + rotated.topLeft.size.y * 0.5f};

    EXPECT_NEAR(rotatedTopLeftCenter.x, uprightTopRightCenter.x, 1e-3f);
    EXPECT_NEAR(rotatedTopLeftCenter.y, uprightTopRightCenter.y, 1e-3f);
}
