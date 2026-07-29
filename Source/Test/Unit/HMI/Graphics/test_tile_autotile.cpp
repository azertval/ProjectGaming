/**
 * @file test_tile_autotile.cpp
 * @brief Tests unitaires des raccords automatiques (LOT-42, EX-EDIT-025).
 */

#include <cstdint>
#include <set>

#include <gtest/gtest.h>

#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/TileAutotile.h"

namespace {

/// Grille 5x5 entierement vide, dans laquelle les tests posent des solides.
core::TileMap emptyGrid() {
    return core::TileMap{5, 5};
}

}  // namespace

/**
 * @brief Les seize configurations donnent seize cases distinctes de la planche.
 * \castest{<b>Les seize configurations de voisinage donnent seize cases distinctes.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la case de chacun des seize masques possibles.<br/>
 * \tattendu Les seize cases sont deux a deux differentes et tiennent dans la planche 4x4.
 * }
 */
TEST(TileAutotileTest, SeizeConfigurationsSeizeCasesDistinctes) {
    std::set<std::pair<int, int>> cells;
    for (std::uint8_t mask = 0; mask < hmi::AUTOTILE_CONFIGURATION_COUNT; ++mask) {
        const hmi::AutotileCell cell = hmi::autotileCell(mask);

        EXPECT_GE(cell.column, 0);
        EXPECT_LT(cell.column, hmi::AUTOTILE_SHEET_SIDE);
        EXPECT_GE(cell.row, 0);
        EXPECT_LT(cell.row, hmi::AUTOTILE_SHEET_SIDE);

        cells.insert({cell.column, cell.row});
    }

    // Deux configurations partageant une case rendraient une des seize inaccessible a l'auteur.
    EXPECT_EQ(cells.size(), static_cast<std::size_t>(hmi::AUTOTILE_CONFIGURATION_COUNT));
}

/**
 * @brief Chaque masque designe la case attendue de la planche.
 * \castest{<b>Chaque masque de voisinage designe la case attendue de la planche.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Verifier la case des seize masques, un a un.<br/>
 * \tattendu Chaque masque rend la case documentee dans la table de correspondance.
 * }
 */
TEST(TileAutotileTest, TableDeCorrespondanceExhaustive) {
    // Ancre de non-regression : changer cette table change l'apparence de toutes les planches
    // deja dessinees par un auteur.
    const struct {
        std::uint8_t mask;
        int column;
        int row;
    } expectations[] = {
        {0, 0, 0},
        {hmi::NEIGHBOR_UP, 1, 0},
        {hmi::NEIGHBOR_RIGHT, 2, 0},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT, 3, 0},
        {hmi::NEIGHBOR_DOWN, 0, 1},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_DOWN, 1, 1},
        {hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN, 2, 1},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN, 3, 1},
        {hmi::NEIGHBOR_LEFT, 0, 2},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_LEFT, 1, 2},
        {hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_LEFT, 2, 2},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_LEFT, 3, 2},
        {hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 0, 3},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 1, 3},
        {hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 2, 3},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 3, 3},
    };

    for (const auto& expected : expectations) {
        const hmi::AutotileCell cell = hmi::autotileCell(expected.mask);
        EXPECT_EQ(cell.column, expected.column) << "masque " << static_cast<int>(expected.mask);
        EXPECT_EQ(cell.row, expected.row) << "masque " << static_cast<int>(expected.mask);
    }
}

/**
 * @brief L'exterieur de la grille compte comme solide.
 * \castest{<b>L'exterieur de la grille compte comme solide.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer un solide isole dans le coin haut-gauche d'une grille vide.<br/>
 * \tattendu Ses voisins haut et gauche, hors grille, sont comptes solides ; bas et droite non.
 * }
 */
TEST(TileAutotileTest, ExterieurDeLaGrilleCompteSolide) {
    core::TileMap tiles = emptyGrid();
    tiles.setTile(0, 0, core::TileType::Solid);

    // Convention explicite : un mur de bordure ne doit pas dessiner de contour sur sa face
    // invisible, hors du niveau.
    const std::uint8_t mask = hmi::solidNeighborMask(tiles, 0, 0);
    EXPECT_TRUE((mask & hmi::NEIGHBOR_UP) != 0);
    EXPECT_TRUE((mask & hmi::NEIGHBOR_LEFT) != 0);
    EXPECT_FALSE((mask & hmi::NEIGHBOR_RIGHT) != 0);
    EXPECT_FALSE((mask & hmi::NEIGHBOR_DOWN) != 0);
}

/**
 * @brief Une tuile entouree de vide au centre de la grille est isolee.
 * \castest{<b>Une tuile solide entouree de vide donne la configuration isolee.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un solide au centre d'une grille vide, loin des bords.<br/>
 * \tattendu Son masque est nul et sa case est celle de la tuile isolee.
 * }
 */
