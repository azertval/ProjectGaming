// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Cli/ArgParsing.h"

namespace aisolver::cli {

std::optional<std::string> findOption(const std::vector<std::string>& args, std::string_view name) {
    for (std::size_t index = 0; index < args.size(); ++index) {
        if (args[index] == name) {
            if (index + 1 >= args.size()) {
                return std::nullopt;
            }
            return args[index + 1];
        }
    }
    return std::nullopt;
}

bool hasFlag(const std::vector<std::string>& args, std::string_view name) {
    for (const std::string& arg : args) {
        if (arg == name) {
            return true;
        }
    }
    return false;
}

}  // namespace aisolver::cli
