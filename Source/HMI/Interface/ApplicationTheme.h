#pragma once

#include <QPalette>

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

/// Applique la palette du châssis d'édition (portée **variable**) comme palette par défaut de
/// l'application. À appeler avant la construction de `MainWindow`.
void applyEditorTheme();

}  // namespace hmi
