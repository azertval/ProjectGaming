/**
 * @file test_path_geometry.cpp
 * @brief Tests unitaires de la géométrie des poignées de parcours (LOT-67 TACHE-04,
 *        `EX-EDIT-032`) — purs, sans Qt ni GPU.
 */

#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/PathGeometry.h"

namespace {

// Plateforme en (1,1) suivant la route donnee, dans le mode donne.
core::MovingPlatformConfig platform(
    std::vector<core::GridPosition> waypoints,
    core::PlatformPathMode mode = core::PlatformPathMode::PingPong) {
    return core::MovingPlatformConfig{
        .startPosition = core::GridPosition{1, 1}, .waypoints = std::move(waypoints), .mode = mode};
}

// Nombre de poignees de la nature donnee.
std::size_t countOf(const std::vector<hmi::PathHandle>& handles, hmi::PathHandleKind kind) {
    std::size_t count = 0;
    for (const hmi::PathHandle& handle : handles) {
        if (handle.kind == kind) {
            ++count;
        }
    }
    return count;
}

}  // namespace

/**
 * @brief Le point de départ n'a pas de poignée : c'est la tuile elle-même, déplacée en repeignant.
 * \castest{<b>Le point de départ n'a pas de poignée.</b><br/>
 * \tcat Unitaire · Path Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer les poignees d'une route a deux points.<br/>2. Compter les poignees de
 * point.<br/>
 * \tattendu Il y a exactement autant de poignees de point que de waypoints, soit une de moins que
 * de sommets du parcours.
 * }
 */
TEST(PathGeometryTest, LePointDeDepartNAPasDePoignee) {
    const std::vector<hmi::PathHandle> handles = hmi::pathHandleLayout(
        platform({core::GridPosition{4, 1}, core::GridPosition{4, 4}}), 0.02f);

    EXPECT_EQ(countOf(handles, hmi::PathHandleKind::Waypoint), 2u);
    // Trois sommets (depart + deux points) donc deux segments, donc deux milieux.
    EXPECT_EQ(countOf(handles, hmi::PathHandleKind::Midpoint), 2u);
}

/**
 * @brief Un circuit fermé expose une poignée de milieu supplémentaire, sur le segment de
 *        fermeture : y insérer un point revient à l'ajouter en fin de route (`EX-GP-054`).
 * \castest{<b>Un circuit fermé expose un milieu de segment supplémentaire.</b><br/>
 * \tcat Unitaire · Path Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer les poignees d'une meme route en aller-retour puis en boucle.<br/>2.
 * Comparer le nombre de milieux et le rang du dernier.<br/>
 * \tattendu La boucle a un milieu de plus, de rang egal au nombre de waypoints : inserer la revient
 * a ajouter un point en fin de route.
 * }
 */
TEST(PathGeometryTest, CircuitFermeExposeUnMilieuDeFermeture) {
    const std::vector<core::GridPosition> route{core::GridPosition{4, 1}, core::GridPosition{4, 4}};
    const std::vector<hmi::PathHandle> pingPong = hmi::pathHandleLayout(platform(route), 0.02f);
    const std::vector<hmi::PathHandle> loop =
        hmi::pathHandleLayout(platform(route, core::PlatformPathMode::Loop), 0.02f);

    EXPECT_EQ(countOf(pingPong, hmi::PathHandleKind::Midpoint), 2u);
    ASSERT_EQ(countOf(loop, hmi::PathHandleKind::Midpoint), 3u);
    EXPECT_EQ(loop.back().index, route.size());  // insertion en fin de route
}

/**
 * @brief Les poignées gardent une taille **écran** constante : leur taille en unités monde suit
 *        l'échelle de la caméra, jamais l'inverse (`EX-EDIT-030`).
 * \castest{<b>Les poignées gardent une taille écran constante quel que soit le zoom.</b><br/>
 * \tcat Unitaire · Path Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer les poignees a deux echelles de camera differentes.<br/>2. Comparer leur
 * taille en unites monde.<br/>
 * \tattendu La taille monde est proportionnelle a l'echelle : la taille apparente a l'ecran reste
 * la meme, sinon les poignees deviendraient inutilisables aux extremes de zoom.
 * }
 */
TEST(PathGeometryTest, PoigneesGardentUneTailleEcranConstante) {
    const core::MovingPlatformConfig config = platform({core::GridPosition{4, 1}});

    const std::vector<hmi::PathHandle> zoomedOut = hmi::pathHandleLayout(config, 0.04f);
    const std::vector<hmi::PathHandle> zoomedIn = hmi::pathHandleLayout(config, 0.01f);
    ASSERT_FALSE(zoomedOut.empty());
    ASSERT_FALSE(zoomedIn.empty());

    EXPECT_FLOAT_EQ(zoomedOut.front().rect.size.x, hmi::PATH_HANDLE_SCREEN_SIZE * 0.04f);
    EXPECT_FLOAT_EQ(zoomedIn.front().rect.size.x, hmi::PATH_HANDLE_SCREEN_SIZE * 0.01f);
}

