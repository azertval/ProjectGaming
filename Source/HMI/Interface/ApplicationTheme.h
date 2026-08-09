#pragma once

#include <QPalette>
#include <string>
#include <unordered_map>

/**
 * @file HMI/Interface/ApplicationTheme.h
 * @brief Application du système de design de l'IHM Qt (`LOT-56`) : style, palettes et thème.
 *
 * Couche **Qt** au-dessus des jetons purs (`HMI/Interface/DesignTokens.h`) : construit la
 * `QPalette` de l'application et choisit son style, avant la création du moindre widget
 * (`EX-IHM-050`). Non compilée dans `UnitTests` (dépend de Qt), à l'inverse de `DesignTokens.cpp`.
 */

namespace hmi {

struct DesignTokens;

/// Choisit explicitement le style Qt de l'application (non natif, condition pour que la palette et
/// la feuille de style s'appliquent à l'ensemble des contrôles). À appeler **avant** la création du
/// moindre widget : un style appliqué après ne se propage pas aux widgets déjà construits.
void applyApplicationStyle();

/// Construit une `QPalette` complète — groupes actif, inactif et désactivé — à partir d'un jeu de
/// jetons. Le groupe désactivé est explicitement dérivé du texte atténué : c'est celui qu'on
/// oublie le plus souvent, et celui qui trahit le plus vite un thème incomplet.
[[nodiscard]] QPalette buildApplicationPalette(const DesignTokens& tokens);

/// Construit la table de substitution `${...}` -> valeur pour un jeu de jetons du châssis
/// d'édition (portée variable) ; les marqueurs `${identity.*}` sont toujours résolus depuis
/// `identityTokens()` (portée invariante, jamais affectée par le thème de l'éditeur).
[[nodiscard]] std::unordered_map<std::string, std::string> buildStyleSheetValues(
    const DesignTokens& editorTokens);

/// Charge le modèle de feuille de style embarqué (`:/resources/theme.qss`), le substitue avec
/// @p editorTokens et l'applique à l'application. Repli explicite : fichier absent/illisible ou
/// marqueur inconnu -> avertissement journalisé, l'application reste utilisable sans feuille de
/// style (comportement historique de `main.cpp`, non régressé).
void applyStyleSheet(const DesignTokens& editorTokens);

/// Enregistre la police embarquée (`Assets/Fonts/Inter-{Regular,Bold}.ttf`) auprès de Qt et
/// l'applique comme police par défaut de l'application. Repli explicite si le fichier est absent
/// ou refusé par Qt : famille **générique** demandée à Qt (`QFont::StyleHint`), jamais un second
/// nom de police codé en dur ; avertissement journalisé.
void applyFont();

/// Applique le thème complet du châssis d'édition (portée **variable**) : police, palette puis
/// feuille de style, tous dérivés du même jeu de jetons. À appeler avant la construction de
/// `MainWindow`.
void applyEditorTheme();

}  // namespace hmi
