// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <string>
#include <vector>

#include "AiSolver/Cli/Commands.h"

/**
 * @file AiSolver/Cli/Main.cpp
 * @brief Point d'entrée de `aisolver-cli` (`LOT-ANNEXE-19`, TACHE-01, `EX-IA-020`) : analyse la
 * sous-commande, délègue à `Commands.h`, aussi mince que possible (toute logique testable vit dans
 * `Commands.cpp`).
 */

namespace {

void printUsage() {
    std::cerr
        << "Usage : aisolver-cli <train|evaluate|export-replay> [options]\n"
        << "\n"
        << "  train --level <chemin> --algo <evo|pg|ac|avance>\n"
        << "    Commun     : [--seed N] [--config <chemin>] [--runs-root <chemin>]\n"
        << "                 [--hidden-size N] [--max-steps N] [--stuck-threshold N]\n"
        << "                 (budget et seuil derives du niveau si absents ou nuls)\n"
        << "    Evolutif   : [--population-size N] [--mutation-rate X] [--mutation-strength X]\n"
        << "                 [--tournament-size N] [--max-generations N]\n"
        << "                 [--required-successes N] [--crossover-rate X]\n"
        << "    Gradient   : [--episodes N] [--learning-rate X] [--gamma X]\n"
        << "                 [--critic-learning-rate X] (acteur-critique)\n"
        << "                 [--optimizer <sgd|adam>] [--batch-episodes N] [--entropy X]\n"
        << "                 [--grad-clip X] [--action-repeat N] [--exploration-floor X]\n"
        << "    DQN        : [--dqn-replay-capacity N] [--dqn-batch-size N]\n"
        << "                 [--dqn-warmup-size N] [--dqn-update-period N]\n"
        << "                 [--dqn-target-sync-period N] [--dqn-epsilon-start X]\n"
        << "                 [--dqn-epsilon-end X] [--dqn-epsilon-decay N]\n"
        << "\n"
        << "  evaluate --model <chemin> --algo <evo|pg|ac|avance> --level <chemin>\n"
        << "                 [--repetitions N] [--report <chemin>] [--max-steps N] [--seed N]\n"
        << "                 [--decoding <argmax|stochastic>]\n"
        << "\n"
        << "  export-replay --model <chemin> --algo <evo|pg|ac|avance> --level <chemin>\n"
        << "                 --output <chemin> [--seed N]\n"
        << "\n"
        << "Priorite des hyperparametres : defauts documentes, puis --config, puis les options\n"
        << "individuelles ci-dessus. La configuration resolue est ecrite dans le config.json du\n"
        << "run.\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    const std::string subcommand = argv[1];
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc) - 2);
    for (int index = 2; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }

    std::string error;
    if (subcommand == "train") {
        const std::optional<aisolver::cli::TrainArgs> parsed =
            aisolver::cli::parseTrainArgs(args, error);
        if (!parsed.has_value()) {
            std::cerr << error << "\n";
            return 1;
        }
        return aisolver::cli::runTrain(*parsed);
    }
    if (subcommand == "evaluate") {
        const std::optional<aisolver::cli::EvaluateArgs> parsed =
            aisolver::cli::parseEvaluateArgs(args, error);
        if (!parsed.has_value()) {
            std::cerr << error << "\n";
            return 1;
        }
        return aisolver::cli::runEvaluate(*parsed);
    }
    if (subcommand == "export-replay") {
        const std::optional<aisolver::cli::ExportReplayArgs> parsed =
            aisolver::cli::parseExportReplayArgs(args, error);
        if (!parsed.has_value()) {
            std::cerr << error << "\n";
            return 1;
        }
        return aisolver::cli::runExportReplay(*parsed);
    }

    std::cerr << "Sous-commande inconnue : " << subcommand << "\n";
    printUsage();
    return 1;
}
