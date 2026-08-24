// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Cli/TrainingConfig.h"

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

namespace aisolver::cli {

namespace {

void applyJsonFile(const std::filesystem::path& configFile, TrainingConfig& config) {
    std::ifstream file(configFile);
    if (!file) {
        return;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    const std::string text = buffer.str();
    if (!nlohmann::json::accept(text)) {
        return;
    }
    const nlohmann::json root = nlohmann::json::parse(text, nullptr, false);
    if (!root.is_object()) {
        return;
    }

    if (root.contains("populationSize")) {
        config.evolutionary.populationSize = root["populationSize"].get<std::size_t>();
    }
    if (root.contains("tournamentSize")) {
        config.evolutionary.tournamentSize = root["tournamentSize"].get<int>();
    }
    if (root.contains("mutationRate")) {
        config.evolutionary.mutationRate = root["mutationRate"].get<float>();
    }
    if (root.contains("mutationStrength")) {
        config.evolutionary.mutationStrength = root["mutationStrength"].get<float>();
    }
    if (root.contains("requiredConsecutiveSuccesses")) {
        config.stopping.requiredConsecutiveSuccesses =
            root["requiredConsecutiveSuccesses"].get<int>();
    }
    if (root.contains("maxGenerations")) {
        config.stopping.maxGenerations = root["maxGenerations"].get<int>();
    }
    if (root.contains("hiddenSize")) {
        config.hiddenSize = root["hiddenSize"].get<std::size_t>();
    }
    if (root.contains("gamma")) {
        config.gamma = root["gamma"].get<float>();
    }
    if (root.contains("learningRate")) {
        config.learningRate = root["learningRate"].get<float>();
    }
    if (root.contains("optimizer")) {
        config.optimizer = root["optimizer"].get<std::string>();
    }
    if (root.contains("episodes")) {
        config.episodes = root["episodes"].get<std::size_t>();
    }
    if (root.contains("dqnReplayCapacity")) {
        config.dqnReplayCapacity = root["dqnReplayCapacity"].get<std::size_t>();
    }
    if (root.contains("dqnBatchSize")) {
        config.dqnBatchSize = root["dqnBatchSize"].get<std::size_t>();
    }
    if (root.contains("dqnWarmupSize")) {
        config.dqnWarmupSize = root["dqnWarmupSize"].get<std::size_t>();
    }
    if (root.contains("dqnUpdatePeriodSteps")) {
        config.dqnUpdatePeriodSteps = root["dqnUpdatePeriodSteps"].get<std::size_t>();
    }
    if (root.contains("dqnTargetSyncPeriodSteps")) {
        config.dqnTargetSyncPeriodSteps = root["dqnTargetSyncPeriodSteps"].get<std::size_t>();
    }
    if (root.contains("dqnEpsilonStart")) {
        config.dqnEpsilonStart = root["dqnEpsilonStart"].get<float>();
    }
    if (root.contains("dqnEpsilonEnd")) {
        config.dqnEpsilonEnd = root["dqnEpsilonEnd"].get<float>();
    }
    if (root.contains("dqnEpsilonDecaySteps")) {
        config.dqnEpsilonDecaySteps = root["dqnEpsilonDecaySteps"].get<std::size_t>();
    }
}

void applyOverrides(const CommandLineOverrides& overrides, TrainingConfig& config) {
    if (overrides.populationSize.has_value()) {
        config.evolutionary.populationSize = *overrides.populationSize;
    }
    if (overrides.mutationRate.has_value()) {
        config.evolutionary.mutationRate = *overrides.mutationRate;
    }
    if (overrides.episodes.has_value()) {
        config.episodes = *overrides.episodes;
    }
    if (overrides.learningRate.has_value()) {
        config.learningRate = *overrides.learningRate;
    }
    if (overrides.gamma.has_value()) {
        config.gamma = *overrides.gamma;
    }
    if (overrides.optimizer.has_value()) {
        config.optimizer = *overrides.optimizer;
    }
}

}  // namespace

TrainingConfig loadTrainingConfig(const std::optional<std::filesystem::path>& configFile,
                                  const CommandLineOverrides& overrides) {
    TrainingConfig config;
    if (configFile.has_value()) {
        applyJsonFile(*configFile, config);
    }
    applyOverrides(overrides, config);
    return config;
}

bool writeTrainingConfigJson(const TrainingConfig& config, const std::filesystem::path& path) {
    std::error_code ignored;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ignored);
    }
    std::ofstream file(path);
    if (!file) {
        return false;
    }

    nlohmann::ordered_json root;
    root["populationSize"] = config.evolutionary.populationSize;
    root["tournamentSize"] = config.evolutionary.tournamentSize;
    root["mutationRate"] = config.evolutionary.mutationRate;
    root["mutationStrength"] = config.evolutionary.mutationStrength;
    root["requiredConsecutiveSuccesses"] = config.stopping.requiredConsecutiveSuccesses;
    root["maxGenerations"] = config.stopping.maxGenerations;
    root["hiddenSize"] = config.hiddenSize;
    root["gamma"] = config.gamma;
    root["learningRate"] = config.learningRate;
    root["optimizer"] = config.optimizer;
    root["episodes"] = config.episodes;
    root["dqnReplayCapacity"] = config.dqnReplayCapacity;
    root["dqnBatchSize"] = config.dqnBatchSize;
    root["dqnWarmupSize"] = config.dqnWarmupSize;
    root["dqnUpdatePeriodSteps"] = config.dqnUpdatePeriodSteps;
    root["dqnTargetSyncPeriodSteps"] = config.dqnTargetSyncPeriodSteps;
    root["dqnEpsilonStart"] = config.dqnEpsilonStart;
    root["dqnEpsilonEnd"] = config.dqnEpsilonEnd;
    root["dqnEpsilonDecaySteps"] = config.dqnEpsilonDecaySteps;

    file << root.dump(2);
    return file.good();
}

}  // namespace aisolver::cli
