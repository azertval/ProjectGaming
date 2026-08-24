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
    std::cerr << "Usage : aisolver-cli <train|evaluate|export-replay> [options]\n"
              << "  train --level <chemin> --algo <evo|pg|ac|avance> [--seed N] [--config "
                 "<chemin>]\n"
              << "  evaluate --model <chemin> --algo <evo|pg|ac|avance> --level <chemin> "
                 "[--repetitions N] [--report <chemin>]\n"
              << "  export-replay --model <chemin> --algo <evo|pg|ac|avance> --level <chemin> "
                 "--output <chemin> [--seed N]\n";
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
