#pragma once

#include <array>
#include <optional>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Interface/IconGeometry.h"

/**
 * @file HMI/Interface/ActionCatalog.h
 * @brief Catalogue des actions de l'éditeur : outils et commandes principales (`LOT-56` TACHE-04,
 *        `EX-IHM-055`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `ProjectGaming` et directement dans `UnitTests`. La
 * construction Qt des `QAction` (`HMI/Interface/EditorActions.h`) est produite depuis ce
 * catalogue, pour qu'aucune commande n'ait deux définitions (menu, barre d'outils, raccourci).
 */

namespace hmi {

/// Groupe d'exclusivité d'une action : les six outils d'édition forment un groupe **exclusif**
/// (un seul actif à la fois) ; les commandes n'appartiennent à aucun groupe.
enum class EditorActionGroup { None, LevelTools };

/// Description d'une action, indépendante de Qt : de quoi construire un `QAction` complet (icône,
/// libellé, raccourci, caractère cochable) sans dupliquer sa définition ailleurs.
struct EditorActionSpec {
    IconId id;
    const char* labelKey;
    /// Séquence de raccourci au format `QKeySequence` (ex. `"Ctrl+S"`), chaîne vide si aucun.
    const char* shortcut;
    bool checkable;
    EditorActionGroup group;
};

/// Nombre total d'actions du catalogue (six outils, sept commandes principales).
constexpr int EDITOR_ACTION_CATALOG_COUNT = 13;

/// @return Le catalogue complet, dans l'ordre d'affichage voulu de la barre d'outils : les six
///         outils (ordre de la palette/du panneau Outils historique), puis les sept commandes.
[[nodiscard]] const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& editorActionCatalog();

/// @return La spécification de l'action @p id.
[[nodiscard]] const EditorActionSpec& editorActionSpec(IconId id);

/// @return L'outil associé à l'action @p id, si elle appartient au groupe `LevelTools` ;
///         `std::nullopt` pour une commande.
[[nodiscard]] std::optional<EditorTool> editorActionTool(IconId id);

/// @return L'identifiant d'action portant l'outil @p tool.
[[nodiscard]] IconId editorActionForTool(EditorTool tool);

}  // namespace hmi
