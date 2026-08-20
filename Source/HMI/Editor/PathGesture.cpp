#include "HMI/Editor/PathGesture.h"

#include <cmath>

namespace hmi {

namespace {

// Case entiere contenant @p position : un point de parcours EST une case, jamais une position
// continue -- l'aimantation n'est donc pas optionnelle ici, contrairement aux decors libres.
[[nodiscard]] core::GridPosition cellAt(core::Vector2 position) {
    return core::GridPosition{.column = static_cast<int>(std::floor(position.x)),
                              .row = static_cast<int>(std::floor(position.y))};
}

// Case de depart d'un parcours, seule zone cliquable pour le selectionner : c'est la tuile posee
// dans la grille (MovingPlatform ou DangerMover).
[[nodiscard]] bool coversCell(core::GridPosition cell, core::Vector2 point) {
    return cellAt(point) == cell;
}

// Traduit le glisser d'une extremite de danger mobile en axe + portee : on retient l'axe du plus
// grand deplacement (le mouvement dominant), et une portee jamais negative -- DangerMoverConfig
// decrit une course dans le sens POSITIF de son axe (Level.h).
[[nodiscard]] PathGestureAction moverAction(const PathGestureState& state,
                                            core::Vector2 worldPosition) {
    const core::GridPosition cell = cellAt(worldPosition);
    const int deltaColumn = cell.column - state.moverStart.column;
    const int deltaRow = cell.row - state.moverStart.row;
    const bool horizontal = std::abs(deltaColumn) >= std::abs(deltaRow);
    return PathGestureAction{
        .kind = PathGestureActionKind::SetMoverRange,
        .target = *state.selected,
        .axis = horizontal ? core::DangerMoverAxis::Horizontal : core::DangerMoverAxis::Vertical,
        .range = std::max(0, horizontal ? deltaColumn : deltaRow)};
}

// Action correspondant a l'etat courant du glisser, identique en apercu et au relachement -- pour
// que la valeur appliquee soit EXACTEMENT celle du dernier apercu affiche.
[[nodiscard]] PathGestureAction dragAction(const PathGestureState& state,
                                           core::Vector2 worldPosition) {
    if (state.selected->kind == PathTargetKind::Mover) {
        return moverAction(state, worldPosition);
    }
    // Amorce et milieu de segment INSERENT tous deux un point ; seule une poignee de point
    // existant en deplace un.
    const bool inserts = state.handle.kind == PathHandleKind::Midpoint ||
                         state.handle.kind == PathHandleKind::Origin;
    return PathGestureAction{.kind = inserts ? PathGestureActionKind::InsertWaypoint
                                             : PathGestureActionKind::MoveWaypoint,
                             .target = *state.selected,
                             .waypointIndex = state.handle.index,
                             .position = cellAt(worldPosition)};
}

}  // namespace

std::optional<PathHit> designatePathAt(core::Vector2 point,
                                       const std::vector<core::MovingPlatformConfig>& platforms,
                                       const std::vector<core::DangerMoverConfig>& movers,
                                       std::optional<PathSelection> selected,
                                       const std::vector<PathHandle>& selectedHandles) noexcept {
    // 1. Poignees du parcours deja selectionne : elles priment sur tout le reste, y compris sur la
    //    case de depart d'un autre parcours qui passerait dessous.
    if (selected) {
        if (const std::optional<PathHandle> handle = hitTestPathHandles(point, selectedHandles)) {
            return PathHit{.target = *selected, .handle = *handle};
        }
    }

    // 2. Case de depart d'un parcours : la tuile elle-meme sert de zone de selection.
    for (std::size_t index = 0; index < platforms.size(); ++index) {
        if (coversCell(platforms[index].startPosition, point)) {
            return PathHit{
                .target = PathSelection{.kind = PathTargetKind::Platform, .index = index},
                .handle = PathHandle{}};
        }
    }
    for (std::size_t index = 0; index < movers.size(); ++index) {
        if (coversCell(movers[index].startPosition, point)) {
            return PathHit{.target = PathSelection{.kind = PathTargetKind::Mover, .index = index},
                           .handle = PathHandle{}};
        }
    }
    return std::nullopt;
}

void beginPathGesture(PathGestureState& state, const PathHit& hit, core::Vector2 worldPosition,
                      core::GridPosition moverStart) noexcept {
    state.selected = hit.target;
    state.handle = hit.handle;
    state.pressPosition = worldPosition;
    state.moverStart = moverStart;
    // Un clic sur la case de depart (aucune poignee visee) ne fait que selectionner : rester Idle
    // evite qu'un glisser depuis la tuile ne deplace un point au hasard. L'amorce d'une route vide
    // (LOT-68) est en revanche une VRAIE poignee : la glisser cree le premier point.
    state.phase = hit.handle.kind == PathHandleKind::None ? PathGesturePhase::Idle
                                                          : PathGesturePhase::Pressed;
}

PathGestureAction updatePathGesture(PathGestureState& state, core::Vector2 worldPosition) noexcept {
    if (state.phase == PathGesturePhase::Idle || !state.selected) {
        return PathGestureAction{};
    }
    const core::Vector2 delta = worldPosition - state.pressPosition;
    if (state.phase == PathGesturePhase::Pressed && delta.length() < PATH_DRAG_THRESHOLD) {
        return PathGestureAction{};  // encore un simple clic
    }
    state.phase = PathGesturePhase::Dragging;
    return dragAction(state, worldPosition);
}

PathGestureAction endPathGesture(PathGestureState& state, core::Vector2 worldPosition) noexcept {
    // Le seuil est reevalue ICI, pas seulement dans updatePathGesture : un glisser assez rapide
    // peut n'avoir produit aucun evenement de deplacement intermediaire, et le geste doit quand
    // meme aboutir. La condition est la meme des deux cotes, donc l'action finale coincide
    // toujours avec le dernier apercu affiche.
    const bool dragged = state.phase != PathGesturePhase::Idle && state.selected &&
                         (worldPosition - state.pressPosition).length() >= PATH_DRAG_THRESHOLD;
    state.phase = PathGesturePhase::Idle;
    if (!dragged) {
        return PathGestureAction{};
    }
    const PathGestureAction action = dragAction(state, worldPosition);
    // Un point insere prend le rang du segment coupe : le geste suivant doit le voir comme un point
    // EXISTANT, sans quoi un second glisser en inserait un troisieme.
    if (action.kind == PathGestureActionKind::InsertWaypoint) {
        state.handle.kind = PathHandleKind::Waypoint;
    }
    return action;
}

void cancelPathGesture(PathGestureState& state) noexcept {
    state.phase = PathGesturePhase::Idle;
}

}  // namespace hmi
