// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/PlatformPath.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace core {

namespace {

// Coin haut-gauche, en unites monde, de la case @p position -- une case fait exactement une unite
// (Aabb::fromTopLeftSize dans PlatformController), l'origine du monde est le coin de la case (0,0).
[[nodiscard]] Vector2 cellTopLeft(GridPosition position) {
    return Vector2{static_cast<float>(position.column), static_cast<float>(position.row)};
}

}  // namespace

std::vector<Vector2> platformPathPoints(const MovingPlatformConfig& config) {
    std::vector<Vector2> points;
    points.reserve(config.waypoints.size() + 2);
    points.push_back(cellTopLeft(config.startPosition));
    for (const GridPosition& waypoint : config.waypoints) {
        points.push_back(cellTopLeft(waypoint));
    }
    // Circuit ferme : le segment de retour vers le depart fait partie du parcours et se parcourt a
    // la meme vitesse que les autres. Sans waypoint il n'y a rien a fermer.
    if (config.mode == PlatformPathMode::Loop && !config.waypoints.empty()) {
        points.push_back(points.front());
    }
    return points;
}

PlatformPath buildPlatformPath(const MovingPlatformConfig& config) {
    PlatformPath path;
    path.points = platformPathPoints(config);
    path.cumulative.reserve(path.points.size());
    path.cumulative.push_back(0.0);
    for (std::size_t index = 1; index < path.points.size(); ++index) {
        const Vector2 segment = path.points[index] - path.points[index - 1];
        path.cumulative.push_back(path.cumulative.back() + static_cast<double>(segment.length()));
    }
    path.totalLength = path.cumulative.back();
    path.cycleLength =
        config.mode == PlatformPathMode::Loop ? path.totalLength : path.totalLength * 2.0;
    return path;
}

Vector2 platformPositionAt(const PlatformPath& path, double travelled) noexcept {
    // Route degeneree (aucun waypoint, ou tous confondus avec le depart) : plateforme immobile.
    if (path.cycleLength <= 0.0) {
        return path.points.front();
    }

    double offset = std::fmod(travelled, path.cycleLength);
    if (offset < 0.0) {
        offset += path.cycleLength;
    }
    // Aller-retour : au-dela de l'aller, on refait le parcours a l'envers (onde triangulaire).
    // Circuit ferme : rien a replier, offset parcourt directement tout le perimetre.
    if (offset > path.totalLength) {
        offset = path.cycleLength - offset;
    }

    // Dernier sommet dont la distance cumulee ne depasse pas `offset`. upper_bound saute d'un coup
    // toutes les valeurs egales : le segment retenu a donc TOUJOURS une longueur strictement
    // positive, ce qui rend impossible la division par zero sur un waypoint duplique.
    const auto upper = std::upper_bound(path.cumulative.begin(), path.cumulative.end(), offset);
    if (upper == path.cumulative.end()) {
        return path.points.back();
    }
    const auto index = static_cast<std::size_t>(std::distance(path.cumulative.begin(), upper)) - 1;
    const double segmentLength = path.cumulative[index + 1] - path.cumulative[index];
    const auto ratio = static_cast<float>((offset - path.cumulative[index]) / segmentLength);
    return path.points[index] + ((path.points[index + 1] - path.points[index]) * ratio);
}

}  // namespace core
