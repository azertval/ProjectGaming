// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_platform_path.cpp
 * @brief Tests unitaires de la route d'une plateforme mobile (`core::PlatformPath`, LOT-67,
 *        `EX-GP-054`) — la primitive **partagée** par le gameplay et l'overlay d'édition.
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/PlatformPath.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Math/Vector2.h"

namespace {

core::MovingPlatformConfig config(std::vector<core::GridPosition> waypoints,
                                  core::PlatformPathMode mode) {
    return core::MovingPlatformConfig{
        .startPosition = core::GridPosition{1, 1}, .waypoints = std::move(waypoints), .mode = mode};
}

}  // namespace

/**
 * @brief Les sommets d'une route commencent par le départ, sans le répéter, et suivent l'ordre des
 *        waypoints (`EX-GP-054`).
 * \castest{<b>Les sommets d'une route partent du départ et suivent l'ordre des waypoints.</b><br/>
 * \tcat Unitaire · Platform Path<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Calculer les sommets d'une route a deux points en aller-retour.<br/>
 * \tattendu Trois sommets : le depart puis les deux waypoints, en coins haut-gauche de case.
 * }
 */
TEST(PlatformPathTest, LesSommetsPartentDuDepartSansLeRepeter) {
    const std::vector<core::Vector2> points = core::platformPathPoints(config(
        {core::GridPosition{4, 1}, core::GridPosition{4, 3}}, core::PlatformPathMode::PingPong));

    ASSERT_EQ(points.size(), 3u);
    EXPECT_FLOAT_EQ(points[0].x, 1.0f);
    EXPECT_FLOAT_EQ(points[0].y, 1.0f);
    EXPECT_FLOAT_EQ(points[1].x, 4.0f);
    EXPECT_FLOAT_EQ(points[2].y, 3.0f);
}

/**
 * @brief Un circuit fermé répète le point de départ en fin de liste : c'est ce qui matérialise le
 *        segment de fermeture, pour le gameplay comme pour l'overlay (`EX-GP-054`).
 * \castest{<b>Un circuit fermé répète le point de départ en fin de liste.</b><br/>
 * \tcat Unitaire · Platform Path<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Calculer les sommets d'une meme route en aller-retour puis en boucle.<br/>
 * \tattendu La boucle a un sommet de plus, identique au depart ; l'aller-retour n'en a pas.
 * }
 */
TEST(PlatformPathTest, CircuitFermeRepeteLeDepartEnFinDeListe) {
    const std::vector<core::GridPosition> route{core::GridPosition{4, 1}, core::GridPosition{4, 3}};
    const std::vector<core::Vector2> pingPong =
        core::platformPathPoints(config(route, core::PlatformPathMode::PingPong));
    const std::vector<core::Vector2> loop =
        core::platformPathPoints(config(route, core::PlatformPathMode::Loop));

    EXPECT_EQ(pingPong.size(), 3u);
    ASSERT_EQ(loop.size(), 4u);
    EXPECT_FLOAT_EQ(loop.back().x, loop.front().x);
    EXPECT_FLOAT_EQ(loop.back().y, loop.front().y);
}

/**
 * @brief Une route sans waypoint n'est jamais fermée : il n'y a rien à refermer, et le mode boucle
 *        ne doit pas produire un sommet parasite (`EX-NFR-040`).
 * \castest{<b>Une route sans waypoint n'est jamais fermée.</b><br/>
 * \tcat Unitaire · Platform Path<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Calculer les sommets d'une plateforme sans waypoint, en mode boucle.<br/>
 * \tattendu Un seul sommet (le depart), et un cycle de longueur nulle.
 * }
 */
TEST(PlatformPathTest, RouteSansWaypointNEstJamaisFermee) {
    const core::PlatformPath path =
        core::buildPlatformPath(config({}, core::PlatformPathMode::Loop));

    EXPECT_EQ(path.points.size(), 1u);
    EXPECT_DOUBLE_EQ(path.totalLength, 0.0);
    EXPECT_DOUBLE_EQ(path.cycleLength, 0.0);
}

/**
 * @brief Le cycle vaut le double de la route en aller-retour, et son périmètre en circuit fermé —
 *        c'est ce qui distingue les deux modes pour le gameplay (`EX-GP-054`).
 * \castest{<b>Le cycle vaut le double de la route en aller-retour, son périmètre en
 * boucle.</b><br/>
 * \tcat Unitaire · Platform Path<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Construire un carre de 2 cases de cote dans les deux modes.<br/>
 * \tattendu En aller-retour, la route mesure 6 cases et le cycle 12 ; en boucle, la route inclut la
 * fermeture (8 cases) et le cycle vaut ce perimetre.
 * }
 */
TEST(PlatformPathTest, LeCycleDistingueAllerRetourEtCircuitFerme) {
    const std::vector<core::GridPosition> square{core::GridPosition{3, 1}, core::GridPosition{3, 3},
                                                 core::GridPosition{1, 3}};

    const core::PlatformPath pingPong =
        core::buildPlatformPath(config(square, core::PlatformPathMode::PingPong));
    EXPECT_DOUBLE_EQ(pingPong.totalLength, 6.0);
    EXPECT_DOUBLE_EQ(pingPong.cycleLength, 12.0);

    const core::PlatformPath loop =
        core::buildPlatformPath(config(square, core::PlatformPathMode::Loop));
    EXPECT_DOUBLE_EQ(loop.totalLength, 8.0);  // fermeture comprise
    EXPECT_DOUBLE_EQ(loop.cycleLength, 8.0);
}

/**
 * @brief `platformPositionAt` replie une distance négative sur le cycle : un déphasage négatif
 *        reste licite et déterministe (`EX-NFR-002`).
 * \castest{<b>Une distance négative est repliée sur le cycle.</b><br/>
 * \tcat Unitaire · Platform Path<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger la position a une distance negative et a son equivalent positif.<br/>
 * \tattendu Les deux positions coincident.
 * }
 */
TEST(PlatformPathTest, DistanceNegativeEstRepliueSurLeCycle) {
    const core::PlatformPath path = core::buildPlatformPath(
        config({core::GridPosition{5, 1}}, core::PlatformPathMode::PingPong));
    ASSERT_DOUBLE_EQ(path.cycleLength, 8.0);  // aller-retour de 4 cases

    const core::Vector2 negative = core::platformPositionAt(path, -3.0);
    const core::Vector2 positive = core::platformPositionAt(path, 5.0);  // -3 + 8

    EXPECT_NEAR(negative.x, positive.x, 1e-4f);
    EXPECT_NEAR(negative.y, positive.y, 1e-4f);
}
