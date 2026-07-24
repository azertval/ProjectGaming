#include "Core/Physics/SlopeGeometry.h"

#include <algorithm>
#include <cmath>

#include "Core/Levels/TileMap.h"

namespace core {
namespace {

// Tolérance de calage : comparable à kSkin (SweptCollision.cpp), évite qu'une comparaison
// flottante stricte ne rate un contact exactement à la frontière.
constexpr float kFollowTolerance = 1e-3f;

// Évite qu'un bord de boîte tombant PILE sur une frontière de case n'en morde la case suivante
// (même rôle que kSkin, SweptCollision.cpp).
constexpr float kColumnSkin = 1e-4f;

}  // namespace

// Hauteur de surface (voir en-tête pour le repère local). Pentes a 45° : fonction lineaire sur
// toute la largeur de la case. Arrondis (LOT-23) : quart de cercle de rayon 1 (une case), centre
// au coin OPPOSE au coin haut de la pente equivalente — meme modele d'extension, un nouveau `case`
// par type suivable plutot qu'une fonction parallele (la passe de resolution, LOT-22-TACHE-02,
// n'a ainsi jamais besoin de connaitre le nombre ou la nature des familles de courbes geree ici).
//
// Concaves (EX-GP-007) : meme rayon (une case), memes valeurs aux deux bords que l'arrondi convexe
// de meme orientation, mais centre du cercle au coin HAUT du cote CREUX (au lieu du coin BAS du
// cote plein pour le convexe) — la courbure s'inverse : tangente horizontale du cote bas/creux
// (raccord lisse avec un sol plat en contrebas), verticale du cote haut/plein (raccord lisse avec
// un mur), l'exact inverse du convexe (tangente verticale du cote creux, horizontale du cote
// plein). Seule la courbe ENTRE les deux bords differe du convexe.
//
// Pentes/arrondis de PLAFOND (EX-GP-006/EX-GP-007) : leur FACE DU HAUT (celle qu'on foule en
// tombant dessus PAR-DESSUS, pas la silhouette inclinee/courbe du dessous geree par
// resolveCeilingSlopeFollow) est toujours PLATE, exactement au sommet de la case — la matiere
// pleine y commence toujours, quel que soit x (seule sa profondeur vers le bas varie). Une hauteur
// CONSTANTE de 0 suffit donc ici : reutilise resolveSlopeFollow tel quel (aucun code de resolution
// supplementaire), pour que le personnage ne tombe pas au travers d'un plafond incline accessible
// par le dessus.
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
        case TileType::ConcaveUpRight:
            // Memes bords que RoundedUpRight (haut a droite, creux en bas a gauche), mais centre
            // du cercle en (0, 0) (coin HAUT-gauche, cote creux) au lieu de (0, 1) : courbure
            // inversee (concave plutot que convexe).
            return std::sqrt(std::max(0.0f, 1.0f - localX * localX));
        case TileType::ConcaveUpLeft:
            // Symetrique : centre du cercle en (1, 0) (coin haut-droit).
            return std::sqrt(std::max(0.0f, 1.0f - (1.0f - localX) * (1.0f - localX)));
        case TileType::SlopeDownRight:
        case TileType::SlopeDownLeft:
        case TileType::RoundedDownRight:
        case TileType::RoundedDownLeft:
        case TileType::ConcaveDownRight:
        case TileType::ConcaveDownLeft:
            return 0.0f;  // face du haut plate, voir le commentaire ci-dessus
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
        case TileType::ConcaveDownRight:
            floorMirror = TileType::ConcaveUpRight;
            break;
        case TileType::ConcaveDownLeft:
            floorMirror = TileType::ConcaveUpLeft;
            break;
        default:
            return std::nullopt;
    }
    const std::optional<float> floorHeight = slopeSurfaceHeight(floorMirror, localX);
    return floorHeight ? std::optional<float>(1.0f - *floorHeight) : std::nullopt;
}

