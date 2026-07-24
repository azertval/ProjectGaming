#include "Core/Physics/SlopeGeometry.h"

#include <algorithm>
#include <cmath>

#include "Core/Levels/TileMap.h"

namespace core {
namespace {

// Tolérance de calage : comparable à kSkin (SweptCollision.cpp), évite qu'une comparaison
// flottante stricte ne rate un contact exactement à la frontière.
constexpr float kFollowTolerance = 1e-3f;

}  // namespace

// Hauteur de surface (voir en-tête pour le repère local). Pentes a 45° : fonction lineaire sur
// toute la largeur de la case. Arrondis (LOT-23) : quart de cercle de rayon 1 (une case), centre
// au coin OPPOSE au coin haut de la pente equivalente — meme modele d'extension, un nouveau `case`
// par type suivable plutot qu'une fonction parallele (la passe de resolution, LOT-22-TACHE-02,
// n'a ainsi jamais besoin de connaitre le nombre ou la nature des familles de courbes geree ici).
std::optional<float> slopeSurfaceHeight(TileType type, float localX) noexcept {
    switch (type) {
        case TileType::SlopeUpRight:
            // Monte de gauche a droite : haut (0) a droite, bas (1) a gauche.
            return 1.0f - localX;
        case TileType::SlopeUpLeft:
            // Monte de droite a gauche : haut (0) a gauche, bas (1) a droite.
            return localX;
        case TileType::RoundedUpRight:
            // Haut a droite, creux en bas a gauche : centre du cercle en (0, 1) (coin bas-gauche).
            return 1.0f - std::sqrt(std::max(0.0f, 1.0f - (1.0f - localX) * (1.0f - localX)));
        case TileType::RoundedUpLeft:
            // Symetrique : centre du cercle en (1, 1) (coin bas-droit).
            return 1.0f - std::sqrt(std::max(0.0f, 1.0f - localX * localX));
        default:
            return std::nullopt;
    }
}

// Hauteur de silhouette d'une pente/arrondi de plafond (EX-GP-006) : miroir vertical exact de la
// surface de sol de meme orientation (1 - hauteur du miroir), pas une famille de formules
// dupliquee -- voir la documentation de l'en-tete.
std::optional<float> ceilingSlopeHeight(TileType type, float localX) noexcept {
    TileType floorMirror{};
    switch (type) {
        case TileType::SlopeDownRight:
            floorMirror = TileType::SlopeUpRight;
            break;
        case TileType::SlopeDownLeft:
            floorMirror = TileType::SlopeUpLeft;
            break;
        case TileType::RoundedDownRight:
            floorMirror = TileType::RoundedUpRight;
            break;
        case TileType::RoundedDownLeft:
            floorMirror = TileType::RoundedUpLeft;
            break;
        default:
            return std::nullopt;
    }
    const std::optional<float> floorHeight = slopeSurfaceHeight(floorMirror, localX);
    return floorHeight ? std::optional<float>(1.0f - *floorHeight) : std::nullopt;
}

SlopeFollowResult resolveSlopeFollow(float previousBottomY, const Aabb& newBox, float velocityY,
                                     const TileMap& tiles) noexcept {
    SlopeFollowResult result;
    if (velocityY < 0.0f) {
        return result;  // monte (vient de sauter) : le suivi de pente ne s'applique jamais
    }
    const float newBottomY = newBox.max.y;
    if (newBottomY < previousBottomY) {
        return result;  // par construction ne devrait pas arriver (velocityY >= 0), robustesse
    }

    const float centerX = (newBox.min.x + newBox.max.x) * 0.5f;
    const int column = static_cast<int>(std::floor(centerX));
    if (column < 0 || column >= tiles.width()) {
        return result;
    }
    const float localX = centerX - static_cast<float>(column);

    // Parcourt TOUTES les lignes traversées par le bord bas pendant ce pas (pas seulement la
    // position finale) : une chute rapide pourrait sinon « sauter » une pente en un seul pas, la
    // grille classique ne la voyant pas comme solide (isSolid == false pour une pente).
    // `- kFollowTolerance` avant le floor : si le bord bas est EXACTEMENT sur une frontière de case
    // (ex. sortie d'un sol plat qui rejoint pile la base d'une pente montante située une ligne au-
    // dessus), floor() l'attribue à la ligne du dessous et raterait la ligne de la pente — cas
    // pourtant courant (une pente relie normalement deux paliers d'une ligne d'écart).
    const int rowStart = static_cast<int>(std::floor(previousBottomY - kFollowTolerance));
    const int rowEnd = static_cast<int>(std::floor(newBottomY));
    for (int row = rowStart; row <= rowEnd; ++row) {
        if (row < 0 || row >= tiles.height()) {
            continue;
        }
        const std::optional<float> height = slopeSurfaceHeight(tiles.tile(column, row), localX);
        if (!height) {
            continue;
        }
        const float surfaceY = static_cast<float>(row) + *height;
        // Calage dès que le bord bas est À ou SOUS la surface (comme un sol : jamais en-dessous),
        // sans exiger d'être parti d'AU-DESSUS d'elle — un déplacement HORIZONTAL peut faire entrer
        // dans une nouvelle colonne dont la pente exige une hauteur plus haute que la position
        // précédente (qui appartenait à une autre colonne, sans rapport avec cette surface-ci).
        // L'itération part de la ligne la plus haute du pas (rowStart) : la première surface
        // valide rencontrée est la plus haute, donc la bonne (comme un balayage classique).
        if (newBottomY >= surfaceY - kFollowTolerance) {
            result.grounded = true;
            result.bottomY = surfaceY;
            return result;
        }
    }
    return result;
}

// Miroir exact de resolveSlopeFollow ci-dessus, pour le bord HAUT plutot que bas, declenche en
// MONTANT (saut) plutot qu'en tombant -- voir la documentation de l'en-tete (EX-GP-006).
CeilingSlopeFollowResult resolveCeilingSlopeFollow(float previousTopY, const Aabb& newBox,
                                                   float velocityY, const TileMap& tiles) noexcept {
    CeilingSlopeFollowResult result;
    if (velocityY >= 0.0f) {
        return result;  // tombe ou immobile : un plafond ne bloque jamais autre chose qu'un saut
    }
    const float newTopY = newBox.min.y;
    if (newTopY > previousTopY) {
        return result;  // par construction ne devrait pas arriver (velocityY < 0), robustesse
    }

    const float centerX = (newBox.min.x + newBox.max.x) * 0.5f;
    const int column = static_cast<int>(std::floor(centerX));
    if (column < 0 || column >= tiles.width()) {
        return result;
    }
    const float localX = centerX - static_cast<float>(column);

    // Parcourt TOUTES les lignes traversees par le bord haut pendant ce pas (comme
    // resolveSlopeFollow, symetrique) : un saut rapide ne doit jamais "traverser" un plafond
    // incline/courbe en un seul pas. Contrairement au sol, aucun ajustement de tolerance n'est
    // necessaire avant le floor() ici : le parcours DECROISSANT (rowStart >= rowEnd, on monte)
    // inclut deja naturellement la ligne d'entree sans risque de l'omettre par arrondi.
    const int rowStart = static_cast<int>(std::floor(previousTopY));
    const int rowEnd = static_cast<int>(std::floor(newTopY));
    for (int row = rowStart; row >= rowEnd; --row) {
        if (row < 0 || row >= tiles.height()) {
            continue;
        }
        const std::optional<float> height = ceilingSlopeHeight(tiles.tile(column, row), localX);
        if (!height) {
            continue;
        }
        const float surfaceY = static_cast<float>(row) + *height;
        // Blocage des que le bord haut est A ou SOUS la silhouette (jamais au-dessus) : symetrique
        // du calage "a ou sous la surface" du sol, miroir verticalement.
        if (newTopY <= surfaceY + kFollowTolerance) {
            result.blocked = true;
            result.topY = surfaceY;
            return result;
        }
    }
    return result;
}

}  // namespace core
