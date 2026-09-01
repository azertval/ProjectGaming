// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Evolutionary/GeneticOperators.h"

#include "AiSolver/Math/TensorOps.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training::evolutionary {

const Individual& selectParent(const Population& population, const EvolutionaryConfig& config,
                               Rng& rng) {
    PROJECTGAMING_ASSERT(population.size() > 0, "selectParent : population vide");
    const int lastIndex = static_cast<int>(population.size()) - 1;

    std::size_t bestIndex = static_cast<std::size_t>(rng.nextInt(0, lastIndex));
    float bestFitness = population.individual(bestIndex).fitness;
    for (int draw = 1; draw < config.tournamentSize; ++draw) {
        const std::size_t candidateIndex = static_cast<std::size_t>(rng.nextInt(0, lastIndex));
        const float candidateFitness = population.individual(candidateIndex).fitness;
        if (candidateFitness > bestFitness) {
            bestFitness = candidateFitness;
            bestIndex = candidateIndex;
        }
    }
    return population.individual(bestIndex);
}

Individual crossover(const Individual& parentA, const Individual& parentB,
                     const NetworkTopology& topology, const EvolutionaryConfig& config, Rng& rng) {
    auto childNetwork = buildNetwork(topology, rng);

    const std::vector<autodiff::NodePtr> childParameters = childNetwork->parameters();
    const std::vector<autodiff::NodePtr> parametersA = parentA.network().parameters();
    const std::vector<autodiff::NodePtr> parametersB = parentB.network().parameters();
    PROJECTGAMING_ASSERT(
        childParameters.size() == parametersA.size() && parametersA.size() == parametersB.size(),
        "crossover : parents et enfant doivent partager la meme topologie");

    // Tirage AVANT la boucle, jamais par poids : sans croisement, l'enfant est une copie fidele de
    // parentA, ce qu'un tirage par poids ne produirait qu'avec une probabilite negligeable.
    const bool combine = rng.nextFloat() < config.crossoverRate;

    for (std::size_t index = 0; index < childParameters.size(); ++index) {
        Tensor<float>& childValue = childParameters[index]->value;
        const Tensor<float>& valueA = parametersA[index]->value;
        const Tensor<float>& valueB = parametersB[index]->value;
        childValue = valueA.clone();
        if (!combine) {
            continue;
        }
        float* weights = childValue.data();
        for (std::size_t weightIndex = 0; weightIndex < childValue.size(); ++weightIndex) {
            if (rng.nextFloat() < 0.5f) {
                weights[weightIndex] = valueB.data()[weightIndex];
            }
        }
    }
    return Individual(std::move(childNetwork));
}

void mutate(Individual& individual, const EvolutionaryConfig& config, Rng& rng) {
    for (const autodiff::NodePtr& parameter : individual.network().parameters()) {
        float* weights = parameter->value.data();
        for (std::size_t index = 0; index < parameter->value.size(); ++index) {
            if (rng.nextFloat() < config.mutationRate) {
                weights[index] += rng.nextGaussian(0.0f, config.mutationStrength);
            }
        }
    }
}

const Individual& bestIndividual(const Population& population) {
    PROJECTGAMING_ASSERT(population.size() > 0, "bestIndividual : population vide");
    std::size_t bestIndex = 0;
    float bestFitness = population.individual(0).fitness;
    for (std::size_t index = 1; index < population.size(); ++index) {
        const float fitness = population.individual(index).fitness;
        if (fitness > bestFitness) {
            bestFitness = fitness;
            bestIndex = index;
        }
    }
    return population.individual(bestIndex);
}

}  // namespace aisolver::training::evolutionary
