#include "HMI/Editor/LinkGeometry.h"

#include <cmath>

#include "Core/Levels/LevelDraft.h"

namespace hmi {

namespace {

// Écart entre segments frères (même déclencheur), unités monde (fraction de case).
constexpr float FAN_SPACING = 0.12f;

// Longueur de la pointe de flèche et demi-angle d'ouverture des ailes, unités monde / radians.
constexpr float ARROW_HEAD_LENGTH = 0.18f;
constexpr float ARROW_HEAD_ANGLE = 0.45f;  // ~26 degres

// Centre (unites monde) d'une case de grille.
core::Vector2 cellCenter(core::GridPosition cell) noexcept {
    return core::Vector2{static_cast<float>(cell.column) + 0.5f,
                         static_cast<float>(cell.row) + 0.5f};
}

// Fait tourner un vecteur de `angle` radians (repere Y vers le bas, sens trigonometrique standard
// sur les composantes : suffisant ici, seule la symetrie gauche/droite des ailes compte).
core::Vector2 rotate(core::Vector2 vector, float angle) noexcept {
    const float cosAngle = std::cos(angle);
    const float sinAngle = std::sin(angle);
    return core::Vector2{vector.x * cosAngle - vector.y * sinAngle,
                         vector.x * sinAngle + vector.y * cosAngle};
}

}  // namespace

LinkSegment linkSegment(core::GridPosition trigger, core::GridPosition target, int fanIndex,
                        int fanCount) noexcept {
    const core::Vector2 a = cellCenter(trigger);
    const core::Vector2 b = cellCenter(target);
    if (fanCount <= 1) {
        return LinkSegment{a, b};
    }

    const core::Vector2 direction = b - a;
    if (direction.lengthSquared() < 1e-6f) {
        return LinkSegment{a,
                           b};  // declencheur et cible sur la meme case : pas de decalage possible.
    }
    const core::Vector2 unit = direction.normalized();
    const core::Vector2 perpendicular{-unit.y, unit.x};
    const float offset =
        (static_cast<float>(fanIndex) - static_cast<float>(fanCount - 1) * 0.5f) * FAN_SPACING;
    const core::Vector2 shift = perpendicular * offset;
    // Base commune (a, non decalee) : effet d'eventail depuis le declencheur, pas un empilement
    // de traits paralleles.
    return LinkSegment{a, b + shift};
}

ArrowHead arrowHead(core::Vector2 from, core::Vector2 to) noexcept {
    const core::Vector2 direction = to - from;
    if (direction.lengthSquared() < 1e-6f) {
        return ArrowHead{to, to};  // segment degenere : pointe sans direction definie.
    }
    const core::Vector2 back = (-direction.normalized()) * ARROW_HEAD_LENGTH;
    return ArrowHead{to + rotate(back, ARROW_HEAD_ANGLE), to + rotate(back, -ARROW_HEAD_ANGLE)};
}

std::vector<LinkRow> buildLinkRows(const core::LevelDraft& draft) {
    std::vector<LinkRow> rows;
    rows.reserve(draft.mechanisms().size() + draft.dangerLinks().size());
    for (const core::Mechanism& mechanism : draft.mechanisms()) {
        rows.push_back(
            LinkRow{LinkKind::Mechanism, mechanism.switchPosition, mechanism.doorPosition});
    }
    for (const core::DangerLink& dangerLink : draft.dangerLinks()) {
        rows.push_back(
            LinkRow{LinkKind::DangerLink, dangerLink.triggerPosition, dangerLink.dangerPosition});
    }
    return rows;
}

}  // namespace hmi
