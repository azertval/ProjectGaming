// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/PathGeometry.h"

#include <algorithm>

#include "Core/Gameplay/PlatformPath.h"

namespace hmi {

namespace {

// Rectangle cliquable centre sur @p center, a taille ECRAN constante : la taille en pixels est
// convertie en unites monde, jamais l'inverse -- sinon la poignee grossirait avec le zoom.
[[nodiscard]] core::Rect handleRect(core::Vector2 center, float worldUnitsPerScreenPixel) {
    const float size = PATH_HANDLE_SCREEN_SIZE * worldUnitsPerScreenPixel;
    const float half = size * 0.5f;
    return core::Rect{core::Vector2{center.x - half, center.y - half}, core::Vector2{size, size}};
}

}  // namespace

std::vector<core::Vector2> pathPointCenters(const core::MovingPlatformConfig& config) {
    std::vector<core::Vector2> centers = core::platformPathPoints(config);
    for (core::Vector2& center : centers) {
        center.x += 0.5f;
        center.y += 0.5f;
    }
    return centers;
}

std::vector<PathHandle> pathHandleLayout(const core::MovingPlatformConfig& config,
                                         float worldUnitsPerScreenPixel) {
    const std::vector<core::Vector2> centers = pathPointCenters(config);
    std::vector<PathHandle> handles;
    if (centers.empty()) {
        return handles;  // configuration degeneree : rien a dessiner.
    }
    if (centers.size() < 2) {
        // Route VIDE : ni point a deplacer, ni segment a couper. Une seule poignee, sur la tuile
        // de depart, dont le glisser cree le PREMIER point de passage (LOT-68). Sans elle, une
        // plateforme fraichement peinte n'offre rien a saisir et son parcours reste impossible a
        // commencer -- defaut constate a l'essai.
        handles.push_back(
            PathHandle{.kind = PathHandleKind::Origin,
                       .index = 0,
                       .rect = handleRect(centers.front(), worldUnitsPerScreenPixel)});
        return handles;
    }
    handles.reserve(centers.size() * 2);

    // Poignees de point : centers[0] est le DEPART (la tuile elle-meme), qui n'en a pas -- on le
    // deplace en repeignant la tuile, pas en glissant. En mode boucle, le dernier point de
    // `centers` est une repetition du depart : il n'en a pas non plus.
    const std::size_t waypointCount = config.waypoints.size();
    for (std::size_t index = 0; index < waypointCount; ++index) {
        handles.push_back(
            PathHandle{.kind = PathHandleKind::Waypoint,
                       .index = index,
                       .rect = handleRect(centers[index + 1], worldUnitsPerScreenPixel)});
    }

    // Poignees de milieu : une par segment, y compris celui de fermeture d'un circuit -- y inserer
    // un point revient a l'ajouter en fin de route, ce que traduit deja le rang `segment`.
    for (std::size_t segment = 0; segment + 1 < centers.size(); ++segment) {
        const core::Vector2 from = centers[segment];
        const core::Vector2 to = centers[segment + 1];
        if (from == to) {
            continue;  // segment nul (point duplique) : pas de milieu exploitable.
        }
        handles.push_back(PathHandle{
            .kind = PathHandleKind::Midpoint,
            .index = segment,
            .rect = handleRect(core::Vector2{(from.x + to.x) * 0.5f, (from.y + to.y) * 0.5f},
                               worldUnitsPerScreenPixel)});
    }
    return handles;
}

PathHandle moverHandleLayout(const core::DangerMoverConfig& config,
                             float worldUnitsPerScreenPixel) {
    const float column =
        static_cast<float>(config.startPosition.column) +
        (config.axis == core::DangerMoverAxis::Horizontal ? static_cast<float>(config.range)
                                                          : 0.0f);
    const float row =
        static_cast<float>(config.startPosition.row) +
        (config.axis == core::DangerMoverAxis::Vertical ? static_cast<float>(config.range) : 0.0f);
    return PathHandle{
        .kind = PathHandleKind::Waypoint,
        .index = 0,
        .rect = handleRect(core::Vector2{column + 0.5f, row + 0.5f}, worldUnitsPerScreenPixel)};
}

std::optional<PathHandle> hitTestPathHandles(core::Vector2 point,
                                             const std::vector<PathHandle>& handles) noexcept {
    // Deux passes plutot qu'une : sur une route courte, un milieu de segment peut recouvrir un
    // point existant, et DEPLACER un point est le geste attendu par defaut.
    for (PathHandleKind kind :
         {PathHandleKind::Waypoint, PathHandleKind::Midpoint, PathHandleKind::Origin}) {
        const auto found =
            std::find_if(handles.begin(), handles.end(), [kind, point](const PathHandle& handle) {
                return handle.kind == kind && handle.rect.contains(point);
            });
        if (found != handles.end()) {
            return *found;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
