// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/MechanismStateEncoder.h"

#include <cstddef>
#include <vector>

#include "Core/Physics/Aabb.h"

namespace aisolver {

namespace {

constexpr std::size_t DOOR_CHANNEL = 0;
constexpr std::size_t DANGER_CHANNEL = 1;
constexpr std::size_t PLATFORM_CHANNEL = 2;

// Marque une case unique (danger temporise/commute, position fixe dans la grille) sur @p channel,
// si elle tombe dans la fenetre.
void markCell(Tensor<float>& tensor, std::size_t channel, core::GridPosition position,
              core::GridPosition center, int radius) {
    const int dc = position.column - center.column;
    const int dr = position.row - center.row;
    if (dc < -radius || dc > radius || dr < -radius || dr > radius) {
        return;  // hors fenetre
    }
    tensor.at({channel, static_cast<std::size_t>(dr + radius),
               static_cast<std::size_t>(dc + radius)}) = 1.0f;
}

// Marque, sur @p channel, toute case de la fenetre recouverte par une boite continue (danger
// mobile, plateforme mobile) : la position de depart dans TileMap ne represente jamais la boite
// reelle (EX-GP-051, EX-GP-026), qui se deplace en continu -- seul un test de recouvrement
// boite-case la localise.
void rasterizeBox(Tensor<float>& tensor, std::size_t channel, const core::Aabb& box,
                  core::GridPosition center, int radius) {
    for (int dr = -radius; dr <= radius; ++dr) {
        for (int dc = -radius; dc <= radius; ++dc) {
            const auto column = static_cast<float>(center.column + dc);
            const auto row = static_cast<float>(center.row + dr);
            const bool overlaps = box.min.x < column + 1.0f && box.max.x > column &&
                                  box.min.y < row + 1.0f && box.max.y > row;
            if (!overlaps) {
                continue;
            }
            tensor.at({channel, static_cast<std::size_t>(dr + radius),
                       static_cast<std::size_t>(dc + radius)}) = 1.0f;
        }
    }
}

}  // namespace

Tensor<float> MechanismStateEncoder::encode(const core::MechanismController& mechanisms,
                                            const core::DangerController& dangers,
                                            const core::PlatformController& platforms,
                                            const core::Level& level, core::GridPosition center,
                                            int radius) const {
    // Cote de la fenetre carree centree sur `center` : calcule en int, puis converti -- convertir
    // avant l'addition elargirait le calcul (`bugprone-misplaced-widening-cast`).
    const int windowSideInTiles = 2 * radius + 1;
    const auto windowSide = static_cast<std::size_t>(windowSideInTiles);
    Tensor<float> result({static_cast<std::size_t>(CHANNEL_COUNT), windowSide, windowSide});

    // Canal 0 : porte ouverte, une case par mecanisme (mechanism.doorPosition).
    const std::vector<core::Mechanism>& mechanismList = mechanisms.mechanisms();
    for (std::size_t index = 0; index < mechanismList.size(); ++index) {
        if (!mechanisms.isDoorOpen(index)) {
            continue;
        }
        const core::GridPosition doorPosition = mechanismList[index].doorPosition;
        const int dc = doorPosition.column - center.column;
        const int dr = doorPosition.row - center.row;
        if (dc < -radius || dc > radius || dr < -radius || dr > radius) {
            continue;
        }
        result.at({DOOR_CHANNEL, static_cast<std::size_t>(dr + radius),
                   static_cast<std::size_t>(dc + radius)}) = 1.0f;
    }

    // Canal 1 : danger actif -- mobile (rasterise sur sa boite courante), temporise, commute.
    for (std::size_t index = 0; index < dangers.moverCount(); ++index) {
        rasterizeBox(result, DANGER_CHANNEL, dangers.moverBox(index), center, radius);
    }
    for (const core::DangerBlinkConfig& config : level.blinkConfigs()) {
        if (dangers.isBlinkActive(config.position)) {
            markCell(result, DANGER_CHANNEL, config.position, center, radius);
        }
    }
    for (const core::DangerLink& link : level.dangerLinks()) {
        if (mechanisms.isDangerActive(link.dangerPosition)) {
            markCell(result, DANGER_CHANNEL, link.dangerPosition, center, radius);
        }
    }

    // Canal 2 : plateforme mobile, a sa position continue courante.
    for (std::size_t index = 0; index < platforms.count(); ++index) {
        rasterizeBox(result, PLATFORM_CHANNEL, platforms.boxAt(index), center, radius);
    }

    return result;
}

}  // namespace aisolver
