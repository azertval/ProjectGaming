// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Evolutionary/EvolutionaryTrainer.h"

#include <cmath>
#include <utility>

#include "AiSolver/Training/Evolutionary/GeneticOperators.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training::evolutionary {

namespace {

// Copie profonde de la topologie/poids de @p source dans un individu neuf, fitness reporte tel
// quel (elitisme : le meilleur individu n'est jamais reevalue, cf. epic).
Individual cloneIndividual(const Individual& source, const NetworkTopology& topology, Rng& rng) {
    auto network = buildNetwork(topology, rng);
    const std::vector<autodiff::NodePtr> targetParameters = network->parameters();
    const std::vector<autodiff::NodePtr> sourceParameters = source.network().parameters();
    PROJECTGAMING_ASSERT(targetParameters.size() == sourceParameters.size(),
                         "cloneIndividual : topologie incompatible avec l'individu source");
    for (std::size_t index = 0; index < targetParameters.size(); ++index) {
        targetParameters[index]->value = sourceParameters[index]->value.clone();
    }
    Individual clone(std::move(network));
    clone.fitness = source.fitness;
    return clone;
}

}  // namespace

EvolutionaryTrainer::EvolutionaryTrainer(NetworkTopology topology, EvolutionaryConfig config,
                                         HeadlessLevelEnvironment& environment,
                                         std::filesystem::path levelPath, std::uint64_t seed,
                                         TrainingStatsRecorder& recorder, std::string levelName)
    : _topology(std::move(topology)),
      _config(config),
      _environment(environment),
      _levelPath(std::move(levelPath)),
      _seed(seed),
      _rng(seed),
      _recorder(recorder),
      _levelName(std::move(levelName)),
      _population(_topology, _config.populationSize, _rng) {}

void EvolutionaryTrainer::runGeneration() {
    const std::vector<FitnessEvaluation> evaluations =
        _population.evaluateAll(_environment, _levelPath);
    PROJECTGAMING_ASSERT(!evaluations.empty(), "runGeneration : population vide");

    float bestReward = evaluations.front().fitness;
    float worstReward = evaluations.front().fitness;
    float sumReward = 0.0f;
    int bestStepCount = evaluations.front().stepCount;
    EpisodeStatus bestStatus = evaluations.front().status;
    int wonCount = 0;
    for (const FitnessEvaluation& evaluation : evaluations) {
        sumReward += evaluation.fitness;
        if (evaluation.fitness > bestReward) {
            bestReward = evaluation.fitness;
            bestStepCount = evaluation.stepCount;
            bestStatus = evaluation.status;
        }
        if (evaluation.fitness < worstReward) {
            worstReward = evaluation.fitness;
        }
        if (evaluation.status == EpisodeStatus::Won) {
            ++wonCount;
        }
    }
    _lastChampionStatus = bestStatus;
    const float meanReward = sumReward / static_cast<float>(evaluations.size());
    float varianceSum = 0.0f;
    for (const FitnessEvaluation& evaluation : evaluations) {
        const float delta = evaluation.fitness - meanReward;
        varianceSum += delta * delta;
    }
    const float rewardStdDev = std::sqrt(varianceSum / static_cast<float>(evaluations.size()));

    TrainingStatsRow row;
    row.index = _generationIndex;
    row.bestReward = bestReward;
    row.meanReward = meanReward;
    row.worstReward = worstReward;
    row.rewardStdDev = rewardStdDev;
    row.bestStepCount = bestStepCount;
    row.successRate = static_cast<float>(wonCount) / static_cast<float>(evaluations.size());
    row.seed = _seed;
    row.levelName = _levelName;
    _recorder.record(row);

    const Individual& elite = evolutionary::bestIndividual(_population);
    std::vector<Individual> nextIndividuals;
    nextIndividuals.reserve(_population.size());
    nextIndividuals.push_back(cloneIndividual(elite, _topology, _rng));

    for (std::size_t index = 1; index < _population.size(); ++index) {
        const Individual& parentA = evolutionary::selectParent(_population, _config, _rng);
        const Individual& parentB = evolutionary::selectParent(_population, _config, _rng);
        Individual child = evolutionary::crossover(parentA, parentB, _topology, _rng);
        evolutionary::mutate(child, _config, _rng);
        nextIndividuals.push_back(std::move(child));
    }

    _population = Population(_topology, std::move(nextIndividuals));
    ++_generationIndex;
}

const Individual& EvolutionaryTrainer::bestIndividual() const {
    return evolutionary::bestIndividual(_population);
}

}  // namespace aisolver::training::evolutionary
