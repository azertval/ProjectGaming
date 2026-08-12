/**
 * @file test_tile_visuals.cpp
 * @brief Tests unitaires de la correspondance type de tuile -> région d'atlas procédurale
 *        (`hmi::regionForTile`), pour le repli sans asset (`EX-NFR-040`).
 */

#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

// Deux regions sont "distinctes" si elles ne pointent pas exactement la meme case de la grille
// procedurale (memes coordonnees x/y) -- deux types partageant une case seraient indiscernables en
// repli procedural (LOT-63, TACHE-04).
bool sameRegion(const core::AtlasRegion& a, const core::AtlasRegion& b) {
    return a.x == b.x && a.y == b.y;
}

}  // namespace

/**
 * @brief Les trois nouveaux mécanismes du `LOT-63` (clé, porte verrouillée, plateforme mobile)
 *        occupent chacun une case distincte de la grille procédurale, et distincte des mécanismes
 *        existants les plus proches (interrupteur, porte, bloc).
 * \castest{<b>Clé, porte verrouillée et plateforme mobile ont des régions procédurales
 * distinctes.</b><br/>
 * \tcat Unitaire · Tile Visuals<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre la région procédurale de chaque type concerné.<br/>2. Comparer chaque paire.
 * <br/>
 * \tattendu Aucune paire ne partage la même région.
 * }
 */
TEST(TileVisualsTest, NouveauxMecanismesLOT63OntDesRegionsDistinctes) {
    const std::vector<core::TileType> types = {
        core::TileType::Key,    core::TileType::LockedDoor,    core::TileType::MovingPlatform,
        core::TileType::Switch, core::TileType::PressurePlate, core::TileType::Door,
        core::TileType::Block,
    };

    for (std::size_t i = 0; i < types.size(); ++i) {
        for (std::size_t j = i + 1; j < types.size(); ++j) {
            EXPECT_FALSE(sameRegion(hmi::regionForTile(types[i]), hmi::regionForTile(types[j])))
                << "types " << static_cast<int>(types[i]) << " et " << static_cast<int>(types[j])
                << " partagent la meme region procedurale";
        }
    }
}
