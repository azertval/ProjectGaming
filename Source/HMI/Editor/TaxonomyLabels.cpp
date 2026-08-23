// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/TaxonomyLabels.h"

#include <unordered_map>

#include "HMI/Localization/Localization.h"

namespace hmi {

std::string taxonomyLabelKey(const std::string& label) {
    static const std::unordered_map<std::string, std::string> keys = {
        {"Tuile", "palette.cat.tile"},
        {"Interactif", "palette.cat.interactive"},
        {"Piège", "palette.cat.trap"},
        {"Jalon", "palette.cat.marker"},
        {"Pente", "palette.sub.slope"},
        {"Arrondi", "palette.sub.rounded"},
        {"Concave", "palette.sub.concave"},
        {"Bloc poussable", "palette.sub.block"},
        {"Directionnel", "palette.sub.directional"},
        {"Vide (gomme)", "palette.tile.empty"},
        {"Plein", "palette.tile.solid"},
        {"Montée droite", "palette.tile.rise_right"},
        {"Montée gauche", "palette.tile.rise_left"},
        {"Plafond droite", "palette.tile.ceiling_right"},
        {"Plafond gauche", "palette.tile.ceiling_left"},
        {"Demi (½)", "palette.tile.half"},
        {"Quart (¼)", "palette.tile.quarter"},
        {"Interrupteur", "palette.tile.switch"},
        {"Plaque de pression", "palette.tile.plate"},
        {"Porte", "palette.tile.door"},
        {"Clé", "palette.tile.key"},
        {"Porte verrouillée", "palette.tile.locked_door"},
        {"Plateforme mobile", "palette.tile.moving_platform"},
        {"Danger", "palette.tile.danger"},
        {"Danger mobile", "palette.tile.danger_moving"},
        {"Danger commuté", "palette.tile.danger_switched"},
        {"Danger clignotant", "palette.tile.danger_blink"},
        {"Pics vers le haut", "palette.tile.spikes_up"},
        {"Pics vers le bas", "palette.tile.spikes_down"},
        {"Pics vers la gauche", "palette.tile.spikes_left"},
        {"Pics vers la droite", "palette.tile.spikes_right"},
        {"Entrée", "palette.tile.entry"},
        {"Sortie", "palette.tile.exit"},
    };

    const auto found = keys.find(label);
    return found == keys.end() ? std::string{} : found->second;
}

std::string localizedTaxonomyLabel(const Localization* loc, const std::string& label) {
    if (loc == nullptr) {
        return label;
    }
    const std::string key = taxonomyLabelKey(label);
    if (key.empty()) {
        return label;  // repli : libelle source (francais).
    }
    return loc->text(key);
}

}  // namespace hmi
