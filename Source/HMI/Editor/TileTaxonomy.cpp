#include "HMI/Editor/TileTaxonomy.h"

namespace hmi {

std::vector<TileCategory> tileTaxonomy() {
    using core::TileType;
    return {
        {"Tuile",
         {{TileType::Empty, "Vide (gomme)"}, {TileType::Solid, "Plein"}},
         {
             {"Pente",
              {{TileType::SlopeUpRight, "Montée droite"},
               {TileType::SlopeUpLeft, "Montée gauche"},
               {TileType::SlopeDownRight, "Plafond droite"},
               {TileType::SlopeDownLeft, "Plafond gauche"}}},
             {"Arrondi",
              {{TileType::RoundedUpRight, "Montée droite"},
               {TileType::RoundedUpLeft, "Montée gauche"},
               {TileType::RoundedDownRight, "Plafond droite"},
               {TileType::RoundedDownLeft, "Plafond gauche"}}},
             {"Concave",
              {{TileType::ConcaveUpRight, "Montée droite"},
               {TileType::ConcaveUpLeft, "Montée gauche"},
               {TileType::ConcaveDownRight, "Plafond droite"},
               {TileType::ConcaveDownLeft, "Plafond gauche"}}},
             {"Bloc poussable",
              {{TileType::Block, "Plein"},
               {TileType::BlockHalf, "Demi (½)"},
               {TileType::BlockQuarter, "Quart (¼)"}}},
         }},
        {"Interactif",
         {{TileType::Switch, "Interrupteur"},
          {TileType::PressurePlate, "Plaque de pression"},
          {TileType::Door, "Porte"}},
         {}},
        {"Piège",
         {{TileType::Danger, "Danger"},
          {TileType::DangerMover, "Danger mobile"},
          {TileType::DangerSwitched, "Danger commuté"},
          {TileType::DangerBlink, "Danger clignotant"}},
         {
             {"Directionnel",
              {{TileType::DangerUp, "Pics vers le haut"},
               {TileType::DangerDown, "Pics vers le bas"},
               {TileType::DangerLeft, "Pics vers la gauche"},
               {TileType::DangerRight, "Pics vers la droite"}}},
         }},
        {"Jalon", {{TileType::Entry, "Entrée"}, {TileType::Exit, "Sortie"}}, {}},
    };
}

}  // namespace hmi
