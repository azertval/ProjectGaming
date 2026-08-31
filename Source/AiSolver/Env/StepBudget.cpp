// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Env/StepBudget.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "AiSolver/Env/GridDistanceField.h"
#include "Core/Gameplay/MechanismController.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

namespace aisolver {

int objectiveChainLength(const core::Level& level) {
    // Grille de depart : celle du controleur, portes fermees donc solides. C'est la seule source
    // de verite sur ce qu'une porte fermee bloque -- `TileMap::isSolid` seul l'ignore.
    core::MechanismController mechanisms(level);
    core::TileMap collision = mechanisms.collisionMap();

    const std::vector<core::Mechanism>& links = level.mechanisms();
    std::vector<bool> resolved(links.size(), false);

    core::GridPosition position = level.entry();
    int totalCells = 0;

    // Au plus un tronçon par mecanisme, plus le tronçon final vers la sortie.
    for (std::size_t stage = 0; stage <= links.size(); ++stage) {
        // Champ depuis la position COURANTE : la distance de grille est symetrique (BFS a quatre
        // voisins, sans ponderation), donc un unique champ donne la distance a chaque cible
        // candidate -- et permet de savoir laquelle est la plus proche, ce que le champ multi-
        // source de la recompense ne dit pas.
        const GridDistanceField fromHere(collision, position);

        const int exitDistance =
            fromHere.isReachable(level.exit()) ? fromHere.distance(level.exit()) : -1;

        int bestTriggerDistance = -1;
        std::size_t bestTrigger = 0;
        for (std::size_t index = 0; index < links.size(); ++index) {
            if (resolved[index] || !fromHere.isReachable(links[index].switchPosition)) {
                continue;
            }
            const int distance = fromHere.distance(links[index].switchPosition);
            if (bestTriggerDistance < 0 || distance < bestTriggerDistance) {
                bestTriggerDistance = distance;
                bestTrigger = index;
            }
        }

        // La sortie l'emporte des qu'elle est au moins aussi proche : plus rien a debloquer.
        if (exitDistance >= 0 && (bestTriggerDistance < 0 || exitDistance <= bestTriggerDistance)) {
            return totalCells + exitDistance;
        }
        if (bestTriggerDistance < 0) {
            return -1;  // ni la sortie ni un declencheur : la chaine est rompue.
        }

        totalCells += bestTriggerDistance;
        position = links[bestTrigger].switchPosition;

        // Un declencheur peut commander plusieurs portes (les trois portes verrouillees de
        // `demo-final.json` partagent la meme cle) : toutes s'ouvrent ensemble.
        for (std::size_t index = 0; index < links.size(); ++index) {
            if (resolved[index] || links[index].switchPosition != position) {
                continue;
            }
            resolved[index] = true;
            collision.setTile(links[index].doorPosition.column, links[index].doorPosition.row,
                              core::TileType::Empty);
        }
    }
    return -1;
}

int estimateStepBudget(const core::Level& level) {
    const int chainCells = objectiveChainLength(level);
    if (chainCells < 0) {
        return MAX_STEP_BUDGET;
    }
    const long long nominal = static_cast<long long>(chainCells) * STEPS_PER_OBJECTIVE_CELL;
    const long long withMargin = nominal * (100 + STEP_BUDGET_MARGIN_PERCENT) / 100;
    return static_cast<int>(std::clamp<long long>(withMargin, MIN_STEP_BUDGET, MAX_STEP_BUDGET));
}

int stuckThresholdForBudget(int stepBudget) {
    return (std::max)(MIN_STUCK_THRESHOLD, stepBudget / STUCK_THRESHOLD_BUDGET_DIVISOR);
}

}  // namespace aisolver
