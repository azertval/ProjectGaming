#include "Core/Physics/SweptCollision.h"

#include <algorithm>  // std::clamp
#include <cmath>      // std::floor

#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace core {
namespace {

// Fine « peau » : évite d'accrocher la tuile que le bord de la boîte ne fait qu'effleurer
// (bord exactement sur une frontière de cellule). Détermine la portée PERPENDICULAIRE au balayage.
constexpr float kSkin = 1e-4f;

// Choix technique (voir en-tête) : on résout AXE PAR AXE. Chaque axe est un balayage 1D continu.
// L'avantage décisif sur la méthode diagonale « Minkowski + slabs » est le CLAMP DIRECT : on cale
// la position sur la coordonnée entière du mur (ex. `col - size.x`), sans jamais faire
// `pos += delta * t` — donc AUCUNE dérive flottante qui ferait « coller » au mur (bug du bord
// interne constaté au test « glissement »).

// Balayage 1D sur l'axe X. Renvoie l'abscisse résolue du coin haut-gauche ; règle sign à
// -1 (mur à droite → normale vers la gauche), +1 (mur à gauche) ou 0 (aucun contact).
float sweepX(const TileMap& tiles, const Vector2& pos, const Vector2& size, float dx, float& sign) {
    sign = 0.0f;
    const float newX = pos.x + dx;
    const int width = tiles.width();
    const int height = tiles.height();

    // Lignes réellement occupées par la boîte (la peau évite de mordre la ligne juste effleurée).
    const int rowMin = std::clamp(static_cast<int>(std::floor(pos.y)), 0, height - 1);
    const int rowMax = std::clamp(static_cast<int>(std::floor(pos.y + size.y - kSkin)), 0, height - 1);

    if (dx > 0.0f) {
        // On balaie de la colonne du bord droit actuel jusqu'à celle du bord droit visé : le
        // premier solide rencontré est le plus proche (aucune traversée, même si dx ≫ 1 tuile).
        const int colStart = std::clamp(static_cast<int>(std::floor(pos.x + size.x)), 0, width - 1);
        const int colEnd = std::clamp(static_cast<int>(std::floor(newX + size.x)), 0, width - 1);
        for (int col = colStart; col <= colEnd; ++col) {
            for (int row = rowMin; row <= rowMax; ++row) {
                if (tiles.isSolid(col, row)) {
                    sign = -1.0f;
                    return static_cast<float>(col) - size.x;  // bord droit collé au mur (col)
                }
            }
        }
    } else {
        const int colStart = std::clamp(static_cast<int>(std::floor(pos.x)), 0, width - 1);
        const int colEnd = std::clamp(static_cast<int>(std::floor(newX)), 0, width - 1);
        for (int col = colStart; col >= colEnd; --col) {
            for (int row = rowMin; row <= rowMax; ++row) {
                if (tiles.isSolid(col, row)) {
                    sign = 1.0f;
                    return static_cast<float>(col + 1);  // bord gauche collé au mur (col + 1)
                }
            }
        }
    }
    return newX;  // rien sur le chemin : déplacement complet
}

// Symétrique sur l'axe Y (lignes ↔ colonnes). sign : -1 (sol dessous), +1 (plafond dessus), 0.
float sweepY(const TileMap& tiles, const Vector2& pos, const Vector2& size, float dy, float& sign) {
    sign = 0.0f;
    const float newY = pos.y + dy;
    const int width = tiles.width();
    const int height = tiles.height();

    const int colMin = std::clamp(static_cast<int>(std::floor(pos.x)), 0, width - 1);
    const int colMax = std::clamp(static_cast<int>(std::floor(pos.x + size.x - kSkin)), 0, width - 1);

    if (dy > 0.0f) {
        const int rowStart = std::clamp(static_cast<int>(std::floor(pos.y + size.y)), 0, height - 1);
        const int rowEnd = std::clamp(static_cast<int>(std::floor(newY + size.y)), 0, height - 1);
        for (int row = rowStart; row <= rowEnd; ++row) {
            for (int col = colMin; col <= colMax; ++col) {
                if (tiles.isSolid(col, row)) {
                    sign = -1.0f;
                    return static_cast<float>(row) - size.y;  // bord bas posé sur le sol (row)
                }
            }
        }
    } else {
        const int rowStart = std::clamp(static_cast<int>(std::floor(pos.y)), 0, height - 1);
        const int rowEnd = std::clamp(static_cast<int>(std::floor(newY)), 0, height - 1);
        for (int row = rowStart; row >= rowEnd; --row) {
            for (int col = colMin; col <= colMax; ++col) {
                if (tiles.isSolid(col, row)) {
                    sign = 1.0f;
                    return static_cast<float>(row + 1);  // bord haut sous le plafond (row + 1)
                }
            }
        }
    }
    return newY;
}

}  // namespace

// Déplace la boîte de `delta` en s'arrêtant aux tuiles solides, puis en glissant (voir en-tête).
SweepResult sweepAabb(const Aabb& box, const Vector2& delta, const TileMap& tiles) {
    const Vector2 size = box.max - box.min;  // dimensions pleines
    Vector2 pos = box.min;                   // coin haut-gauche
    Vector2 blocked{};

    // Résolution X puis Y. La passe Y part de `pos.x` DÉJÀ résolu : c'est ce qui permet le
    // glissement (buter sur un mur en X, puis continuer à tomber en Y le long de ce mur).
    if (delta.x != 0.0f) {
        pos.x = sweepX(tiles, pos, size, delta.x, blocked.x);
    }
    if (delta.y != 0.0f) {
        pos.y = sweepY(tiles, pos, size, delta.y, blocked.y);
    }

    SweepResult result;
    result.position = pos;
    result.normal = blocked;
    result.hit = (blocked.x != 0.0f) || (blocked.y != 0.0f);
    return result;
}

}  // namespace core
