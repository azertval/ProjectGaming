#include "HMI/Game/LevelRunStats.h"

#include <array>
#include <cstdio>

namespace hmi {

void accumulateStep(LevelRunStats& stats, const std::vector<GameEvent>& stepEvents) {
    ++stats.simulationSteps;
    for (const GameEvent event : stepEvents) {
        switch (event) {
            case GameEvent::Jumped:
                ++stats.jumps;
                break;
            case GameEvent::Died:
                ++stats.deaths;
                break;
            default:
                break;  // les autres evenements ne comptent pas dans le bilan.
        }
    }
}

float elapsedSeconds(const LevelRunStats& stats, float fixedDeltaSeconds) noexcept {
    if (fixedDeltaSeconds <= 0.0f || stats.simulationSteps <= 0) {
        return 0.0f;
    }
    return static_cast<float>(stats.simulationSteps) * fixedDeltaSeconds;
}

std::string formatElapsed(float seconds) {
    const int total = seconds <= 0.0f ? 0 : static_cast<int>(seconds);  // troncature, pas arrondi.
    const int hours = total / 3600;
    const int minutes = (total % 3600) / 60;
    const int remaining = total % 60;

    std::array<char, 32> buffer{};
    if (hours > 0) {
        std::snprintf(buffer.data(), buffer.size(), "%d:%02d:%02d", hours, minutes, remaining);
    } else {
        std::snprintf(buffer.data(), buffer.size(), "%d:%02d", minutes, remaining);
    }
    return std::string(buffer.data());
}

}  // namespace hmi
