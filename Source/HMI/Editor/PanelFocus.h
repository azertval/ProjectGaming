#pragma once

#include <array>
#include <optional>

#include "HMI/Editor/EditorTool.h"

/**
 * @file HMI/Editor/PanelFocus.h
 * @brief Correspondance outil actif → panneau à mettre en avant (`LOT-57` TACHE-02, `EX-IHM-061`).
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) —
 * même patron que `HMI/Interface/ActionCatalog.h`. `MainWindow` ne fait que suivre cette table :
 * aucune condition écrite en dur sur un outil particulier.
 */

namespace hmi {

/// Panneau de droite pouvant être mis en avant (regroupés en onglets, TACHE-02).
enum class PanelId { Levels, Links, Textures };

/// Une entrée de la table : l'outil @p tool met en avant le panneau @p panel.
struct PanelFocusEntry {
    EditorTool tool;
    PanelId panel;
};

/// Nombre d'entrées de la table (tous les outils n'ont pas de panneau dédié).
constexpr int PANEL_FOCUS_CATALOG_COUNT = 2;

/// @return La table complète outil → panneau.
[[nodiscard]] const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT>& panelFocusCatalog();

/// @return Le panneau à mettre en avant pour @p tool, ou `std::nullopt` si cet outil n'en a pas
///         (ses contrôles vivent ailleurs, ex. le panneau Outils pour l'outil Décor).
[[nodiscard]] std::optional<PanelId> panelForTool(EditorTool tool);

}  // namespace hmi
