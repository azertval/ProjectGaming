// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <optional>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Math/Vector2.h"
#include "HMI/Editor/PathGeometry.h"

/**
 * @file HMI/Editor/PathGesture.h
 * @brief Machine à états du geste de manipulation de parcours (LOT-67), pure et testable.
 *
 * Parti pris : désignation, distinction clic/glisser et calcul de l'action
 * sont des fonctions déterministes sur des données, sans Qt ni GPU — le viewport n'en est que le
 * routeur d'événements. Le geste **ne mute rien** : il décrit une action, l'appelant l'applique aux
 * mutateurs de `core::LevelDraft`. Un geste complet produit **une seule** action finale, donc un
 * seul pas d'annulation : l'historique n'est jamais spammé par les positions intermédiaires.
 */

namespace hmi {

/// Nature de l'élément dont on manipule le parcours.
enum class PathTargetKind {
    Platform,  ///< `core::MovingPlatformConfig` : route libre à N points.
    Mover,     ///< `core::DangerMoverConfig` : course contrainte sur un axe.
};

/// Parcours sélectionné : quel type d'élément, et son rang dans le vecteur correspondant du
/// brouillon (`platformConfigs()` ou `moverConfigs()`).
struct PathSelection {
    PathTargetKind kind = PathTargetKind::Platform;
    std::size_t index = 0;

    friend bool operator==(const PathSelection&, const PathSelection&) = default;
};

/// Élément visé par un clic (voir `hmi::designatePathAt`).
struct PathHit {
    PathSelection target;
    PathHandle handle;
};

/// Seuil de déplacement (unités monde) distinguant un clic (sélection simple) d'un glisser — même
/// seuil au-delà duquel un appui devient un glisser plutôt qu'un clic.
inline constexpr float PATH_DRAG_THRESHOLD = 0.1f;

/// Nature de l'action produite par le geste.
enum class PathGestureActionKind {
    None,            ///< Rien à appliquer (glisser sous le seuil, ou simple clic de sélection).
    MoveWaypoint,    ///< Déplacer le point `waypointIndex` vers `position`.
    InsertWaypoint,  ///< Insérer un point au rang `waypointIndex`, à `position`.
    RemoveWaypoint,  ///< Retirer le point `waypointIndex`.
    SetMoverRange,   ///< Redéfinir l'axe et la portée d'un danger mobile.
};

/// Action à appliquer par l'appelant : en aperçu pendant le glisser (`updatePathGesture`, consommée
/// par le rendu), ou pour de bon au relâchement (`endPathGesture`, consommée par les mutateurs).
struct PathGestureAction {
    PathGestureActionKind kind = PathGestureActionKind::None;
    PathSelection target;
    std::size_t waypointIndex = 0;
    core::GridPosition position{};
    core::DangerMoverAxis axis = core::DangerMoverAxis::Horizontal;
    int range = 0;
};

/// Phase du geste en cours.
enum class PathGesturePhase {
    Idle,      ///< Aucun geste en cours.
    Pressed,   ///< Bouton pressé, sous le seuil : pas encore de manipulation.
    Dragging,  ///< Glisser au-delà du seuil : une manipulation est en cours.
};

/**
 * @brief État complet du geste, possédé par l'appelant.
 *
 * `selected` **persiste** au-delà d'un geste terminé : c'est la sélection courante de l'éditeur,
 * distincte de la phase du geste — c'est elle qui pilote l'affichage des poignées et le contenu du
 * panneau de propriétés.
 */
struct PathGestureState {
    PathGesturePhase phase = PathGesturePhase::Idle;
    std::optional<PathSelection> selected;
    PathHandle handle;
    core::Vector2 pressPosition{};
    /// Position de départ du danger mobile à l'instant de l'appui, pour recalculer axe et portée.
    core::GridPosition moverStart{};
};

/**
 * @brief Désigne l'élément sous @p point : quel parcours, et quelle poignée le cas échéant.
 *
 * Fonction **pure**. Priorité aux poignées du parcours **déjà sélectionné** (@p selectedHandles,
 * un parcours non sélectionné n'en affiche pas) ; à défaut, à la **case de départ** d'un parcours,
 * qui sert de zone de sélection — cliquer la tuile d'une plateforme ou d'un danger mobile
 * sélectionne son parcours.
 * @param point           Point testé, en unités monde.
 * @param platforms       Plateformes du brouillon (`core::LevelDraft::platformConfigs()`).
 * @param movers          Dangers mobiles du brouillon (`core::LevelDraft::moverConfigs()`).
 * @param selected        Parcours actuellement sélectionné, si un l'est.
 * @param selectedHandles Poignées du parcours sélectionné (`pathHandleLayout`/`moverHandleLayout`).
 * @return L'élément visé, ou `std::nullopt` si le clic ne touche rien.
 */
[[nodiscard]] std::optional<PathHit> designatePathAt(
    core::Vector2 point, const std::vector<core::MovingPlatformConfig>& platforms,
    const std::vector<core::DangerMoverConfig>& movers, std::optional<PathSelection> selected,
    const std::vector<PathHandle>& selectedHandles) noexcept;

/**
 * @brief Amorce un geste sur l'élément désigné par @p hit (`designatePathAt`).
 *
 * Sélectionne immédiatement le parcours, avant même de savoir si un glisser suivra — un simple
 * clic doit déjà sélectionner et alimenter le panneau de propriétés.
 * @param state         État du geste, muté en place.
 * @param hit           Élément désigné.
 * @param worldPosition Position du clic, en unités monde.
 * @param moverStart    Position de départ du danger mobile visé, si @p hit en désigne un.
 */
void beginPathGesture(PathGestureState& state, const PathHit& hit, core::Vector2 worldPosition,
                      core::GridPosition moverStart) noexcept;

/**
 * @brief Fait progresser un glisser en cours.
 *
 * Sous `PATH_DRAG_THRESHOLD`, reste `Pressed` et renvoie `PathGestureActionKind::None` — un clic
 * sans mouvement ne déplace donc jamais un point par inadvertance. Au-delà, passe `Dragging` et
 * renvoie l'action d'**aperçu** courante ; cette fonction ne mute jamais le brouillon.
 * @param state         État du geste, muté en place.
 * @param worldPosition Position courante du curseur, en unités monde.
 * @return L'action d'aperçu courante.
 */
[[nodiscard]] PathGestureAction updatePathGesture(PathGestureState& state,
                                                  core::Vector2 worldPosition) noexcept;

/**
 * @brief Termine le geste (relâchement) et renvoie l'action **finale** à appliquer.
 *
 * `PathGestureActionKind::None` si le glisser n'a jamais dépassé le seuil (simple clic, déjà
 * traduit en sélection). Remet la phase à `Idle` ; la sélection persiste.
 * @param state         État du geste, muté en place.
 * @param worldPosition Position du relâchement, en unités monde.
 * @return L'action à appliquer aux mutateurs de `core::LevelDraft`.
 */
[[nodiscard]] PathGestureAction endPathGesture(PathGestureState& state,
                                               core::Vector2 worldPosition) noexcept;

/**
 * @brief Abandonne le geste en cours (`Échap`).
 *
 * Aucune action à appliquer : le brouillon n'a jamais été touché pendant le glisser (seul l'aperçu
 * l'était, côté rendu). Remet la phase à `Idle` ; la sélection persiste.
 * @param state État du geste, muté en place.
 */
void cancelPathGesture(PathGestureState& state) noexcept;

}  // namespace hmi
