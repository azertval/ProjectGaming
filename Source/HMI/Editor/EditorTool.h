#pragma once

#include <cstddef>

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
 * `CameraZone` (`LOT-64`, `EX-LVL-007`, `EX-EDIT-029`) dessine une zone de caméra du mode *par
 * salle* : un glisser (comme `Rectangle`) définit un rectangle de cases, ajouté à
 * `core::CameraFramingConfig::zones` au relâchement (`core::LevelDraft::addCameraZone`) ; les
 * zones existantes se retirent depuis le tableau de la section « Cadrage » du panneau Textures,
 * pas depuis le canevas.
 * `Path` (`LOT-67`, `EX-EDIT-032`) manipule la trajectoire des éléments mobiles : cliquer la case
 * d'une plateforme mobile ou d'un danger mobile sélectionne son parcours et en affiche les
 * poignées ; glisser une poignée de point le déplace sur la case visée ; glisser le losange au
 * milieu d'un segment y **insère** un point et le déplace du même geste ; clic droit sur une
 * poignée retire ce point ; `Échap` abandonne un glisser en cours sans toucher au brouillon — voir
 * `hmi::PathGesture`. Le point de **départ** n'a pas de poignée : c'est la tuile elle-même, qu'on
 * déplace en la repeignant. Pour un danger mobile, l'unique poignée d'extrémité redéfinit à la
 * fois l'axe et la portée. Vitesse, déphasage et mode de bouclage se règlent dans le panneau
 * Propriétés, pas au canevas.
 */
enum class EditorTool { Paint, Rectangle, Selection, Link, TextureAssign, Decor, CameraZone, Path };

/// Nombre d'outils d'édition, déclaré au plus près de l'énumération qu'il compte. Sert de garde
/// de **complétude** : le catalogue d'actions doit exposer exactement autant d'outils de niveau
/// (`hmi::editorActionCatalog`), ce qu'un test vérifie — sans quoi un outil ajouté à
/// l'énumération peut apparaître dans la barre d'outils sans jamais être relié au viewport.
inline constexpr std::size_t EDITOR_TOOL_COUNT = 8;

}  // namespace hmi