TEST(TileAutotileTest, TuileIsoleeAuCentre) {
    core::TileMap tiles = emptyGrid();
    tiles.setTile(2, 2, core::TileType::Solid);

    EXPECT_EQ(hmi::solidNeighborMask(tiles, 2, 2), 0);
    EXPECT_EQ(hmi::autotileCell(0), (hmi::AutotileCell{0, 0}));
}

/**
 * @brief Le dessus d'une plateforme differe de son interieur.
 * \castest{<b>Le dessus d'une plateforme pleine differe de son interieur.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Remplir de solide les lignes 2 a 4 d'une grille vide.<br/>
 * 2. Comparer la case de la ligne de surface et celle d'une case enfouie.<br/>
 * \tattendu La surface rend la case « bord haut », l'interieur la case « plein » ; elles different.
 * }
 */
TEST(TileAutotileTest, DessusDePlateformeDistinctDeLInterieur) {
    core::TileMap tiles = emptyGrid();
    for (int row = 2; row < 5; ++row) {
        for (int column = 0; column < 5; ++column) {
            tiles.setTile(column, row, core::TileType::Solid);
        }
    }

    // C'est exactement ce que le rendu en couleurs plates ne savait pas faire : la grille restait
    // visible et le dessus d'une plateforme etait identique a son interieur.
    const hmi::AutotileCell surface = hmi::autotileCell(hmi::solidNeighborMask(tiles, 2, 2));
    const hmi::AutotileCell inside = hmi::autotileCell(hmi::solidNeighborMask(tiles, 2, 3));

    EXPECT_EQ(surface, (hmi::AutotileCell{2, 3}));  // droite + bas + gauche = bord haut
    EXPECT_EQ(inside, (hmi::AutotileCell{3, 3}));   // les quatre voisins = interieur plein
    EXPECT_FALSE(surface == inside);
}

/**
 * @brief Les blocs poussables se raccordent aux murs.
 * \castest{<b>Le voisinage est celui de la solidite, pas du type de tuile.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un solide et, a sa droite, un bloc poussable.<br/>
 * \tattendu Le solide compte le bloc comme voisin solide.
 * }
 */
TEST(TileAutotileTest, RaccordEntreTypesSolidesDifferents) {
    core::TileMap tiles = emptyGrid();
    tiles.setTile(2, 2, core::TileType::Solid);
    tiles.setTile(3, 2, core::TileType::Block);

    // Se raccorder au seul type identique laisserait une couture visible partout ou un bloc jouxte
    // un mur, alors que les deux forment visuellement la meme matiere.
    EXPECT_TRUE((hmi::solidNeighborMask(tiles, 2, 2) & hmi::NEIGHBOR_RIGHT) != 0);
}

/**
 * @brief Les pentes ne participent pas au voisinage solide.
 * \castest{<b>Une pente n'est pas comptee comme voisin solide.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un solide et, a sa droite, une pente montante.<br/>
 * \tattendu Le solide ne compte pas la pente comme voisin solide.
 * }
 */
TEST(TileAutotileTest, PenteNonCompteeCommeVoisinSolide) {
    core::TileMap tiles = emptyGrid();
    tiles.setTile(2, 2, core::TileType::Solid);
    tiles.setTile(3, 2, core::TileType::SlopeUpRight);

    // core::isSolid exclut les pentes : elles restent en mode single, avec leur masque de
    // silhouette. Coherent avec la physique, qui les traite par une passe de suivi dediee.
    EXPECT_FALSE((hmi::solidNeighborMask(tiles, 2, 2) & hmi::NEIGHBOR_RIGHT) != 0);
}

/**
 * @brief La case representative est l'interieur plein.
 * \castest{<b>La case representative d'une planche est son interieur plein.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander la case representative de la planche.<br/>
 * \tattendu Elle est celle du masque a quatre voisins, et non la case zero.
 * }
 */
TEST(TileAutotileTest, CaseRepresentativeEstLInterieurPlein) {
    const hmi::AutotileCell full = hmi::autotileCell(
        hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT);

    EXPECT_EQ(hmi::autotileRepresentativeCell(), full);
    EXPECT_FALSE(hmi::autotileRepresentativeCell() == (hmi::AutotileCell{0, 0}));
}

/**
 * @brief Les bits hors des quatre voisins sont ignores.
 * \castest{<b>Un masque portant des bits parasites reste dans la planche.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander la case d'un masque dont les bits de poids fort sont a 1.<br/>
 * \tattendu La case est celle des quatre bits de poids faible, sans depassement de table.
 * }
 */
TEST(TileAutotileTest, BitsParasitesIgnores) {
    constexpr std::uint8_t noisy = 0xF0 | hmi::NEIGHBOR_UP;

    EXPECT_EQ(hmi::autotileCell(noisy), hmi::autotileCell(hmi::NEIGHBOR_UP));
}
