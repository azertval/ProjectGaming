#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/TileTaxonomy.h"

namespace {

// Aplati la taxonomie en la liste de tous les types qu'elle contient (directs + sous-groupes),
// dans l'ordre d'affichage.
std::vector<core::TileType> flatten(const std::vector<hmi::TileCategory>& taxonomy) {
    std::vector<core::TileType> types;
    for (const hmi::TileCategory& category : taxonomy) {
        for (const hmi::TileEntry& entry : category.tiles) {
            types.push_back(entry.type);
        }
        for (const hmi::TileSubgroup& subgroup : category.subgroups) {
            for (const hmi::TileEntry& entry : subgroup.tiles) {
                types.push_back(entry.type);
            }
        }
    }
    return types;
}

// Nombre total de valeurs de l'énumération core::TileType (dernière valeur + 1).
constexpr std::size_t TILE_TYPE_COUNT = static_cast<std::size_t>(core::TileType::DangerBlink) + 1;

}  // namespace

// Chaque type de tuile figure exactement une fois : aucun doublon, aucun oubli.
TEST(TileTaxonomy, ChaqueTypeFigureExactementUneFois) {
    const std::vector<core::TileType> types = flatten(hmi::tileTaxonomy());

    EXPECT_EQ(types.size(), TILE_TYPE_COUNT) << "la taxonomie doit couvrir tous les types de tuiles";

    const std::set<core::TileType> unique(types.begin(), types.end());
    EXPECT_EQ(unique.size(), types.size()) << "aucun type ne doit apparaitre deux fois";
    EXPECT_EQ(unique.size(), TILE_TYPE_COUNT) << "aucun type ne doit manquer";
}

// Toute entrée porte un libellé non vide (affichage dans l'arbre).
TEST(TileTaxonomy, ChaqueEntreeAUnLibelle) {
    for (const hmi::TileCategory& category : hmi::tileTaxonomy()) {
        EXPECT_FALSE(category.label.empty());
        for (const hmi::TileEntry& entry : category.tiles) {
            EXPECT_FALSE(entry.label.empty());
        }
        for (const hmi::TileSubgroup& subgroup : category.subgroups) {
            EXPECT_FALSE(subgroup.label.empty());
            for (const hmi::TileEntry& entry : subgroup.tiles) {
                EXPECT_FALSE(entry.label.empty());
            }
        }
    }
}