SlopeFollowResult resolveSlopeFollow(const Aabb& previousBox, const Aabb& newBox, float velocityY,
                                     const TileMap& tiles) noexcept {
    SlopeFollowResult result;
    if (velocityY < 0.0f) {
        return result;  // monte (vient de sauter) : le suivi de pente ne s'applique jamais
    }
    const float previousBottomY = previousBox.max.y;
    const float newBottomY = newBox.max.y;
    if (newBottomY < previousBottomY) {
        return result;  // par construction ne devrait pas arriver (velocityY >= 0), robustesse
    }

    const float centerX = (newBox.min.x + newBox.max.x) * 0.5f;
    const int width = tiles.width();
    const int centerColumn = static_cast<int>(std::floor(centerX));

    // Étendue horizontale couverte par la boîte pendant TOUT le pas — @p previousBox (avant) ET
    // @p newBox (après), pas seulement sa position finale : un déplacement horizontal pendant le
    // même pas qu'une chute peut faire sortir la boîte d'une colonne pertinente AVANT que le seuil
    // vertical n'y soit atteint, alors que cette colonne était encore couverte au début du pas —
    // l'ignorer la rendrait "invisible" à tort. Se réduit exactement au cas newBox seul dès que la
    // boîte ne bouge pas horizontalement ce pas (previousBox == newBox en x) : aucune régression
    // pour l'appelant immobile.
    const float leftEdge = (std::min)(previousBox.min.x, newBox.min.x);
    const float rightEdge = (std::max)(previousBox.max.x, newBox.max.x);
    const int colStart = std::clamp(static_cast<int>(std::floor(leftEdge)), 0, width - 1);
    const int colEnd = std::clamp(
        static_cast<int>(std::floor(rightEdge - kColumnSkin)), 0, width - 1);
    if (colStart > colEnd) {
        return result;
    }

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
        // Parmi les colonnes couvertes par la boîte, la surface valide la plus HAUTE (bord bas le
        // moins profond) est la bonne : c'est elle qui arrête la chute en premier, comme un
        // balayage classique — une colonne voisine plus permissive (case vide, ou pente moins
        // haute à cet endroit) ne doit jamais faire ignorer un appui plus haut ailleurs sous la
        // boîte.
        bool found = false;
        float bestSurfaceY = 0.0f;
        for (int col = colStart; col <= colEnd; ++col) {
            if (col < 0 || col >= width) {
                continue;
            }
            // Colonne centrale : localX depuis le CENTRE de la boîte (position finale). Colonne
            // supplémentaire : localX depuis le bord le plus GÉNÉREUX ayant pu la couvrir pendant
            // le pas (avant ou après), pas seulement le bord final.
            float localX;
            if (col == centerColumn) {
                localX = centerX - static_cast<float>(col);
            } else if (col < centerColumn) {
                localX = leftEdge - static_cast<float>(col);
            } else {
                localX = rightEdge - static_cast<float>(col);
            }
            const TileType tileType = tiles.tile(col, row);
            const std::optional<float> height = slopeSurfaceHeight(tileType, localX);
            if (!height) {
                continue;
            }
            // Une tuile de PLAFOND (`isCeilingSlope`) n'a de face du haut plate (`h == 0`, voir
            // l'en-tête) que pour porter un personnage qui tombe dessus PAR AU-DESSUS — jamais un
            // personnage bloqué PAR EN DESSOUS par sa silhouette (`resolveCeilingSlopeFollow`) et
            // dont le bord bas reste, du fait de sa propre hauteur, encore DANS la même case
            // (silhouette fine : blocage proche du sommet de la case, `EX-GP-007`). Exigé ici : le
            // bord bas devait déjà être AU-DESSUS (ou pile au sommet) de la case avant ce pas —
            // cohérent avec « tombe dessus », pas « était déjà dedans » (sans cette distinction, le
            // chevauchement résiduel après un blocage par en dessous serait pris pour un
            // atterrissage par-dessus, et téléporterait le personnage au-dessus du plafond).
            if (isCeilingSlope(tileType) && previousBottomY > static_cast<float>(row) + kFollowTolerance) {
                continue;
            }
            const float surfaceY = static_cast<float>(row) + *height;
            // Calage dès que le bord bas est À ou SOUS la surface (comme un sol : jamais en-
            // dessous), sans exiger d'être parti d'AU-DESSUS d'elle — un déplacement HORIZONTAL
            // peut faire entrer dans une nouvelle colonne dont la pente exige une hauteur plus
            // haute que la position précédente (qui appartenait à une autre colonne, sans rapport
            // avec cette surface-ci).
            if (newBottomY >= surfaceY - kFollowTolerance && (!found || surfaceY < bestSurfaceY)) {
                found = true;
                bestSurfaceY = surfaceY;
            }
        }
        // L'itération part de la ligne la plus haute du pas (rowStart) : la première ligne où une
        // colonne couverte offre un appui valide est la bonne (comme un balayage classique).
        if (found) {
            result.grounded = true;
            result.bottomY = bestSurfaceY;
            return result;
        }
    }
    return result;
}

