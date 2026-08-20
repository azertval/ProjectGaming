// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/DecorGesture.h"

#include <algorithm>
#include <cmath>

namespace hmi {

namespace {

constexpr float TWO_PI = 6.28318530717958647692F;
constexpr float MIN_DECOR_SIZE = 0.05F;  // unites monde : evite une echelle nulle/degeneree.

// Normalise un angle dans [0, 2*pi[ (meme regle que core::LevelDraft::rotateDecor).
float normalizeRotation(float rotation) noexcept {
    float normalized = std::fmod(rotation, TWO_PI);
    if (normalized < 0.0F) {
        normalized += TWO_PI;
    }
    return normalized;
}

// Coin FIXE (oppose a la poignee saisie) autour duquel le redimensionnement pivote.
core::Vector2 resizeAnchor(DecorHandle handle, core::Vector2 position,
                           core::Vector2 size) noexcept {
    switch (handle) {
        case DecorHandle::TopLeft:
            return core::Vector2{position.x + size.x, position.y + size.y};  // bas-droit
        case DecorHandle::TopRight:
            return core::Vector2{position.x, position.y + size.y};  // bas-gauche
        case DecorHandle::BottomLeft:
            return core::Vector2{position.x + size.x, position.y};  // haut-droit
        case DecorHandle::BottomRight:
        default:
            return position;  // haut-gauche
    }
}

// Calcule l'action (Move/Resize/Rotate) correspondant a l'etat courant du geste et a la position
// donnee -- partagee par updateDecorGesture (apercu) et endDecorGesture (action finale), pour ne
// jamais laisser diverger le calcul entre les deux.
DecorGestureAction computeAction(const DecorGestureState& state, core::Vector2 worldPosition,
                                 bool snapToGrid) noexcept {
    DecorGestureAction action;
    action.index = *state.selectedIndex;
    const core::Vector2 delta = worldPosition - state.pressPosition;

    if (state.handle == DecorHandle::Body) {
        action.kind = DecorGestureActionKind::Move;
        core::Vector2 position = state.initialPosition + delta;
        if (snapToGrid) {
            position.x = std::round(position.x);
            position.y = std::round(position.y);
        }
        action.position = position;
        action.scale = state.initialScale;
        action.rotation = state.initialRotation;
        return action;
    }

    if (state.handle == DecorHandle::Rotation) {
        action.kind = DecorGestureActionKind::Rotate;
        const core::Vector2 center{state.initialPosition.x + (state.initialSize.x * 0.5F),
                                   state.initialPosition.y + (state.initialSize.y * 0.5F)};
        const core::Vector2 toCursor = worldPosition - center;
        // atan2(dx, -dy) : 0 pointe vers le haut (dx=0, dy<0), croissant dans le sens horaire --
        // convention arbitraire mais fixe, la seule exigence etant la coherence apercu/final.
        action.rotation = normalizeRotation(std::atan2(toCursor.x, -toCursor.y));
        action.position = state.initialPosition;
        action.scale = state.initialScale;
        return action;
    }

    // Coin (TopLeft/TopRight/BottomLeft/BottomRight) : redimensionnement autour du coin oppose,
    // fixe. Formuler via min/max du coin saisi et de l'ancre couvre les quatre poignees sans
    // branche par poignee : la nouvelle position est le coin haut-gauche du rectangle forme par
    // les deux points, la nouvelle taille son ecart absolu.
    action.kind = DecorGestureActionKind::Resize;
    const core::Vector2 anchor =
        resizeAnchor(state.handle, state.initialPosition, state.initialSize);
    core::Vector2 corner = worldPosition;
    if (snapToGrid) {
        corner.x = std::round(corner.x);
        corner.y = std::round(corner.y);
    }
    core::Vector2 newSize{std::fabs(corner.x - anchor.x), std::fabs(corner.y - anchor.y)};
    newSize.x = (std::max)(newSize.x, MIN_DECOR_SIZE);
    newSize.y = (std::max)(newSize.y, MIN_DECOR_SIZE);
    action.position = core::Vector2{(std::min)(corner.x, anchor.x), (std::min)(corner.y, anchor.y)};
    action.scale = core::Vector2{state.initialScale.x * newSize.x / state.initialSize.x,
                                 state.initialScale.y * newSize.y / state.initialSize.y};
    action.rotation = state.initialRotation;
    return action;
}

}  // namespace

std::optional<DecorHit> designateDecorAt(
    core::Vector2 point, const std::vector<core::Rect>& bounds,
    std::optional<std::size_t> selectedIndex,
    const std::optional<DecorHandleLayout>& selectedHandles) noexcept {
    if (selectedIndex && selectedHandles) {
        const DecorHandle handle = hitTestDecorHandles(point, *selectedHandles);
        if (handle != DecorHandle::None) {
            return DecorHit{.index = *selectedIndex, .handle = handle};
        }
    }
    // Du dernier au premier : le dernier du vecteur est le plus au-dessus de sa couche (LOT-49
    // TACHE-01), donc le plus probable vise par un clic sur une zone de recouvrement.
    for (std::size_t i = bounds.size(); i > 0; --i) {
        const std::size_t index = i - 1;
        if (bounds[index].contains(point)) {
            return DecorHit{.index = index, .handle = DecorHandle::Body};
        }
    }
    return std::nullopt;
}

void beginDecorGesture(DecorGestureState& state, DecorHit hit, core::Vector2 worldPosition,
                       const core::Decor& decor, const core::Rect& bounds) noexcept {
    state.phase = DecorGesturePhase::Pressed;
    state.selectedIndex = hit.index;
    state.handle = hit.handle;
    state.pressPosition = worldPosition;
    state.initialPosition = decor.position;
    state.initialScale = decor.scale;
    state.initialRotation = decor.rotation;
    state.initialSize = bounds.size;
}

DecorGestureAction updateDecorGesture(DecorGestureState& state, core::Vector2 worldPosition,
                                      bool snapToGrid) noexcept {
    if (state.phase == DecorGesturePhase::Idle || !state.selectedIndex) {
        return DecorGestureAction{};
    }
    if (state.phase == DecorGesturePhase::Pressed) {
        const core::Vector2 delta = worldPosition - state.pressPosition;
        if (delta.lengthSquared() < DECOR_DRAG_THRESHOLD * DECOR_DRAG_THRESHOLD) {
            return DecorGestureAction{};  // sous le seuil : pas encore un glisser.
        }
        state.phase = DecorGesturePhase::Dragging;
    }
    return computeAction(state, worldPosition, snapToGrid);
}

DecorGestureAction endDecorGesture(DecorGestureState& state, core::Vector2 worldPosition,
                                   bool snapToGrid) noexcept {
    DecorGestureAction result;
    if (state.phase != DecorGesturePhase::Idle && state.selectedIndex) {
        // Verifie le seuil depuis la position de l'appui, plutot que de se fier a state.phase ==
        // Dragging (deja bascule par un updateDecorGesture precedent) : un relachement loin de
        // l'appui doit compter comme un glisser meme sans evenement de mouvement intermediaire
        // (glisser tres rapide, ou report direct press->release dans les tests).
        const core::Vector2 delta = worldPosition - state.pressPosition;
        if (delta.lengthSquared() >= DECOR_DRAG_THRESHOLD * DECOR_DRAG_THRESHOLD) {
            result = computeAction(state, worldPosition, snapToGrid);
        }
    }
    // Sinon : simple clic, deja traduit en selection par beginDecorGesture -- rien de plus a
    // appliquer (kind reste None).
    state.phase = DecorGesturePhase::Idle;
    state.handle = DecorHandle::None;
    return result;
}

void cancelDecorGesture(DecorGestureState& state) noexcept {
    state.phase = DecorGesturePhase::Idle;
    state.handle = DecorHandle::None;
}

}  // namespace hmi
