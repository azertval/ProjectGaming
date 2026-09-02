// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/VolatileBlockController.h"

#include "Core/Gameplay/PlatformController.h"
#include "Core/Levels/TileType.h"

namespace core {

namespace {

// Portee verticale d'un ground pound pour briser un bloc fragile, en cases. La boite du personnage
// est celle d'AVANT le pas (voir update) et la chute imposee vaut 30 unites/s, soit 0,5 case par
// pas fixe : ses pieds peuvent donc etre encore une demi-case au-dessus du bloc au moment ou l'on
// decide. Une case entiere couvre ce cas avec de la marge, sans jamais atteindre un bloc situe un
// etage plus bas.
constexpr float POUND_REACH_CELLS = 1.0F;

// Chevauchement horizontal strict (aire non nulle sur l'axe X).
[[nodiscard]] bool overlapsHorizontally(const Aabb& a, const Aabb& b) {
    return a.min.x < b.max.x && a.max.x > b.min.x;
}

[[nodiscard]] Aabb boxOf(GridPosition position) {
    return Aabb::fromTopLeftSize(
        Vector2{static_cast<float>(position.column), static_cast<float>(position.row)},
        Vector2{1.0F, 1.0F});
}

}  // namespace

VolatileBlockController::VolatileBlockController(const Level& level) {
    const TileMap& map = level.tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const TileType type = map.tile(column, row);
            if (type != TileType::FragileBlock && type != TileType::VanishingBlock) {
                continue;
            }
            VolatileBlock block;
            block.position = GridPosition{column, row};
            block.fragile = (type == TileType::FragileBlock);
            _blocks.push_back(block);
        }
    }
}

void VolatileBlockController::update(const Aabb& playerBox, bool groundPounding) {
    _goneThisStep.clear();

    for (VolatileBlock& block : _blocks) {
        if (block.gone) {
            continue;
        }
        const Aabb box = boxOf(block.position);

        if (block.fragile) {
            // EX-GP-028 : seul un ground pound brise, et seulement PAR LE DESSUS. Un dash, meme
            // vertical ou booste, n'a aucun effet -- la restriction est deliberee (voir
            // l'exigence).
            if (!groundPounding) {
                continue;
            }
            const float feet = playerBox.max.y;
            const bool fromAbove =
                feet <= box.min.y + REST_TOLERANCE && feet >= box.min.y - POUND_REACH_CELLS;
            if (overlapsHorizontally(playerBox, box) && fromAbove) {
                block.gone = true;
                _goneThisStep.push_back(block.position);
            }
            continue;
        }

        // EX-GP-029 : bloc ephemere. Le declencheur est le FRONT « reposait dessus -> n'y repose
        // plus » (jamais un contact : passer dessous ne doit rien armer), le moteur ne conservant
        // aucun etat de contact persistant -- meme patron que hmi::detectPlayerEvents.
        const bool rests = restsOnTopOfPlatform(playerBox, box);
        if (block.countdown > 0) {
            --block.countdown;
            if (block.countdown == 0) {
                block.gone = true;
                _goneThisStep.push_back(block.position);
            }
        } else if (block.wasRested && !rests) {
            block.countdown = VANISH_DELAY_STEPS;  // revenir dessus ne l'annulera pas
        }
        block.wasRested = rests;
    }
}

TileMap VolatileBlockController::collisionMap(const TileMap& base) const {
    TileMap map = base;
    for (const VolatileBlock& block : _blocks) {
        if (block.gone) {
            // Soustraction pure : un bloc intact est deja solide via core::isSolid, il n'y a donc
            // rien a AJOUTER ici -- seulement a retirer ce qui a disparu.
            map.setTile(block.position.column, block.position.row, TileType::Empty);
        }
    }
    return map;
}

}  // namespace core
