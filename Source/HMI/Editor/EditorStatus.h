#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "HMI/Editor/EditorTool.h"

/**
 * @file HMI/Editor/EditorStatus.h
 * @brief Choix du contenu de la barre d'état de l'éditeur : zones permanentes et aide
 *        contextuelle à l'outil actif (`LOT-57` TACHE-01, `EX-IHM-060`).
 */

namespace hmi {

class Localization;

/// État affiché pour un niveau en cours d'édition (barre d'état, `EX-EDIT-013`/`EX-EDIT-012`).
struct LevelStatusInfo {
    std::string name;                                  ///< Nom du niveau ouvert.
    bool dirty = false;                                ///< Modifications non enregistrées.
    EditorTool tool = EditorTool::Paint;                ///< Outil d'édition actif.
    std::optional<core::GridPosition> hoveredCell;      ///< Case survolée, si le curseur est dessus.
    float zoom = 1.0f;                                  ///< Facteur de zoom courant.
};

/**
 * @brief Contexte d'édition dont la barre d'état décide l'affichage.
 *
 * `level` est le seul contexte livré aujourd'hui ; un futur atelier pixel art (`LOT-54`) y ajoutera
 * un second contexte (édition d'un asset) sans changer cette forme — d'où la structure englobante
 * plutôt qu'un simple `LevelStatusInfo` passé directement.
 */
struct EditorStatusContext {
    std::optional<LevelStatusInfo> level;
};

/// Lignes à afficher pour la barre d'état de l'éditeur, à un instant donné.
struct EditorStatusLines {
    /// Zones permanentes, dans l'ordre d'affichage : niveau, modifications non enregistrées, outil
    /// actif, case survolée, zoom. Une zone vide (chaîne vide) quand l'information n'a pas de sens
    /// pour le contexte courant — jamais de libellé de remplacement.
    std::vector<std::string> permanent;
    /// Aide contextuelle à l'outil actif ; vide hors contexte de niveau.
    std::string help;
};

/**
 * @brief Décide le contenu de la barre d'état de l'éditeur pour @p context.
 *
 * Fonction **pure** (`EX-NFR-010`) : ne lit que ce qu'on lui passe, aucune dépendance Qt/GPU — même
 * patron que `hmi::gameHudLines` (`LOT-52`). Le rendu (`MainWindow`) n'a plus qu'à afficher ce que
 * cette fonction décide, et à la rappeler après l'expiration d'un message transitoire pour restaurer
 * l'aide affichée.
 * @param context      Contexte d'édition courant ; `context.level` absent (aucun niveau ouvert)
 *                      produit des zones et une aide vides.
 * @param localization Catalogue de traduction (`EX-REN-033`) — aucune chaîne en dur.
 * @return Les zones permanentes et l'aide contextuelle à afficher.
 */
[[nodiscard]] EditorStatusLines editorStatusLines(const EditorStatusContext& context,
                                                   const Localization& localization);

}  // namespace hmi
