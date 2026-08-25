// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Replay/LevelFingerprint.h"

namespace aisolver {

namespace {

constexpr std::uint64_t FNV1A_OFFSET_BASIS = 0xcbf29ce484222325ULL;
constexpr std::uint64_t FNV1A_PRIME = 0x100000001b3ULL;

}  // namespace

LevelFingerprint computeLevelFingerprint(std::string_view levelFileContent) noexcept {
    std::uint64_t hash = FNV1A_OFFSET_BASIS;
    for (const char byte : levelFileContent) {
        hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(byte));
        hash *= FNV1A_PRIME;
    }
    return hash;
}

}  // namespace aisolver
