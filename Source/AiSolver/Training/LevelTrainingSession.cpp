// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/LevelTrainingSession.h"

#include <limits>
#include <utility>

#include "AiSolver/Math/Autodiff/Node.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"
#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

namespace {

// Copie profonde d'un individu vers une instance independante (meme patron que le clonage de
// l'elite dans EvolutionaryTrainer.cpp) : Individual n'est pas copiable (unique_ptr<Network>), et
// TrainingResult doit posseder son propre exemplaire, independant de la population interne du
// trainer (detruite avec la session). Le Rng local ne sert qu'a materialiser la forme du reseau
// neuf ; ses poids sont immediatement ecrases par ceux de la source, aucun impact sur le
// determinisme de l'entrainement.
evolutionary::Individual cloneIndividual(const evolutionary::Individual& source,
                                         const evolutionary::NetworkTopology& topology) {
    Rng scratchRng(0);
    auto network = evolutionary::buildNetwork(topology, scratchRng);
    const std::vector<autodiff::NodePtr> targetParameters = network->parameters();
    const std::vector<autodiff::NodePtr> sourceParameters = source.network().parameters();
    PROJECTGAMING_ASSERT(targetParameters.size() == sourceParameters.size(),
                         "cloneIndividual : topologie incompatible avec l'individu source");
    for (std::size_t index = 0; index < targetParameters.size(); ++index) {
        targetParameters[index]->value = sourceParameters[index]->value.clone();
    }
    evolutionary::Individual clone(std::move(network));
    clone.fitness = source.fitness;
    return clone;
}

}  // namespace

int updateConsecutiveStableWins(int previousCount, bool sameChampionAsBefore,
                                bool resolvingNow) noexcept {
    if (sameChampionAsBefore && resolvingNow) {
        return previousCount + 1;
    }
    return resolvingNow ? 1 : 0;
}

LevelTrainingSession::LevelTrainingSession(std::filesystem::path levelPath,
                                           evolutionary::NetworkTopology topology,
                                           evolutionary::EvolutionaryConfig config,
                                           StoppingConfig stopping, std::uint64_t seed,
                                           const std::filesystem::path& statsCsvPath,
                                           EnvironmentConfig environmentConfig)
    : _levelPath(std::move(levelPath)),
      _topology(topology),
      _stopping(stopping),
      _environment(environmentConfig),
      _recorder(statsCsvPath),
      _trainer(topology, config, _environment, _levelPath, seed, _recorder,
               _levelPath.stem().string()) {}

TrainingResult LevelTrainingSession::run(
    const std::function<bool()>& shouldStop,
    const std::function<void(const evolutionary::Individual&)>& onGenerationChampion) {
    float previousChampionFitness = -std::numeric_limits<float>::infinity();
    int consecutiveStableWins = 0;

    for (int generation = 0; generation < _stopping.maxGenerations; ++generation) {
        if (shouldStop && shouldStop()) {
            return TrainingResult{false, static_cast<unsigned>(generation),
                                  cloneIndividual(_trainer.bestIndividual(), _topology)};
        }
        _trainer.runGeneration();

        const evolutionary::Individual& champion = _trainer.bestIndividual();
        if (onGenerationChampion) {
            onGenerationChampion(champion);
        }
        const bool sameChampionAsBefore = champion.fitness == previousChampionFitness;
        const bool resolvingNow = _trainer.lastChampionStatus() == EpisodeStatus::Won;

        consecutiveStableWins =
            updateConsecutiveStableWins(consecutiveStableWins, sameChampionAsBefore, resolvingNow);
        previousChampionFitness = champion.fitness;

        if (consecutiveStableWins >= _stopping.requiredConsecutiveSuccesses) {
            return TrainingResult{true, static_cast<unsigned>(generation) + 1,
                                  cloneIndividual(champion, _topology)};
        }
    }

    return TrainingResult{false, static_cast<unsigned>(_stopping.maxGenerations),
                          cloneIndividual(_trainer.bestIndividual(), _topology)};
}

}  // namespace aisolver::training
