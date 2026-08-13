#include "Core/Gameplay/PlatformController.h"

#include <cmath>

#include "Core/Levels/TileMap.h"

namespace core {

namespace {

// Duree d'un pas fixe : le jeu tourne a 60 pas/s (core::FixedTimestep), meme hypothese que
// core::DangerController pour les dangers mobiles (EX-GP-051).
constexpr float FIXED_DELTA_SECONDS = 1.0F / 60.0F;

// Tolerance de contact "repose sur le dessus" : meme ordre de grandeur que
// BlockController::PUSH_TOUCH_TOLERANCE (bords qui se touchent, pas qui se chevauchent).
constexpr float REST_TOUCH_TOLERANCE = 0.05F;

// Chevauchement horizontal entre deux boites (aire strictement positive sur l'axe X).
[[nodiscard]] bool overlapsHorizontally(const Aabb& a, const Aabb& b) {
    return a.min.x < b.max.x && a.max.x > b.min.x;
}

}  // namespace

PlatformController::PlatformController(const Level& level) : _configs(level.platformConfigs()) {}

void PlatformController::update() noexcept {
    ++_stepCount;
}

Aabb PlatformController::boxAtStep(std::size_t index, long long stepCount) const noexcept {
    const MovingPlatformConfig& config = _configs[index];
    const Vector2 start{static_cast<float>(config.startPosition.column),
                        static_cast<float>(config.startPosition.row)};
    const Vector2 end{static_cast<float>(config.endPosition.column),
                      static_cast<float>(config.endPosition.row)};
    const Vector2 segment = end - start;
    const float distance = segment.length();

    Vector2 topLeft = start;
    if (distance > 0.0F) {
        // Aller-retour triangulaire deterministe le long du segment start->end : 0 -> distance ->
        // 0, cycle = 2 * distance. Meme formule que DangerController::moverBox, generalisee a un
        // segment 2D quelconque plutot qu'un seul axe.
        const float distancePerStep = config.speed * FIXED_DELTA_SECONDS;
        const float totalDistance = static_cast<float>(stepCount + config.phase) * distancePerStep;
        const float cycleLength = distance * 2.0F;
        float phase = std::fmod(totalDistance, cycleLength);
        if (phase < 0.0F) {
            phase += cycleLength;
        }
        const float offset = phase <= distance ? phase : cycleLength - phase;
        topLeft = start + (segment * (offset / distance));
    }
    return Aabb::fromTopLeftSize(topLeft, Vector2{1.0F, 1.0F});
}

Aabb PlatformController::boxAt(std::size_t index) const noexcept {
    return boxAtStep(index, _stepCount);
}

Aabb PlatformController::previousBoxAt(std::size_t index) const noexcept {
    return boxAtStep(index, _stepCount - 1);
}

Vector2 PlatformController::deltaAt(std::size_t index) const noexcept {
    return boxAt(index).min - previousBoxAt(index).min;
}

std::vector<PlatformSample> PlatformController::samples() const {
    std::vector<PlatformSample> result;
    result.reserve(_configs.size());
    for (std::size_t index = 0; index < _configs.size(); ++index) {
        result.push_back(sampleAt(index));
    }
    return result;
}

bool restsOnTopOfPlatform(const Aabb& box, const Aabb& platformBox) noexcept {
    if (!overlapsHorizontally(box, platformBox)) {
        return false;
    }
    return std::fabs(box.max.y - platformBox.min.y) <= REST_TOUCH_TOLERANCE;
}

bool isSquishedByPlatform(const Aabb& carriedBox, const TileMap& collision) noexcept {
    const int firstColumn = static_cast<int>(std::floor(carriedBox.min.x));
    const int lastColumn = static_cast<int>(std::ceil(carriedBox.max.x)) - 1;
    const int firstRow = static_cast<int>(std::floor(carriedBox.min.y));
    const int lastRow = static_cast<int>(std::ceil(carriedBox.max.y)) - 1;
    for (int row = firstRow; row <= lastRow; ++row) {
        for (int column = firstColumn; column <= lastColumn; ++column) {
            if (!collision.inBounds(column, row)) {
                continue;
            }
            const Aabb cell = Aabb::fromTopLeftSize(
                Vector2{static_cast<float>(column), static_cast<float>(row)}, Vector2{1.0F, 1.0F});
            const bool overlaps = carriedBox.min.x < cell.max.x && carriedBox.max.x > cell.min.x &&
                                  carriedBox.min.y < cell.max.y && carriedBox.max.y > cell.min.y;
            if (overlaps && collision.isSolid(column, row)) {
                return true;
            }
        }
    }
    return false;
}

}  // namespace core
