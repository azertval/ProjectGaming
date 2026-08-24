// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Evolutionary/Population.h"

#include <utility>

namespace aisolver::training::evolutionary {

Population::Population(NetworkTopology topology, std::size_t size, Rng& rng)
    : _topology(std::move(topology)) {
    _individuals.reserve(size);
    for (std::size_t index = 0; index < size; ++index) {
        _individuals.emplace_back(buildNetwork(_topology, rng));
    }
}

Population::Population(NetworkTopology topology, std::vector<Individual> individuals)
    : _topology(std::move(topology)), _individuals(std::move(individuals)) {}

std::vector<FitnessEvaluation> Population::evaluateAll(HeadlessLevelEnvironment& environment,
                                                       const std::filesystem::path& levelPath) {
    std::vector<FitnessEvaluation> evaluations;
    evaluations.reserve(_individuals.size());
    for (Individual& individual : _individuals) {
        evaluations.push_back(evaluateFitness(individual, environment, levelPath));
    }
    return evaluations;
}

}  // namespace aisolver::training::evolutionary
