#include "Core/Gameplay/PlatformController.h"

#include <cmath>

#include "Core/Gameplay/PlatformPath.h"
#include "Core/Levels/TileMap.h"

namespace core {

namespace {

// Duree d'un pas fixe : le jeu tourne a 60 pas/s (core::FixedTimestep), meme hypothese que
// core::DangerController pour les dangers mobiles (EX-GP-051). En double : la distance parcourue
// est cumulee sur des millions de pas (voir boxAtStep), un 1/60 arrondi en float y introduirait
// une derive systematique.
constexpr double FIXED_DELTA_SECONDS = 1.0 / 60.0;

// Tolerance de contact "repose sur le dessus" : meme ordre de grandeur que
// BlockController::PUSH_TOUCH_TOLERANCE (bords qui se touchent, pas qui se chevauchent).
constexpr float REST_TOUCH_TOLERANCE = 0.05F;

// Chevauchement horizontal entre deux boites (aire strictement positive sur l'axe X).
[[nodiscard]] bool overlapsHorizontally(const Aabb& a, const Aabb& b) {
    return a.min.x < b.max.x && a.max.x > b.min.x;
}

}  // namespace

PlatformController::PlatformController(const Level& level) : _configs(level.platformConfigs()) {
    // Routes precalculees une fois pour toutes : boxAtStep est appelee plusieurs fois par pas et
    // par consommateur (portage du personnage, des blocs, interpolation d'affichage), on ne
    // recalcule jamais les longueurs de segments a l'execution.
    _paths.reserve(_configs.size());
    for (const MovingPlatformConfig& config : _configs) {
        _paths.push_back(buildPlatformPath(config));
    }
}

void PlatformController::update() noexcept {
    ++_stepCount;
}

Aabb PlatformController::boxAtStep(std::size_t index, long long stepCount) const noexcept {
    // Distance parcourue depuis le chargement, en DOUBLE : convertir (stepCount + phase) en float
    // perdrait le bit de poids faible des ~16,7 millions de pas (~77 h de jeu), ce qui decalerait
    // visiblement la plateforme en fin de longue session. Seule la position finale repasse en
    // float. Aucune accumulation d'un pas a l'autre : fonction pure de stepCount (EX-NFR-002).
    const double travelled = static_cast<double>(stepCount + _configs[index].phase) *
                             (static_cast<double>(_configs[index].speed) * FIXED_DELTA_SECONDS);
    return Aabb::fromTopLeftSize(platformPositionAt(_paths[index], travelled), Vector2{1.0F, 1.0F});
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
