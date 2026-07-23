#include "HMI/Editor/LevelSizeValidation.h"

#include <charconv>
#include <string_view>

namespace hmi {

namespace {

// Retire les espaces ASCII de bord.
std::string_view trimmedView(std::string_view text) {
    const std::size_t first = text.find_first_not_of(' ');
    if (first == std::string_view::npos) {
        return {};
    }
    const std::size_t last = text.find_last_not_of(' ');
    return text.substr(first, last - first + 1);
}

// Analyse un entier strictement compris entre 1 et MAX_LEVEL_DIMENSION inclus. std::from_chars
// (pas std::stoi) : jamais d'exception sur une entree invalide, un simple code d'erreur.
std::optional<int> parseDimension(std::string_view text) {
    const std::string_view trimmed = trimmedView(text);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    int value = 0;
    const auto result = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), value);
    if (result.ec != std::errc{} || result.ptr != trimmed.data() + trimmed.size()) {
        return std::nullopt;  // texte partiellement numerique (ex. "12abc") refuse
    }
    if (value < 1 || value > MAX_LEVEL_DIMENSION) {
        return std::nullopt;
    }
    return value;
}

}  // namespace

std::optional<std::pair<int, int>> parseLevelSize(const std::string& text) {
    // Separateur largeur/hauteur : x/X (historique) ou * (EX-EDIT-017, alternatif — ex. "60*40").
    const std::size_t separator = text.find_first_of("xX*");
    if (separator == std::string::npos) {
        return std::nullopt;
    }
    const std::optional<int> width = parseDimension(std::string_view(text).substr(0, separator));
    const std::optional<int> height =
        parseDimension(std::string_view(text).substr(separator + 1));
    if (!width || !height) {
        return std::nullopt;
    }
    return std::make_pair(*width, *height);
}

bool isValidLevelSize(const std::string& text) {
    return parseLevelSize(text).has_value();
}

}  // namespace hmi
