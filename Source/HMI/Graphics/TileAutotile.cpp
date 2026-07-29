#include "HMI/Graphics/TileAutotile.h"

#include <array>

namespace hmi {

namespace {

// Correspondance masque -> case de la planche, une entree par configuration (index = masque).
//
// Convention de lecture : la case dessine la silhouette de la tuile, donc les cotes ou la MATIERE
// s'arrete. Un bit a 1 = voisin solide = aucun bord a dessiner de ce cote.
//
// L'index de la planche est simplement le masque lu en base 4 (colonne = masque % 4, ligne =
// masque / 4). La table est neanmoins ecrite en clair plutot que calculee : elle documente ce que
// l'auteur d'une planche doit dessiner dans chaque case, ce qu'une formule ne dirait pas. C'est
// aussi ce qui la rend verifiable configuration par configuration.
constexpr std::array<AutotileCell, AUTOTILE_CONFIGURATION_COUNT> CELL_BY_MASK{{
    // 0  ---- : aucun voisin solide -> tuile isolee, bordee sur ses quatre cotes.
    AutotileCell{0, 0},
    // 1  U--- : voisin en haut -> extremite basse d'une colonne (ouverte vers le haut).
    AutotileCell{1, 0},
    // 2  -R-- : voisin a droite -> extremite gauche d'une rangee.
    AutotileCell{2, 0},
    // 3  UR-- : haut + droite -> coin exterieur bas-gauche.
    AutotileCell{3, 0},
    // 4  --D- : voisin en bas -> extremite haute d'une colonne.
    AutotileCell{0, 1},
    // 5  U-D- : haut + bas -> segment vertical de colonne (bords a gauche et a droite).
    AutotileCell{1, 1},
    // 6  -RD- : droite + bas -> coin exterieur haut-gauche.
    AutotileCell{2, 1},
    // 7  URD- : haut + droite + bas -> bord gauche d'une masse.
    AutotileCell{3, 1},
    // 8  ---L : voisin a gauche -> extremite droite d'une rangee.
    AutotileCell{0, 2},
    // 9  U--L : haut + gauche -> coin exterieur bas-droite.
    AutotileCell{1, 2},
    // 10 -R-L : gauche + droite -> segment horizontal (bords en haut et en bas).
    AutotileCell{2, 2},
    // 11 UR-L : haut + droite + gauche -> bord bas d'une masse.
    AutotileCell{3, 2},
    // 12 --DL : bas + gauche -> coin exterieur haut-droite.
    AutotileCell{0, 3},
    // 13 U-DL : haut + bas + gauche -> bord droit d'une masse.
    AutotileCell{1, 3},
    // 14 -RDL : droite + bas + gauche -> bord haut d'une masse (le dessus d'une plateforme).
    AutotileCell{2, 3},
    // 15 URDL : les quatre voisins solides -> interieur plein, invisible depuis l'exterieur.
    AutotileCell{3, 3},
}};

// Interieur plein : masque dont les quatre bits sont a 1.
constexpr std::uint8_t FULL_MASK = NEIGHBOR_UP | NEIGHBOR_RIGHT | NEIGHBOR_DOWN | NEIGHBOR_LEFT;

// Vrai si la case designee est solide, l'exterieur de la grille comptant comme solide (voir la
// convention documentee dans l'en-tete).
[[nodiscard]] bool isSolidOrOutside(const core::TileMap& tiles, int column, int row) noexcept {
    if (!tiles.inBounds(column, row)) {
        return true;
    }
    return tiles.isSolid(column, row);
}

}  // namespace

std::uint8_t solidNeighborMask(const core::TileMap& tiles, int column, int row) noexcept {
    std::uint8_t mask = 0;
    if (isSolidOrOutside(tiles, column, row - 1)) {
        mask |= NEIGHBOR_UP;
    }
    if (isSolidOrOutside(tiles, column + 1, row)) {
        mask |= NEIGHBOR_RIGHT;
    }
    if (isSolidOrOutside(tiles, column, row + 1)) {
        mask |= NEIGHBOR_DOWN;
    }
    if (isSolidOrOutside(tiles, column - 1, row)) {
        mask |= NEIGHBOR_LEFT;
    }
    return mask;
}

AutotileCell autotileCell(std::uint8_t mask) noexcept {
    // Seuls les quatre bits de poids faible portent du sens : masquer plutot que supposer
    // l'appelant discipline evite un depassement de table sur une valeur inattendue.
    return CELL_BY_MASK[mask & FULL_MASK];
}

AutotileCell autotileRepresentativeCell() noexcept {
    return CELL_BY_MASK[FULL_MASK];
}

}  // namespace hmi
