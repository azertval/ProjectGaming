#include "Core/Physics/AabbVsAabb.h"

namespace core {
namespace {

// Même tolérance que kSkin (SweptCollision.cpp) : évite qu'un simple effleurement de frontière ne
// compte comme un chevauchement (cohérence des réglages entre les deux routines de balayage).
constexpr float K_SKIN = 1e-4F;

// Chevauchement d'aire strictement positive sur un axe, en ignorant une fine peau à la frontière.
[[nodiscard]] bool overlaps(float aMin, float aMax, float bMin, float bMax) noexcept {
    return aMin < bMax - K_SKIN && aMax > bMin + K_SKIN;
}

// Résout le blocage sur UN axe (le second appelé voit déjà la position de l'axe précédent résolue
// dans @p pos, cf. sweepAabbVsAabb) : chevauchement testé sur l'axe PERPENDICULAIRE (@p otherPos/
// @p otherSize contre @p obstacleOtherMin/Max), avance @p pos de @p delta sauf si un contact avec
// @p obstacleMin/Max l'en empêche -- @p pos est alors calé à la frontière. @return le normal signé
// (0 si non bloqué).
[[nodiscard]] float resolveAxisBlock(float& pos, float size, float delta, float otherPos,
                                     float otherSize, float obstacleOtherMin,
                                     float obstacleOtherMax, float obstacleMin,
                                     float obstacleMax) noexcept {
    float blocked = 0.0F;
    if (overlaps(otherPos, otherPos + otherSize, obstacleOtherMin, obstacleOtherMax)) {
        if (delta > 0.0F && pos + size <= obstacleMin + K_SKIN) {
            if (pos + size + delta > obstacleMin) {
                pos = obstacleMin - size;
                blocked = -1.0F;
            }
        } else if (delta < 0.0F && pos >= obstacleMax - K_SKIN) {
            if (pos + delta < obstacleMax) {
                pos = obstacleMax;
                blocked = 1.0F;
            }
        }
    }
    if (blocked == 0.0F) {
        pos += delta;
    }
    return blocked;
}

}  // namespace

SweepResult sweepAabbVsAabb(const Aabb& box, const Vector2& delta, const Aabb& obstacle) noexcept {
    Vector2 pos = box.min;
    const Vector2 size = box.max - box.min;
    Vector2 blocked{};

    // Axe X : perpendiculaire testé avec la position Y COURANTE (avant résolution Y), comme
    // sweepX le fait avec la grille — permet le glissement le long de l'obstacle.
    if (delta.x != 0.0F) {
        blocked.x = resolveAxisBlock(pos.x, size.x, delta.x, pos.y, size.y, obstacle.min.y,
                                     obstacle.max.y, obstacle.min.x, obstacle.max.x);
    }

    // Axe Y : perpendiculaire testé avec la position X DÉJÀ résolue ci-dessus (même ordre X→Y que
    // sweepAabb, EX-NFR-002 : déterministe).
    if (delta.y != 0.0F) {
        blocked.y = resolveAxisBlock(pos.y, size.y, delta.y, pos.x, size.x, obstacle.min.x,
                                     obstacle.max.x, obstacle.min.y, obstacle.max.y);
    }

    SweepResult result;
    result.position = pos;
    result.normal = blocked;
    result.hit = (blocked.x != 0.0F) || (blocked.y != 0.0F);
    return result;
}

}  // namespace core
