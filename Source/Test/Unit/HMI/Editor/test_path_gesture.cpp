/**
 * @file test_path_gesture.cpp
 * @brief Tests unitaires du geste de manipulation de parcours (LOT-67 TACHE-04, `EX-EDIT-032`) —
 *        purs, sans Qt ni GPU.
 */

#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/PathGesture.h"

namespace {

const std::vector<core::MovingPlatformConfig> PLATFORMS{core::MovingPlatformConfig{
    .startPosition = core::GridPosition{1, 1}, .waypoints = {core::GridPosition{5, 1}}}};

const std::vector<core::DangerMoverConfig> MOVERS{
    core::DangerMoverConfig{.startPosition = core::GridPosition{2, 5},
                            .axis = core::DangerMoverAxis::Horizontal,
                            .range = 2}};

// Poignees de la plateforme 0, a une echelle de camera realiste.
std::vector<hmi::PathHandle> platformHandles() {
    return hmi::pathHandleLayout(PLATFORMS.front(), 0.02f);
}

constexpr hmi::PathSelection PLATFORM_0{.kind = hmi::PathTargetKind::Platform, .index = 0};

}  // namespace

/**
 * @brief Cliquer la case d'une plateforme sélectionne son parcours sans rien manipuler.
 * \castest{<b>Cliquer la case d'une plateforme sélectionne son parcours.</b><br/>
 * \tcat Unitaire · Path Gesture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Designer un point dans la case de depart de la plateforme.<br/>2. Amorcer le
 * geste.<br/>
 * \tattendu Le parcours est selectionne, aucune poignee n'est armee et la phase reste au repos :
 * un glisser depuis la tuile ne deplacera aucun point par inadvertance.
 * }
 */
TEST(PathGestureTest, CliquerLaCaseSelectionneLeParcours) {
    const std::optional<hmi::PathHit> hit =
        hmi::designatePathAt(core::Vector2{1.5f, 1.5f}, PLATFORMS, MOVERS, std::nullopt, {});
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(hit->target, PLATFORM_0);
    EXPECT_EQ(hit->handle.kind, hmi::PathHandleKind::None);

    hmi::PathGestureState state;
    hmi::beginPathGesture(state, *hit, core::Vector2{1.5f, 1.5f}, core::GridPosition{});
    ASSERT_TRUE(state.selected.has_value());
    EXPECT_EQ(*state.selected, PLATFORM_0);
    EXPECT_EQ(state.phase, hmi::PathGesturePhase::Idle);
}

/**
 * @brief Un clic sans mouvement ne produit aucune action : le seuil de glisser évite tout
 *        micro-déplacement involontaire d'un point.
 * \castest{<b>Un clic sans mouvement ne déplace aucun point.</b><br/>
 * \tcat Unitaire · Path Gesture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Amorcer un geste sur une poignee de point.<br/>2. Relacher au meme endroit.<br/>
 * \tattendu Aucune action n'est produite, ni en apercu ni au relachement.
 * }
 */
TEST(PathGestureTest, ClicSansMouvementNeProduitAucuneAction) {
    const std::vector<hmi::PathHandle> handles = platformHandles();
    const std::optional<hmi::PathHit> hit =
        hmi::designatePathAt(core::Vector2{5.5f, 1.5f}, PLATFORMS, MOVERS, PLATFORM_0, handles);
    ASSERT_TRUE(hit.has_value());
    ASSERT_EQ(hit->handle.kind, hmi::PathHandleKind::Waypoint);

    hmi::PathGestureState state;
    hmi::beginPathGesture(state, *hit, core::Vector2{5.5f, 1.5f}, core::GridPosition{});
    EXPECT_EQ(hmi::updatePathGesture(state, core::Vector2{5.51f, 1.5f}).kind,
              hmi::PathGestureActionKind::None);
    EXPECT_EQ(hmi::endPathGesture(state, core::Vector2{5.51f, 1.5f}).kind,
              hmi::PathGestureActionKind::None);
}

/**
 * @brief Glisser une poignée de point produit un déplacement, aimanté sur la case visée : un point
 *        de parcours **est** une case (`EX-GP-054`).
 * \castest{<b>Glisser une poignée de point le déplace sur la case visée.</b><br/>
 * \tcat Unitaire · Path Gesture<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Amorcer un geste sur la poignee du point de rang 0.<br/>2. Glisser bien au-dela du
 * seuil puis relacher.<br/>
 * \tattendu Une action de deplacement est produite, pour le bon rang et sur la case visee.
 * }
 */
