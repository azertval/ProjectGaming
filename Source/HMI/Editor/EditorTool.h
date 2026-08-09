#pragma once

/**
 * @file HMI/Editor/EditorTool.h
 * @brief Outil actif dans la grille de l'éditeur (LOT-15, `EX-EDIT-014`).
 */

namespace hmi {

/**
 * @brief Outil actif dans la grille de l'éditeur, changé via `Tab` ou la barre d'outils
 *        (`hmi::EditorActions`, panneau `DecorsPanel`).
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
 * `Decor` (`LOT-49`/`LOT-50`, `EX-DEC-001`, `EX-DEC-010`) place, sélectionne, déplace,
 * redimensionne, pivote et supprime des décors libres, jamais calés sur la grille : cliquer un
 * décor le sélectionne et arme un glisser (corps = déplacer, coin = redimensionner, poignée dédiée
 * = pivoter, `hmi::DecorGesture`) ; cliquer une zone vide pose l'asset/la couche sélectionnés dans
 * la bibliothèque de décors (aucun décor sous le curseur, aucune sélection) ; clic droit ou touche
 * « Suppr » retire le décor visé/sélectionné ; `Échap` abandonne un glisser en cours sans y
 * toucher.
 */
enum class EditorTool { Paint, Rectangle, Selection, Link, TextureAssign, Decor };

}  // namespace hmi
