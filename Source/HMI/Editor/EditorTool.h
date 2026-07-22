#pragma once

/**
 * @file HMI/Editor/EditorTool.h
 * @brief Outil actif dans la grille de l'éditeur (LOT-15, `EX-EDIT-014`).
 */

namespace hmi {

/**
 * @brief Outil actif dans la grille de l'éditeur, changé via `Tab` ou la `ToolBar`.
 *
 * `Paint` peint case par case au clic/glisser (comportement LOT-14, inchangé). `Rectangle` peint
 * un rectangle entier au relâchement d'un glisser. `Selection` définit une zone (glisser) dont le
 * contenu peut être copié (`Ctrl+C`) puis collé ailleurs (`Ctrl+V`), sans peindre directement. La
 * liaison de mécanismes (`Maj`+clic) reste disponible quel que soit l'outil actif.
 */
enum class EditorTool { Paint, Rectangle, Selection };

}  // namespace hmi
