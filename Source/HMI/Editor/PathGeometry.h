// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "Core/Levels/Level.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Editor/PathGeometry.h
 * @brief Géométrie **partagée** des poignées d'un parcours (LOT-67), pure et testable.
 *
 * Même parti que `hmi::DecorGeometry` : source unique pour le rendu (`hmi::DraftRenderer`) et pour
 * la détection (`hmi::PathGesture`). Si chacun calculait ses propres rectangles, ils finiraient par
 * diverger et les poignées cesseraient de répondre là où elles s'affichent.
 *
 * Les points de parcours viennent de `core::platformPathPoints`, la **même** primitive que celle
 * parcourue par le gameplay : ce qu'on manipule dans l'éditeur est exactement ce qui bougera.
 */

namespace hmi {

/// Nature d'une poignée de parcours.
enum class PathHandleKind {
    None,
    /// Point existant de la route : le glisser le déplace, le clic droit le retire.
    Waypoint,
    /// Milieu d'un segment : le glisser **insère** un point à cet endroit et le déplace du même
    /// geste — patron classique des éditeurs de courbe, qui évite un mode « ajouter » séparé.
    Midpoint,
    /// **Amorce** d'une route encore vide, posée sur la tuile de départ (`LOT-68`). Sans elle, une
    /// plateforme fraîchement peinte n'offre rien à saisir — ni point à déplacer, ni segment à
    /// couper — et son parcours ne peut jamais être commencé.
    ///
    /// La glisser n'a **pas** pour effet de déplacer le départ, qui se déplace en repeignant la
    /// tuile : elle crée le premier point de passage. La règle « le point de départ n'a pas de
    /// poignée » reste donc entière, puisque rien ici ne le déplace.
    Origin,
};

/**
 * @brief Une poignée de parcours : sa nature, son rang et son rectangle cliquable.
 *
 * Pour `Waypoint`, `index` est le rang dans `core::MovingPlatformConfig::waypoints`. Pour
 * `Midpoint`, c'est le rang qu'**occupera** le point inséré (`LevelDraft::insertPlatformWaypoint`),
 * ce qui recouvre aussi le segment de fermeture d'un circuit : y insérer un point revient à
 * l'ajouter en fin de route.
 */
struct PathHandle {
    PathHandleKind kind = PathHandleKind::None;
    std::size_t index = 0;
    core::Rect rect;
};

/// Taille écran (en pixels) d'une poignée de parcours, **constante quel que soit le zoom** — même
/// raison et même valeur que `DECOR_HANDLE_SCREEN_SIZE` : sinon les poignées deviennent
/// inutilisables aux extrêmes de zoom.
inline constexpr float PATH_HANDLE_SCREEN_SIZE = 10.0f;

/**
 * @brief Centres des poignées d'un parcours, en unités monde (centres de case).
 *
 * Fonction **pure**. Les points de `core::platformPathPoints` sont des coins haut-gauche de case ;
 * ils sont recentrés ici, une seule fois, pour que rendu et détection partagent la convention.
 * @param config Plateforme dont on calcule le parcours.
 * @return Les centres, dans l'ordre du parcours (départ inclus).
 */
[[nodiscard]] std::vector<core::Vector2> pathPointCenters(const core::MovingPlatformConfig& config);

/**
 * @brief Calcule les poignées d'un parcours sélectionné.
 *
 * Fonction **pure**. Une poignée `Waypoint` par point de route (le **départ n'en a pas** : il est
 * la tuile elle-même, qu'on déplace en repeignant, pas en glissant), et une poignée `Midpoint` au
 * milieu de chaque segment. Les rectangles ont une taille écran constante
 * (`PATH_HANDLE_SCREEN_SIZE`), convertie en unités monde via @p worldUnitsPerScreenPixel.
 * @param config                   Plateforme sélectionnée.
 * @param worldUnitsPerScreenPixel Échelle inverse de la caméra courante
 *                                 (`1 / (hmi::Camera2D::PIXELS_PER_UNIT * zoom)`).
 * @return Les poignées, poignées de point d'abord puis milieux de segment.
 */
[[nodiscard]] std::vector<PathHandle> pathHandleLayout(const core::MovingPlatformConfig& config,
                                                       float worldUnitsPerScreenPixel);

/**
 * @brief Calcule l'unique poignée d'un danger mobile : son extrémité de course.
 *
 * Fonction **pure**. Un `core::DangerMoverConfig` n'a pas de route libre — un axe et une portée
 * entière — donc une seule poignée, à l'extrémité, dont le glisser redéfinit les deux.
 * @param config                   Danger mobile sélectionné.
 * @param worldUnitsPerScreenPixel Même échelle que `pathHandleLayout`.
 * @return La poignée d'extrémité (`PathHandleKind::Waypoint`, rang 0).
 */
[[nodiscard]] PathHandle moverHandleLayout(const core::DangerMoverConfig& config,
                                           float worldUnitsPerScreenPixel);

/**
 * @brief Poignée de @p handles sous @p point, s'il y en a une.
 *
 * Fonction **pure**. Les poignées de point sont testées **avant** les milieux de segment : sur une
 * route courte, un milieu peut recouvrir un point, et déplacer un point existant est le geste
 * attendu par défaut — insérer reste accessible en visant franchement le milieu.
 * @param point   Point testé, en unités monde.
 * @param handles Poignées du parcours sélectionné (`pathHandleLayout`).
 * @return La poignée touchée, ou `std::nullopt` si aucune.
 */
[[nodiscard]] std::optional<PathHandle> hitTestPathHandles(
    core::Vector2 point, const std::vector<PathHandle>& handles) noexcept;

}  // namespace hmi
