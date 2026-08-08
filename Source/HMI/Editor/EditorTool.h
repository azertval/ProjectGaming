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
 * `TextureAssign` (`LOT-45`, `EX-EDIT-043`) assigne une texture par instance à une case : clic
 * gauche assigne/remplace l'asset sélectionné dans la bibliothèque, reclic du même asset ou clic
 * droit retire l'assignation — voir `hmi::resolveTextureAssignClick`.
 * `Decor` (`LOT-49`, `EX-DEC-001`, placement minimal — la manipulation complète relève de
 * `LOT-50`) pose un décor libre à la position **exacte** du clic (jamais calée sur la grille) :
 * clic gauche pose l'asset et la couche sélectionnés dans la bibliothèque de décors, clic droit ou
 * touche « Suppr » retire le décor le plus proche du curseur — voir `hmi::nearestDecorAt`.
 */
enum class EditorTool { Paint, Rectangle, Selection, Link, TextureAssign, Decor };

}  // namespace hmi
