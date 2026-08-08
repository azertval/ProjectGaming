#pragma once

#include <string>
#include <vector>

/**
 * @file HMI/Game/GameHud.h
 * @brief Choix du contenu de l'affichage tête haute : budgets restants, nom du tableau
 *        (`LOT-52` TACHE-03, `EX-IHM-003`).
 */

namespace core {
struct Player;
}

namespace hmi {

class Localization;

/**
 * @brief Lignes à afficher pour l'affichage tête haute d'un pas de simulation donné.
 *
 * Fonction **pure** (`EX-NFR-010`) : ne lit que ce qu'on lui passe, aucun accès GPU/fichier. C'est
 * ce qui rend le contenu du HUD assertable sans rendu — la brique de texte (`hmi::composeText`,
 * TACHE-02) n'a plus qu'à afficher ce que cette fonction décide.
 *
 * **Budget illimité, rien à afficher** : `core::Player::jumpsRemaining`/`dashesRemaining` valent
 * `-1` quand le niveau ne fixe pas de budget (`EX-GP-024`) — la grande majorité des tableaux. Un
 * compteur permanent affichant l'infini serait du bruit visuel sur tous ces tableaux ; la ligne
 * correspondante n'est donc produite que si le budget est **fini** (valeur `>= 0`).
 * @param player       Composant du personnage suivi (lu, jamais modifié — `EX-ARCH-012`).
 * @param levelName    Nom du tableau en cours (`core::Level::name`), affiché tel quel (donnée, pas
 *                     un libellé traduit).
 * @param localization Catalogue de traduction, pour les libellés « Sauts »/« Dashs » (`EX-REN-033`)
 *                     — aucune chaîne en dur, y compris les libellés courts.
 * @return Les lignes à afficher, dans l'ordre : budgets (sauts puis dashs, seulement s'ils sont
 *         finis), puis le nom du tableau.
 */
[[nodiscard]] std::vector<std::string> gameHudLines(const core::Player& player,
                                                     const std::string& levelName,
                                                     const Localization& localization);

}  // namespace hmi
