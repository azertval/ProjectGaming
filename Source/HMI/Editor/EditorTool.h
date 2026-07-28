#pragma once

/**
 * @file HMI/Editor/EditorTool.h
 * @brief Outil actif dans la grille de l'éditeur (LOT-15, `EX-EDIT-014`).
 */

namespace hmi {

/**
 * @brief Outil actif dans la grille de l'éditeur, changé via `Tab` ou le panneau `ToolPanel`.
 *
 * `Paint` peint case par case au clic/glisser (comportement LOT-14, inchangé). `Rectangle` peint
 * un rectangle entier au relâchement d'un glisser. `Selection` définit une zone (glisser) dont le
 * contenu peut être copié (`Ctrl+C`) puis collé ailleurs (`Ctrl+V`), sans peindre directement.
 * `Link` (`LOT-37`, `EX-IHM-030`) lie/délie un mécanisme : cliquer un déclencheur (interrupteur/
 * plaque) passe en attente de cible, cliquer une cible (porte/danger commuté) crée la liaison ;
 * refaire la même paire la supprime (bascule) — voir `hmi::resolveLinkClick`. `Échap` annule une
 * attente en cours (revient à aucune case sélectionnée), sans toucher au brouillon.
 */
enum class EditorTool { Paint, Rectangle, Selection, Link };

}  // namespace hmi
