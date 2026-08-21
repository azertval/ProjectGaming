// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/DangerController.h"

#include <cmath>

namespace core {

namespace {

// Vitesse de conception d'un danger mobile (EX-GP-051), en cases par seconde — pas encore
// configurable par tuile (cf. exclusion de l'epic LOT-31 : pas d'edition numerique en jeu).
constexpr float MOVER_SPEED_CELLS_PER_SECOND = 2.0F;

// Duree d'un pas fixe : le jeu tourne a 60 pas/s (core::FixedTimestep), meme hypothese que les
// periodes/dephasages de DangerBlinkConfig, exprimes en pas fixes.
constexpr float FIXED_DELTA_SECONDS = 1.0F / 60.0F;

}  // namespace

DangerController::DangerController(const Level& level)
    : _moverConfigs(level.moverConfigs()), _blinkConfigs(level.blinkConfigs()) {}

void DangerController::update() noexcept {
    ++_stepCount;
}

Aabb DangerController::moverBox(std::size_t index) const noexcept {
    const DangerMoverConfig& config = _moverConfigs[index];
    float offset = 0.0F;
    if (config.range > 0) {
        // Aller-retour triangulaire deterministe : 0 -> range -> 0, period = 2 * range cases.
        const float distancePerStep = MOVER_SPEED_CELLS_PER_SECOND * FIXED_DELTA_SECONDS;
        const float totalDistance = static_cast<float>(_stepCount) * distancePerStep;
        const float cycleLength = static_cast<float>(config.range) * 2.0F;
        float phase = std::fmod(totalDistance, cycleLength);
        if (phase < 0.0F) {
            phase += cycleLength;
        }
        offset = phase <= static_cast<float>(config.range) ? phase : cycleLength - phase;
    }

    const float left = static_cast<float>(config.startPosition.column) +
                       (config.axis == DangerMoverAxis::Horizontal ? offset : 0.0F);
    const float top = static_cast<float>(config.startPosition.row) +
                      (config.axis == DangerMoverAxis::Vertical ? offset : 0.0F);
    return Aabb{.min = Vector2{left, top}, .max = Vector2{left + 1.0F, top + 1.0F}};
}

bool DangerController::isBlinkActive(GridPosition position) const noexcept {
    for (const DangerBlinkConfig& config : _blinkConfigs) {
        if (config.position != position) {
            continue;
        }
        if (config.period <= 0) {
            return false;  // configuration degeneree (robustesse) : jamais mortel
        }
        const long long cursor =
            (((_stepCount - config.phase) % config.period) + config.period) % config.period;
        return cursor < static_cast<long long>(config.activeDuration);
    }
    return false;
}

}  // namespace core
