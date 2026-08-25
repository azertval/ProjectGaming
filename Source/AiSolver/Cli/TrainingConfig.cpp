// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Cli/TrainingConfig.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

#include <nlohmann/json.hpp>

namespace aisolver::cli {

namespace {

/// Affecte @p target depuis la cle @p key de @p root, si elle est presente **et** du bon type.
///
/// Une valeur presente mais du mauvais type (`{"gamma": "abc"}`) est ignoree : le defaut documente
/// reste en place. `get<T>()` seul leverait un `nlohmann::json::type_error` -- aucune exception ne
/// doit franchir cette frontiere (`EX-NFR-040`), et cette fonction n'a pas de canal d'erreur, la
/// configuration resolue etant toujours complete par construction. Meme convention que
/// `Replay/ReplayFile.cpp`.
template <typename T>
void readNumber(const nlohmann::json& root, const char* key, T& target) {
    if (root.contains(key) && root[key].is_number()) {
        target = root[key].get<T>();
    }
}

/// @copydoc readNumber
void readString(const nlohmann::json& root, const char* key, std::string& target) {
    if (root.contains(key) && root[key].is_string()) {
        target = root[key].get<std::string>();
    }
}

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

    readString(root, "algo", config.algorithmId);
    readNumber(root, "populationSize", config.evolutionary.populationSize);
    readNumber(root, "tournamentSize", config.evolutionary.tournamentSize);
    readNumber(root, "mutationRate", config.evolutionary.mutationRate);
    readNumber(root, "mutationStrength", config.evolutionary.mutationStrength);
    readNumber(root, "requiredConsecutiveSuccesses", config.stopping.requiredConsecutiveSuccesses);
    readNumber(root, "maxGenerations", config.stopping.maxGenerations);
    readNumber(root, "hiddenSize", config.hiddenSize);
    readNumber(root, "gamma", config.gamma);
    readNumber(root, "learningRate", config.learningRate);
    readString(root, "optimizer", config.optimizer);
    readNumber(root, "episodes", config.episodes);
    readNumber(root, "dqnReplayCapacity", config.dqnReplayCapacity);
    readNumber(root, "dqnBatchSize", config.dqnBatchSize);
    readNumber(root, "dqnWarmupSize", config.dqnWarmupSize);
    readNumber(root, "dqnUpdatePeriodSteps", config.dqnUpdatePeriodSteps);
    readNumber(root, "dqnTargetSyncPeriodSteps", config.dqnTargetSyncPeriodSteps);
    readNumber(root, "dqnEpsilonStart", config.dqnEpsilonStart);
    readNumber(root, "dqnEpsilonEnd", config.dqnEpsilonEnd);
    readNumber(root, "dqnEpsilonDecaySteps", config.dqnEpsilonDecaySteps);
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
    root["algo"] = config.algorithmId;
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

std::size_t hiddenSizeForModel(const std::filesystem::path& modelPath) {
    std::error_code error;
    const std::filesystem::path configPath = modelPath.parent_path() / "config.json";
    if (!std::filesystem::exists(configPath, error) || error) {
        return training::evolutionary::DEFAULT_HIDDEN_SIZE;
    }
    return loadTrainingConfig(configPath, CommandLineOverrides{}).hiddenSize;
}

}  // namespace aisolver::cli
