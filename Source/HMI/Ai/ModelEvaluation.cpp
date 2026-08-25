// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Ai/ModelEvaluation.h"

#include <memory>

#include "AiSolver/Cli/TrainingConfig.h"
#include "AiSolver/Env/ObservationEncoder.h"
#include "AiSolver/Eval/ActionDecodingMode.h"
#include "AiSolver/Eval/ActorCriticTrainedPolicy.h"
#include "AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h"
#include "AiSolver/Eval/BenchmarkConfig.h"
#include "AiSolver/Eval/BenchmarkResult.h"
#include "AiSolver/Eval/BenchmarkRunner.h"
#include "AiSolver/Eval/EvolutionaryTrainedPolicy.h"
#include "AiSolver/Eval/ReinforceTrainedPolicy.h"
#include "AiSolver/Math/Rng.h"
#include "AiSolver/Nn/Network.h"
#include "AiSolver/Nn/Serialization.h"
#include "AiSolver/Training/Dqn/QNetwork.h"
#include "AiSolver/Training/Evolutionary/NetworkTopology.h"

namespace hmi {

// Meme dispatch que aisolver::cli::runEvaluate (Cli/Commands.cpp, LOT-ANNEXE-19) : adapte a un
// resultat en types simples plutot qu'un code de sortie/flux console, jamais une regle differente.
std::optional<EvaluationOutcome> evaluateModel(const QString& modelPath, const QString& levelPath,
                                               const QString& algo, int repetitions) {
    using namespace aisolver;

    const std::filesystem::path model = modelPath.toStdString();
    const std::filesystem::path level = levelPath.toStdString();
    const std::string algoStd = algo.toStdString();

    const std::size_t inputSize = ObservationEncoder().inputSize();
    // Taille relue du run qui a produit le modele, jamais supposee : voir
    // `cli::hiddenSizeForModel`.
    const std::size_t hiddenSize = cli::hiddenSizeForModel(model);
    const training::evolutionary::NetworkTopology topology =
        training::evolutionary::policyTopology(inputSize, hiddenSize);
    Rng scratchRng(0);

    eval::BenchmarkConfig config;
    config.repetitions = repetitions;
    config.decodingMode = eval::ActionDecodingMode::Argmax;

    eval::BenchmarkResult result;
    if (algoStd == "avance") {
        training::QNetwork network(inputSize, hiddenSize, scratchRng);
        if (!nn::loadWeights(network.network(), model)) {
            return std::nullopt;
        }
        eval::AdvancedAlgorithmTrainedPolicy policy(network);
        result = eval::BenchmarkRunner::run(policy, level, config);
    } else {
        std::unique_ptr<nn::Network> network =
            training::evolutionary::buildNetwork(topology, scratchRng);
        if (!nn::loadWeights(*network, model)) {
            return std::nullopt;
        }
        if (algoStd == "pg") {
            eval::ReinforceTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, level, config);
        } else if (algoStd == "ac") {
            eval::ActorCriticTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, level, config);
        } else {
            eval::EvolutionaryTrainedPolicy policy(*network);
            result = eval::BenchmarkRunner::run(policy, level, config);
        }
    }

    return EvaluationOutcome{result.successRate(), result.meanStepsOnSuccess(),
                             result.stepVariance()};
}

}  // namespace hmi
