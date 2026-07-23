#pragma once

#include <optional>
#include <string>
#include <utility>

/**
 * @file HMI/Editor/LevelSizeValidation.h
 * @brief Analyse et validation d'une taille de grille saisie dans l'éditeur (LOT-16, `EX-EDIT-017`).
 */

namespace hmi {

/**
 * @brief Plafond de taille par axe (largeur, hauteur), en cases.
 *
 * Garde-fou d'usage porté par `HMI`, pas une limite de `Core` (`TileMap`/`LevelDraft` acceptent
 * toute dimension positive, `EX-NFR-010`) : évite qu'une saisie erronée ou des flèches maintenues
 * trop longtemps ne produisent une grille démesurée. Très au-delà des tailles livrées à ce jour
 * (12x8 à 14x8) ; ajustable ici sans toucher au modèle de niveau.
 */
constexpr int MAX_LEVEL_DIMENSION = 100;

/**
 * @brief Analyse un texte au format « largeur x hauteur » (ex. `40x30`, ou `40*30`).
 *
 * Séparateur `x`/`X`/`*`, espaces tolérés autour de chaque valeur. Chaque dimension doit être un
 * entier compris entre 1 et `MAX_LEVEL_DIMENSION` inclus.
 * @param text Texte saisi.
 * @return La paire (largeur, hauteur) si le texte est bien formé et dans les bornes ; sinon vide.
 */
[[nodiscard]] std::optional<std::pair<int, int>> parseLevelSize(const std::string& text);

/**
 * @brief Indique si @p text peut être confirmé comme taille de grille.
 * @param text Texte saisi.
 * @return `true` si `parseLevelSize(text)` réussit — sert directement de validateur à
 *         `TextInputField`.
 */
[[nodiscard]] bool isValidLevelSize(const std::string& text);

}  // namespace hmi
