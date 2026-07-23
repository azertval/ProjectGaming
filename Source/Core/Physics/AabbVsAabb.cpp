#include "Core/Physics/AabbVsAabb.h"

namespace core {
namespace {

// Même tolérance que kSkin (SweptCollision.cpp) : évite qu'un simple effleurement de frontière ne
// compte comme un chevauchement (cohérence des réglages entre les deux routines de balayage).
constexpr float kSkin = 1e-4f;

// Chevauchement d'aire strictement positive sur un axe, en ignorant une fine peau à la frontière.
[[nodiscard]] bool overlaps(float aMin, float aMax, float bMin, float bMax) noexcept {
    return aMin < bMax - kSkin && aMax > bMin + kSkin;
}

}  // namespace

SweepResult sweepAabbVsAabb(const Aabb& box, const Vector2& delta, const Aabb& obstacle) noexcept {
    Vector2 pos = box.min;
    const Vector2 size = box.max - box.min;
    Vector2 blocked{};

    // Axe X : perpendiculaire testé avec la position Y COURANTE (avant résolution Y), comme
    // sweepX le fait avec la grille — permet le glissement le long de l'obstacle.
    if (delta.x != 0.0f) {
        if (overlaps(pos.y, pos.y + size.y, obstacle.min.y, obstacle.max.y)) {
            if (delta.x > 0.0f && pos.x + size.x <= obstacle.min.x + kSkin) {
                if (pos.x + size.x + delta.x > obstacle.min.x) {
                    pos.x = obstacle.min.x - size.x;
                    blocked.x = -1.0f;
                }
            } else if (delta.x < 0.0f && pos.x >= obstacle.max.x - kSkin) {
                if (pos.x + delta.x < obstacle.max.x) {
                    pos.x = obstacle.max.x;
                    blocked.x = 1.0f;
                }
            }
        }
        if (blocked.x == 0.0f) {
            pos.x += delta.x;
        }
    }

    // Axe Y : perpendiculaire testé avec la position X DÉJÀ résolue ci-dessus (même ordre X→Y que
    // sweepAabb, EX-NFR-002 : déterministe).
    if (delta.y != 0.0f) {
        if (overlaps(pos.x, pos.x + size.x, obstacle.min.x, obstacle.max.x)) {
            if (delta.y > 0.0f && pos.y + size.y <= obstacle.min.y + kSkin) {
                if (pos.y + size.y + delta.y > obstacle.min.y) {
                    pos.y = obstacle.min.y - size.y;
                    blocked.y = -1.0f;
                }
            } else if (delta.y < 0.0f && pos.y >= obstacle.max.y - kSkin) {
                if (pos.y + delta.y < obstacle.max.y) {
                    pos.y = obstacle.max.y;
                    blocked.y = 1.0f;
                }
            }
        }
        if (blocked.y == 0.0f) {
            pos.y += delta.y;
        }
    }

    SweepResult result;
    result.position = pos;
    result.normal = blocked;
    result.hit = (blocked.x != 0.0f) || (blocked.y != 0.0f);
    return result;
}

}  // namespace core
