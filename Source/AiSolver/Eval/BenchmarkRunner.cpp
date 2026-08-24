// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Eval/BenchmarkRunner.h"

#include <optional>

#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/NoisyObservation.h"
#include "Core/Diagnostics/Assert.h"
#include "Core/Physics/PlayerSpawn.h"

namespace aisolver::eval {

std::uint64_t deriveSeed(std::uint64_t base, int repetitionIndex) {
    return base + static_cast<std::uint64_t>(repetitionIndex);
}

BenchmarkResult BenchmarkRunner::run(TrainedPolicy& policy, const std::filesystem::path& levelPath,
                                     const BenchmarkConfig& config) {
    return runWithNoise(policy, levelPath, config, 0.0f);
}

BenchmarkResult BenchmarkRunner::runWithNoise(TrainedPolicy& policy,
                                              const std::filesystem::path& levelPath,
                                              const BenchmarkConfig& config, float noiseAmplitude) {
    BenchmarkResult result;
    result.episodes.reserve(static_cast<std::size_t>(config.repetitions));

    const ObservationEncoder observationEncoder;
    const NoisyObservationWrapper noisyEncoder(observationEncoder, noiseAmplitude);

    for (int repetition = 0; repetition < config.repetitions; ++repetition) {
        Rng rng(deriveSeed(config.rngSeedBase, repetition));
        HeadlessLevelEnvironment environment(EnvironmentConfig{.maxSteps = config.maxStepsPerEpisode});
        const bool loaded = environment.reset(levelPath);
        PROJECTGAMING_ASSERT(loaded, "BenchmarkRunner::runWithNoise : le niveau doit se charger");

        const core::GridPosition entry = environment.level().entry();
        core::Aabb previousBox = core::Aabb::fromTopLeftSize(
            core::playerSpawnPosition(entry.column, entry.row), core::playerSize());
        core::Player playerState{};
        core::Velocity playerVelocity{};

        core::LevelOutcome outcome = core::LevelOutcome::Playing;
        int stepIndex = 0;
        while (outcome == core::LevelOutcome::Playing && !environment.budgetExhausted()) {
            const Tensor<float> observation =
                noisyEncoder.encode(environment, previousBox, playerState, playerVelocity, rng);
            const std::optional<core::PlayerInput> input =
                policy.selectAction(observation, config.decodingMode, rng);
            PROJECTGAMING_ASSERT(
                input.has_value(),
                "BenchmarkRunner::runWithNoise : mode de decodage non supporte par cette politique "
                "(verifier TrainedPolicy::supportsMode avant l'appel)");

            const StepObservation stepObservation = environment.step(*input);
            outcome = stepObservation.outcome;
            stepIndex = stepObservation.stepIndex;

            previousBox = stepObservation.playerBox;
            playerState = stepObservation.playerState;
            playerVelocity = stepObservation.playerVelocity;
        }

        result.episodes.push_back(EpisodeOutcome{outcome, stepIndex});
    }
    return result;
}

}  // namespace aisolver::eval