TEST(PathGestureTest, GlisserUnePoigneeDeplaceLePoint) {
    const std::vector<hmi::PathHandle> handles = platformHandles();
    const std::optional<hmi::PathHit> hit =
        hmi::designatePathAt(core::Vector2{5.5f, 1.5f}, PLATFORMS, MOVERS, PLATFORM_0, handles);
    ASSERT_TRUE(hit.has_value());

    hmi::PathGestureState state;
    hmi::beginPathGesture(state, *hit, core::Vector2{5.5f, 1.5f}, core::GridPosition{});
    const hmi::PathGestureAction action = hmi::endPathGesture(state, core::Vector2{8.3f, 4.7f});

    EXPECT_EQ(action.kind, hmi::PathGestureActionKind::MoveWaypoint);
    EXPECT_EQ(action.target, PLATFORM_0);
    EXPECT_EQ(action.waypointIndex, 0u);
    EXPECT_EQ(action.position, (core::GridPosition{8, 4}));
}

/**
 * @brief Glisser le milieu d'un segment **insère** un point à ce rang, sans toucher aux points
 *        existants (`EX-EDIT-032`).
 * \castest{<b>Glisser le milieu d'un segment y insère un point.</b><br/>
 * \tcat Unitaire · Path Gesture<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Amorcer un geste sur la poignee de milieu du premier segment.<br/>2. Glisser puis
 * relacher.<br/>
 * \tattendu Une action d'insertion est produite, au rang du segment coupe et sur la case visee.
 * }
 */
TEST(PathGestureTest, GlisserUnMilieuInsereUnPoint) {
    const std::vector<hmi::PathHandle> handles = platformHandles();
    // Milieu du segment (1,1)->(5,1), soit le centre de case x = 3,5.
    const std::optional<hmi::PathHit> hit =
        hmi::designatePathAt(core::Vector2{3.5f, 1.5f}, PLATFORMS, MOVERS, PLATFORM_0, handles);
    ASSERT_TRUE(hit.has_value());
    ASSERT_EQ(hit->handle.kind, hmi::PathHandleKind::Midpoint);

    hmi::PathGestureState state;
    hmi::beginPathGesture(state, *hit, core::Vector2{3.5f, 1.5f}, core::GridPosition{});
    const hmi::PathGestureAction action = hmi::endPathGesture(state, core::Vector2{3.2f, 6.8f});

    EXPECT_EQ(action.kind, hmi::PathGestureActionKind::InsertWaypoint);
    EXPECT_EQ(action.waypointIndex, 0u);  // le point insere prend le rang du segment coupe
    EXPECT_EQ(action.position, (core::GridPosition{3, 6}));
}

/**
 * @brief Un geste complet ne produit qu'**une seule** action finale : l'historique d'annulation
 *        n'est jamais spammé par les positions intermédiaires (`EX-EDIT-005`).
 * \castest{<b>Un geste complet ne produit qu'une seule action finale.</b><br/>
 * \tcat Unitaire · Path Gesture<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Amorcer un geste puis enchainer plusieurs mises a jour d'apercu.<br/>2.
 * Relacher.<br/>
 * \tattendu Les apercus decrivent la manipulation mais ne sont que des apercus ; le relachement
 * produit l'action a appliquer, et un second relachement n'en produit plus aucune.
 * }
 */
TEST(PathGestureTest, UnGesteCompletNeProduitQuUneSeuleActionFinale) {
    const std::vector<hmi::PathHandle> handles = platformHandles();
    const std::optional<hmi::PathHit> hit =
        hmi::designatePathAt(core::Vector2{5.5f, 1.5f}, PLATFORMS, MOVERS, PLATFORM_0, handles);
    ASSERT_TRUE(hit.has_value());

    hmi::PathGestureState state;
    hmi::beginPathGesture(state, *hit, core::Vector2{5.5f, 1.5f}, core::GridPosition{});
    for (float x = 6.0f; x < 9.0f; x += 1.0f) {
        EXPECT_EQ(hmi::updatePathGesture(state, core::Vector2{x, 1.5f}).kind,
                  hmi::PathGestureActionKind::MoveWaypoint);
    }

    EXPECT_EQ(hmi::endPathGesture(state, core::Vector2{9.5f, 1.5f}).kind,
              hmi::PathGestureActionKind::MoveWaypoint);
    // Le geste est termine : plus rien a appliquer, la selection persiste.
    EXPECT_EQ(hmi::endPathGesture(state, core::Vector2{9.5f, 1.5f}).kind,
              hmi::PathGestureActionKind::None);
    EXPECT_TRUE(state.selected.has_value());
}

