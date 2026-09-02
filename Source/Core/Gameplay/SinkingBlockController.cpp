// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/SinkingBlockController.h"

#include <cmath>

#include "Core/Levels/TileType.h"

namespace core {

namespace {

// Duree d'un pas fixe : le jeu tourne a 60 pas/s (core::FixedTimestep), meme hypothese que
// core::PlatformController. En double, pour la meme raison : la distance parcourue est fonction
// d'un compteur de pas qui peut devenir grand, et un 1/60 arrondi en float y introduirait une
// derive systematique.
constexpr double FIXED_DELTA_SECONDS = 1.0 / 60.0;

// Deux boites se TOUCHENT (bords jointifs admis, a la tolerance pres) plutot que de se chevaucher
// strictement : se poser sur un bloc aligne exactement les bords, et un test de chevauchement
// d'aire strictement positive ne le verrait donc jamais.
[[nodiscard]] bool touches(const Aabb& a, const Aabb& b, float tolerance) {
    return a.min.x < b.max.x + tolerance && a.max.x > b.min.x - tolerance &&
           a.min.y < b.max.y + tolerance && a.max.y > b.min.y - tolerance;
}

}  // namespace

SinkingBlockController::SinkingBlockController(const Level& level)
    : _mapHeight(level.tileMap().height()) {
    const TileMap& map = level.tileMap();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            if (map.tile(column, row) != TileType::SinkingBlock) {
                continue;
            }
            SinkingBlock block;
            block.start = GridPosition{column, row};
            block.previousY = static_cast<float>(row);
            block.currentY = static_cast<float>(row);
            _blocks.push_back(block);
        }
    }
    refreshSamples();
}

float SinkingBlockController::stopLimitAt(std::size_t index, const TileMap& baseCollision,
                                          bool& hasLimit) const noexcept {
    // Premiere case PLEINE sous la case de depart : le bloc s'y arrete, son coin haut-gauche cale
    // juste au-dessus. On balaye depuis le depart et non depuis la position courante -- un bloc ne
    // pouvant jamais franchir la matiere, les deux donnent le meme resultat, et partir du depart
    // rend la borne independante de l'ordre des appels.
    const SinkingBlock& block = _blocks[index];
    for (int row = block.start.row + 1; row < baseCollision.height(); ++row) {
        if (baseCollision.isSolid(block.start.column, row)) {
            hasLimit = true;
            return static_cast<float>(row - 1);
        }
    }
    hasLimit = false;  // rien dessous : le bloc finira par sortir du tableau
    return 0.0F;
}

void SinkingBlockController::update(const Aabb& playerBox, const TileMap& baseCollision) {
    ++_stepCount;

    for (std::size_t index = 0; index < _blocks.size(); ++index) {
        SinkingBlock& block = _blocks[index];
        if (block.removed) {
            continue;
        }

        block.previousY = block.currentY;

        // 1. Armement : premier contact, par n'importe quelle face (EX-GP-027). Aller simple.
        if (!block.armed && touches(playerBox, boxAt(index), CONTACT_TOLERANCE)) {
            block.armed = true;
            block.armedStep = _stepCount;
        }
        if (!block.armed || block.stopped) {
            continue;  // immobile : un bloc non arme reste solide, un bloc arrete est fige
        }

        // 2. Descente : fonction PURE du nombre de pas ecoules depuis l'armement, jamais une
        //    accumulation `position += vitesse * dt` (EX-NFR-002).
        const double travelled =
            static_cast<double>(_stepCount - block.armedStep) *
            (static_cast<double>(SINK_SPEED_CELLS_PER_SECOND) * FIXED_DELTA_SECONDS);
        float target = static_cast<float>(static_cast<double>(block.start.row) + travelled);

        // 3. Arret contre la matiere, ou sortie par le bas du tableau.
        bool hasLimit = false;
        const float limit = stopLimitAt(index, baseCollision, hasLimit);
        if (hasLimit && target >= limit) {
            target = limit;
            block.stopped = true;  // definitif : ne repart pas si la case se libere
        } else if (!hasLimit && target >= static_cast<float>(_mapHeight)) {
            block.removed = true;
        }
        block.currentY = target;
    }

    refreshSamples();
}

Aabb SinkingBlockController::boxAt(std::size_t index) const noexcept {
    return Aabb::fromTopLeftSize(
        Vector2{static_cast<float>(_blocks[index].start.column), _blocks[index].currentY},
        Vector2{1.0F, 1.0F});
}

TileMap SinkingBlockController::collisionMap(const TileMap& base) const {
    TileMap map = base;
    // Deux passes : liberer TOUTES les cases de depart avant d'ecrire quoi que ce soit, sans quoi
    // un bloc descendu d'une case effacerait la case ou vient de se reporter le bloc du dessus.
    for (const SinkingBlock& block : _blocks) {
        map.setTile(block.start.column, block.start.row, TileType::Empty);
    }
    for (const SinkingBlock& block : _blocks) {
        if (block.removed) {
            continue;  // sorti par le bas : il n'est plus nulle part
        }
        const int row = static_cast<int>(std::lround(block.currentY));
        if (row < 0 || row >= map.height()) {
            continue;
        }
        if (map.tile(block.start.column, row) != TileType::Empty) {
            continue;  // matiere statique ou bloc poussable deja compose : priorite a l'existant
        }
        map.setTile(block.start.column, row, TileType::SinkingBlock);
    }
    return map;
}

void SinkingBlockController::refreshSamples() {
    _samples.clear();
    _samples.reserve(_blocks.size());
    for (const SinkingBlock& block : _blocks) {
        if (block.removed) {
            continue;  // un echantillon fantome continuerait de porter le personnage
        }
        const float column = static_cast<float>(block.start.column);
        _samples.push_back(PlatformSample{
            .previousBox =
                Aabb::fromTopLeftSize(Vector2{column, block.previousY}, Vector2{1.0F, 1.0F}),
            .currentBox =
                Aabb::fromTopLeftSize(Vector2{column, block.currentY}, Vector2{1.0F, 1.0F})});
    }
}

}  // namespace core
