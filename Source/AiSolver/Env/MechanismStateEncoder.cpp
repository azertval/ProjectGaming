// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/MechanismStateEncoder.h"

#include <cstddef>
#include <vector>

#include "Core/Physics/Aabb.h"

namespace aisolver {

namespace {

constexpr std::size_t kDangerChannel = 1;

// Marque une case unique (danger temporise/commute, position fixe dans la grille) sur le canal
// danger, si elle tombe dans la fenetre.
void markCell(Tensor<float>& tensor, core::GridPosition position, core::GridPosition center,
              int radius) {
    const int dc = position.column - center.column;
    const int dr = position.row - center.row;
    if (dc < -radius || dc > radius || dr < -radius || dr > radius) {
        return;  // hors fenetre
    }
    tensor.at({kDangerChannel, static_cast<std::size_t>(dr + radius),
               static_cast<std::size_t>(dc + radius)}) = 1.0f;
}

// Marque, sur le canal danger, toute case de la fenetre recouverte par une boite continue (danger
// mobile) : la position de depart dans TileMap ne represente jamais sa boite reellement mortelle
// (EX-GP-051), qui se deplace en continu -- seul un test de recouvrement boite-case la localise.
void rasterizeBox(Tensor<float>& tensor, const core::Aabb& box, core::GridPosition center,
                  int radius) {
    for (int dr = -radius; dr <= radius; ++dr) {
        for (int dc = -radius; dc <= radius; ++dc) {
            const auto column = static_cast<float>(center.column + dc);
            const auto row = static_cast<float>(center.row + dr);
            const bool overlaps = box.min.x < column + 1.0f && box.max.x > column &&
                                  box.min.y < row + 1.0f && box.max.y > row;
            if (!overlaps) {
                continue;
            }
            tensor.at({kDangerChannel, static_cast<std::size_t>(dr + radius),
                       static_cast<std::size_t>(dc + radius)}) = 1.0f;
        }
    }
}

}  // namespace

Tensor<float> MechanismStateEncoder::encode(const core::MechanismController& mechanisms,
                                            const core::DangerController& dangers,
                                            const core::Level& level, core::GridPosition center,
                                            int radius) const {
    // Cote de la fenetre carree centree sur `center` : calcule en int, puis converti -- convertir
    // avant l'addition elargirait le calcul (`bugprone-misplaced-widening-cast`).
    const int windowSideInTiles = 2 * radius + 1;
    const auto windowSide = static_cast<std::size_t>(windowSideInTiles);
    Tensor<float> result({static_cast<std::size_t>(kChannelCount), windowSide, windowSide});

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
        result.at({0, static_cast<std::size_t>(dr + radius),
                   static_cast<std::size_t>(dc + radius)}) = 1.0f;
    }

    // Canal 1 : danger actif -- mobile (rasterise sur sa boite courante), temporise, commute.
    for (std::size_t index = 0; index < dangers.moverCount(); ++index) {
        rasterizeBox(result, dangers.moverBox(index), center, radius);
    }
    for (const core::DangerBlinkConfig& config : level.blinkConfigs()) {
        if (dangers.isBlinkActive(config.position)) {
            markCell(result, config.position, center, radius);
        }
    }
    for (const core::DangerLink& link : level.dangerLinks()) {
        if (mechanisms.isDangerActive(link.dangerPosition)) {
            markCell(result, link.dangerPosition, center, radius);
        }
    }

    return result;
}

}  // namespace aisolver
