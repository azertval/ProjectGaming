#include "HMI/Editor/DecorPlacementGesture.h"

namespace hmi {

std::optional<std::size_t> nearestDecorAt(core::Vector2 worldPosition,
                                          const std::vector<core::Decor>& decors) noexcept {
    std::optional<std::size_t> best;
    float bestDistanceSquared = DECOR_PICK_RADIUS * DECOR_PICK_RADIUS;
    for (std::size_t index = 0; index < decors.size(); ++index) {
        const core::Vector2 delta = decors[index].position - worldPosition;
        const float distanceSquared = delta.lengthSquared();
        // <= (pas <) : a distance egale, le rang le plus eleve (pose le plus recemment) gagne,
        // conforme a l'ordre de superposition intra-couche (voir en-tete).
        if (distanceSquared <= bestDistanceSquared) {
            bestDistanceSquared = distanceSquared;
            best = index;
        }
    }
    return best;
}

}  // namespace hmi
