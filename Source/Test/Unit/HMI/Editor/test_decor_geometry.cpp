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

    const hmi::DecorHandleLayout wide = hmi::decorHandleLayout(bounds, 0.1f);
    const hmi::DecorHandleLayout narrow = hmi::decorHandleLayout(bounds, 0.05f);

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
    const hmi::DecorHandleLayout layout = hmi::decorHandleLayout(bounds, 0.1f);

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
    const hmi::DecorHandleLayout layout = hmi::decorHandleLayout(bounds, 0.1f);

    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{0.0f, 0.0f}, layout), hmi::DecorHandle::TopLeft);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{4.0f, 0.0f}, layout), hmi::DecorHandle::TopRight);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{0.0f, 2.0f}, layout), hmi::DecorHandle::BottomLeft);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{4.0f, 2.0f}, layout), hmi::DecorHandle::BottomRight);
    // Centre de la poignee de rotation : x = centre du bord superieur (2.0), y = -rotationOffset
    // (24 * 0.1 = 2.4 unites monde au-dessus du bord superieur, en y=0).
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{2.0f, -2.4f}, layout), hmi::DecorHandle::Rotation);
    EXPECT_EQ(hmi::hitTestDecorHandles(Vector2{2.0f, 1.0f}, layout), hmi::DecorHandle::None);
}