/**
 * @brief Un point existant l'emporte sur un milieu de segment qui le recouvrirait : déplacer est le
 *        geste attendu par défaut, insérer reste accessible en visant franchement le milieu.
 * \castest{<b>Un point existant l'emporte sur un milieu de segment qui le recouvre.</b><br/>
 * \tcat Unitaire · Path Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire des poignees ou un milieu recouvre un point (poignees tres larges).<br/>2.
 * Tester un point commun aux deux.<br/>
 * \tattendu La poignee retournee est celle du point, pas celle du milieu.
 * }
 */
TEST(PathGeometryTest, PointExistantLEmporteSurLeMilieuDeSegment) {
    // Poignees volontairement enormes (echelle 1 unite/pixel) pour garantir le recouvrement.
    const std::vector<hmi::PathHandle> handles =
        hmi::pathHandleLayout(platform({core::GridPosition{2, 1}}), 1.0f);

    const std::optional<hmi::PathHandle> hit =
        hmi::hitTestPathHandles(core::Vector2{2.5f, 1.5f}, handles);
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(hit->kind, hmi::PathHandleKind::Waypoint);
}

/**
 * @brief Un danger mobile expose une seule poignée, à l'extrémité de sa course (`EX-GP-051`).
 * \castest{<b>Un danger mobile expose une seule poignée, à l'extrémité de sa course.</b><br/>
 * \tcat Unitaire · Path Geometry<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer la poignee d'un danger mobile horizontal de portee 3.<br/>2. Verifier la
 * position de son centre.<br/>
 * \tattendu La poignee est centree sur la case situee trois cases a droite du depart.
 * }
 */
TEST(PathGeometryTest, DangerMobileExposeUnePoigneeDExtremite) {
    const core::DangerMoverConfig config{.startPosition = core::GridPosition{2, 5},
                                         .axis = core::DangerMoverAxis::Horizontal,
                                         .range = 3};

    const hmi::PathHandle handle = hmi::moverHandleLayout(config, 0.02f);

    EXPECT_EQ(handle.kind, hmi::PathHandleKind::Waypoint);
    const float centerX = handle.rect.position.x + (handle.rect.size.x * 0.5f);
    const float centerY = handle.rect.position.y + (handle.rect.size.y * 0.5f);
    EXPECT_NEAR(centerX, 5.5f, 1e-4f);  // 2 + 3 cases, centre de case
    EXPECT_NEAR(centerY, 5.5f, 1e-4f);
}

/**
 * @brief Une plateforme sans route expose **une** poignée d'amorce, sur sa tuile de départ
 *        (`LOT-68`).
 *
 * Ce test disait l'inverse jusqu'au `LOT-68` : il figeait l'absence de poignée comme un
 * comportement voulu. C'était le défaut — sans point à déplacer ni segment à couper, une plateforme
 * fraîchement peinte n'offrait rien à saisir, et son parcours ne pouvait jamais être commencé.
 * L'amorce ne **déplace** pas le départ (on le déplace en repeignant la tuile) : elle crée le
 * premier point de passage, ce qui laisse entière la règle « le point de départ n'a pas de
 * poignée ».
 * \castest{<b>Une plateforme sans route expose une poignee d'amorce sur son depart.</b><br/>
 * \tcat Unitaire · Path Geometry<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer les poignees d'une plateforme sans waypoint.<br/>2. Verifier la nature de
 * l'unique poignee et sa position.<br/>
 * \tattendu Une seule poignee, de nature Origin, centree sur la tuile de depart.
 * }
 */
TEST(PathGeometryTest, PlateformeSansRouteExposeUnePoigneeDAmorce) {
    const std::vector<hmi::PathHandle> handles = hmi::pathHandleLayout(platform({}), 0.02f);

    ASSERT_EQ(handles.size(), 1u) << "sans poignee, le parcours d'une plateforme fraichement "
                                     "peinte ne peut pas etre commence";
    EXPECT_EQ(handles.front().kind, hmi::PathHandleKind::Origin);
    EXPECT_EQ(handles.front().index, 0u);
    // Centree sur la tuile de depart de la fixture, comme toute poignee de point.
    const hmi::PathHandle& handle = handles.front();
    const float centerX = handle.rect.position.x + (handle.rect.size.x * 0.5f);
    const float centerY = handle.rect.position.y + (handle.rect.size.y * 0.5f);
    const std::vector<core::Vector2> centers = hmi::pathPointCenters(platform({}));
    ASSERT_FALSE(centers.empty());
    EXPECT_NEAR(centerX, centers.front().x, 1e-4f);
    EXPECT_NEAR(centerY, centers.front().y, 1e-4f);
}