/**
 * @brief Abandonner un glisser (`Échap`) ne produit aucune action et conserve la sélection.
 * \castest{<b>Abandonner un glisser ne produit aucune action.</b><br/>
 * \tcat Unitaire · Path Gesture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Amorcer un glisser au-dela du seuil.<br/>2. L'abandonner puis tenter de le
 * terminer.<br/>
 * \tattendu Aucune action n'est produite et la selection reste en place.
 * }
 */
TEST(PathGestureTest, AbandonnerUnGlisserNeProduitAucuneAction) {
    const std::vector<hmi::PathHandle> handles = platformHandles();
    const std::optional<hmi::PathHit> hit =
        hmi::designatePathAt(core::Vector2{5.5f, 1.5f}, PLATFORMS, MOVERS, PLATFORM_0, handles);
    ASSERT_TRUE(hit.has_value());

    hmi::PathGestureState state;
    hmi::beginPathGesture(state, *hit, core::Vector2{5.5f, 1.5f}, core::GridPosition{});
    (void)hmi::updatePathGesture(state, core::Vector2{9.0f, 1.5f});
    hmi::cancelPathGesture(state);

    EXPECT_EQ(state.phase, hmi::PathGesturePhase::Idle);
    EXPECT_EQ(hmi::endPathGesture(state, core::Vector2{9.0f, 1.5f}).kind,
              hmi::PathGestureActionKind::None);
    EXPECT_TRUE(state.selected.has_value());
}

/**
 * @brief Glisser l'extrémité d'un danger mobile en redéfinit l'axe **et** la portée, jamais
 *        négative (`EX-GP-051`).
 * \castest{<b>Glisser l'extrémité d'un danger mobile redéfinit son axe et sa portée.</b><br/>
 * \tcat Unitaire · Path Gesture<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Glisser l'extremite vers le bas (deplacement vertical dominant).<br/>2. Recommencer
 * en glissant vers la gauche, au-dela du depart.<br/>
 * \tattendu Le premier glisser bascule sur l'axe vertical avec la bonne portee ; le second, qui
 * pointerait une portee negative, la ramene a zero -- DangerMoverConfig decrit une course dans le
 * sens positif de son axe.
 * }
 */
TEST(PathGestureTest, GlisserUnDangerMobileRedefinitAxeEtPortee) {
    const hmi::PathSelection mover0{.kind = hmi::PathTargetKind::Mover, .index = 0};
    const std::vector<hmi::PathHandle> handles = {hmi::moverHandleLayout(MOVERS.front(), 0.02f)};
    const std::optional<hmi::PathHit> hit =
        hmi::designatePathAt(core::Vector2{4.5f, 5.5f}, PLATFORMS, MOVERS, mover0, handles);
    ASSERT_TRUE(hit.has_value());

    hmi::PathGestureState state;
    hmi::beginPathGesture(state, *hit, core::Vector2{4.5f, 5.5f}, MOVERS.front().startPosition);
    const hmi::PathGestureAction vertical = hmi::endPathGesture(state, core::Vector2{2.5f, 9.5f});
    EXPECT_EQ(vertical.kind, hmi::PathGestureActionKind::SetMoverRange);
    EXPECT_EQ(vertical.axis, core::DangerMoverAxis::Vertical);
    EXPECT_EQ(vertical.range, 4);  // de la ligne 5 a la ligne 9

    hmi::beginPathGesture(state, *hit, core::Vector2{4.5f, 5.5f}, MOVERS.front().startPosition);
    const hmi::PathGestureAction clamped = hmi::endPathGesture(state, core::Vector2{0.5f, 5.5f});
    EXPECT_EQ(clamped.axis, core::DangerMoverAxis::Horizontal);
    EXPECT_EQ(clamped.range, 0);  // jamais negative
}
