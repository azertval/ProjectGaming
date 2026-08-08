#include "HMI/Game/GameHud.h"

#include "Core/Ecs/Components/Player.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Remplace la premiere occurrence de "%1" par value (pas de dependance Qt : GameSession/GameHud
// restent utilisables hors interface, meme discipline que le reste de Core/Game).
std::string formatCount(const std::string& templateText, int value) {
    const std::string placeholder = "%1";
    const std::size_t position = templateText.find(placeholder);
    if (position == std::string::npos) {
        return templateText;
    }
    std::string result = templateText;
    result.replace(position, placeholder.size(), std::to_string(value));
    return result;
}

}  // namespace

// Lignes a afficher pour l'affichage tete haute d'un pas de simulation donne (voir en-tete).
std::vector<std::string> gameHudLines(const core::Player& player, const std::string& levelName,
                                      const Localization& localization) {
    std::vector<std::string> lines;

    // Budget fini seulement (EX-GP-024) : -1 = illimite, la majorite des tableaux, pas de ligne.
    if (player.jumpsRemaining >= 0) {
        lines.push_back(
            formatCount(localization.text("hud.jumps_remaining"), player.jumpsRemaining));
    }
    if (player.dashesRemaining >= 0) {
        lines.push_back(
            formatCount(localization.text("hud.dashes_remaining"), player.dashesRemaining));
    }

    lines.push_back(levelName);
    return lines;
}

}  // namespace hmi
