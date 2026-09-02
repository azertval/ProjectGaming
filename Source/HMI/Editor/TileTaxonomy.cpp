// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/TileTaxonomy.h"

namespace hmi {

std::vector<TileCategory> tileTaxonomy() {
    using core::TileType;
    return {
        {.label = "Tuile",
         .tiles = {{.type = TileType::Empty, .label = "Vide (gomme)"},
                   {.type = TileType::Solid, .label = "Plein"}},
         .subgroups =
             {
                 {.label = "Pente",
                  .tiles = {{.type = TileType::SlopeUpRight, .label = "Montée droite"},
                            {.type = TileType::SlopeUpLeft, .label = "Montée gauche"},
                            {.type = TileType::SlopeDownRight, .label = "Plafond droite"},
                            {.type = TileType::SlopeDownLeft, .label = "Plafond gauche"}}},
                 {.label = "Arrondi",
                  .tiles = {{.type = TileType::RoundedUpRight, .label = "Montée droite"},
                            {.type = TileType::RoundedUpLeft, .label = "Montée gauche"},
                            {.type = TileType::RoundedDownRight, .label = "Plafond droite"},
                            {.type = TileType::RoundedDownLeft, .label = "Plafond gauche"}}},
                 {.label = "Concave",
                  .tiles = {{.type = TileType::ConcaveUpRight, .label = "Montée droite"},
                            {.type = TileType::ConcaveUpLeft, .label = "Montée gauche"},
                            {.type = TileType::ConcaveDownRight, .label = "Plafond droite"},
                            {.type = TileType::ConcaveDownLeft, .label = "Plafond gauche"}}},
                 {.label = "Bloc poussable",
                  .tiles = {{.type = TileType::Block, .label = "Plein"},
                            {.type = TileType::BlockHalf, .label = "Demi (½)"},
                            {.type = TileType::BlockQuarter, .label = "Quart (¼)"}}},
                 {.label = "Bloc volatil",
                  .tiles = {{.type = TileType::SinkingBlock, .label = "Descendant"},
                            {.type = TileType::FragileBlock, .label = "Fragile"},
                            {.type = TileType::VanishingBlock, .label = "Éphémère"}}},
             }},
        {.label = "Interactif",
         .tiles = {{.type = TileType::Switch, .label = "Interrupteur"},
                   {.type = TileType::PressurePlate, .label = "Plaque de pression"},
                   {.type = TileType::Door, .label = "Porte"},
                   {.type = TileType::Key, .label = "Clé"},
                   {.type = TileType::LockedDoor, .label = "Porte verrouillée"},
                   {.type = TileType::MovingPlatform, .label = "Plateforme mobile"}},
         .subgroups = {}},
        {.label = "Piège",
         .tiles = {{.type = TileType::Danger, .label = "Danger"},
                   {.type = TileType::DangerMover, .label = "Danger mobile"},
                   {.type = TileType::DangerSwitched, .label = "Danger commuté"},
                   {.type = TileType::DangerBlink, .label = "Danger clignotant"}},
         .subgroups =
             {
                 {.label = "Directionnel",
                  .tiles = {{.type = TileType::DangerUp, .label = "Pics vers le haut"},
                            {.type = TileType::DangerDown, .label = "Pics vers le bas"},
                            {.type = TileType::DangerLeft, .label = "Pics vers la gauche"},
                            {.type = TileType::DangerRight, .label = "Pics vers la droite"}}},
             }},
        {.label = "Jalon",
         .tiles = {{.type = TileType::Entry, .label = "Entrée"},
                   {.type = TileType::Exit, .label = "Sortie"}},
         .subgroups = {}},
    };
}

}  // namespace hmi