// Miroir exact de resolveSlopeFollow ci-dessus, pour le bord HAUT plutot que bas, declenche en
// MONTANT (saut) plutot qu'en tombant -- voir la documentation de l'en-tete (EX-GP-006/EX-GP-007).
CeilingSlopeFollowResult resolveCeilingSlopeFollow(float previousTopY, float sweptMinX,
                                                   float sweptMaxX, const Aabb& newBox,
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
    const int width = tiles.width();
    const int centerColumn = static_cast<int>(std::floor(centerX));

    // Étendue horizontale couverte par la boîte pendant TOUTE la montée courante (fournie par
    // l'appelant, @p sweptMinX/@p sweptMaxX) unionée avec sa position actuelle : voir le
    // commentaire de l'en-tête — un pas seul ne suffit pas toujours (marche rapide combinée à une
    // montée lente peut faire "disparaître" la colonne qui aurait dû bloquer sur PLUSIEURS pas, pas
    // seulement le précédent).
    const float leftEdge = (std::min)(sweptMinX, newBox.min.x);
    const float rightEdge = (std::max)(sweptMaxX, newBox.max.x);
    const int colStart = std::clamp(static_cast<int>(std::floor(leftEdge)), 0, width - 1);
    const int colEnd = std::clamp(
        static_cast<int>(std::floor(rightEdge - kColumnSkin)), 0, width - 1);
    if (colStart > colEnd) {
        return result;
    }

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
        // Parmi les colonnes couvertes par la boîte, la silhouette valide la plus BASSE (bord haut
        // le moins profond, donc atteinte le plus tôt en montant) est la bonne : symétrique du
        // choix de la surface la plus haute dans resolveSlopeFollow.
        bool found = false;
        float bestSurfaceY = 0.0f;
        for (int col = colStart; col <= colEnd; ++col) {
            if (col < 0 || col >= width) {
                continue;
            }
            // Même règle que resolveSlopeFollow : colonne centrale → localX depuis le centre de la
            // boîte (position finale) ; colonne supplémentaire → localX depuis le bord le plus
            // généreux ayant pu la couvrir pendant le pas.
            float localX;
            if (col == centerColumn) {
                localX = centerX - static_cast<float>(col);
            } else if (col < centerColumn) {
                localX = leftEdge - static_cast<float>(col);
            } else {
                localX = rightEdge - static_cast<float>(col);
            }
            const std::optional<float> height = ceilingSlopeHeight(tiles.tile(col, row), localX);
            if (!height) {
                continue;
            }
            const float surfaceY = static_cast<float>(row) + *height;
            // Blocage des que le bord haut est A ou SOUS la silhouette (jamais au-dessus) :
            // symetrique du calage "a ou sous la surface" du sol, miroir verticalement.
            if (newTopY <= surfaceY + kFollowTolerance && (!found || surfaceY > bestSurfaceY)) {
                found = true;
                bestSurfaceY = surfaceY;
            }
        }
        if (found) {
            result.blocked = true;
            result.topY = bestSurfaceY;
            return result;
        }
    }
    return result;
}

}  // namespace core
