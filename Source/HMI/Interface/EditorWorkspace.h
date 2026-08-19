#pragma once

#include <cstddef>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/PixelTool.h"

/**
 * @file HMI/Interface/EditorWorkspace.h
 * @brief Espaces de travail exclusifs de l'éditeur (`LOT-68`, `EX-IHM-073`).
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) —
 * même patron que `hmi::dressingFor` (`ScreenFlow.h`) et `hmi::panelForTool` (`PanelFocus.h`).
 * `MainWindow` ne fait que suivre ces tables : aucune condition écrite en dur sur un dock ou une
 * barre d'outils en particulier.
 *
 * L'éditeur affichait jusqu'ici ses **neuf** docks et ses **deux** barres d'outils simultanément,
 * soit une trentaine de contrôles permanents, alors que l'édition de niveau et l'atelier pixel art
 * ne se pratiquent jamais en même temps : les deux tiers de l'interface étaient hors sujet à tout
 * instant.
 *
 * À ne pas confondre avec la **mise en avant** de `EX-IHM-061`, qui est une suggestion et ne masque
 * jamais rien. Changer d'espace est un acte explicite de l'utilisateur, et c'est ce qui rend le
 * masquage légitime ici.
 */

namespace hmi {

/// Espace de travail actif. Les deux s'excluent : aucun état intermédiaire n'existe.
enum class EditorWorkspace {
    Level,     ///< Édition de niveau.
    PixelArt,  ///< Atelier pixel art.
};

/// Nombre d'espaces, déclaré au plus près de l'énumération qu'il compte.
inline constexpr std::size_t EDITOR_WORKSPACE_COUNT = 2;

/// Ce qu'un espace affiche, hors panneaux (voir `workspaceForPanel`). Même patron que
/// `hmi::ScreenDressing` : une table décide, la fenêtre applique.
struct WorkspaceDressing {
    bool levelToolBarVisible = false;
    bool pixelToolBarVisible = false;
    bool workshopMenuVisible = false;

    [[nodiscard]] friend bool operator==(const WorkspaceDressing&,
                                         const WorkspaceDressing&) noexcept = default;
};

/// @return Les barres et menus visibles dans @p workspace. Jamais les deux barres d'outils à la
/// fois : c'est la propriété que le lot existe pour établir.
[[nodiscard]] WorkspaceDressing dressingForWorkspace(EditorWorkspace workspace) noexcept;

/// @return L'espace auquel appartient @p panel. **Total** : chaque panneau appartient à exactement
/// un espace, sans quoi il resterait affiché dans les deux.
[[nodiscard]] EditorWorkspace workspaceForPanel(PanelId panel) noexcept;

/// @return L'espace auquel appartient @p tool — toujours `Level`, les deux groupes d'outils étant
/// disjoints par construction. Existe pour que `MainWindow` bascule d'espace en suivant une table
/// plutôt qu'une condition écrite en dur.
[[nodiscard]] EditorWorkspace workspaceForTool(EditorTool tool) noexcept;

/// @return L'espace auquel appartient @p tool — toujours `PixelArt`.
[[nodiscard]] EditorWorkspace workspaceForPixelTool(PixelTool tool) noexcept;

}  // namespace hmi
