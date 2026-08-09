#pragma once

#include <QObject>
#include <array>

#include "HMI/Interface/ActionCatalog.h"

class QAction;
class QActionGroup;
class QToolBar;

/**
 * @file HMI/Interface/EditorActions.h
 * @brief Actions Qt de l'éditeur : outils et commandes principales (`LOT-56` TACHE-04).
 */

namespace hmi {

class Localization;
struct DesignTokens;

/**
 * @brief Construit et possède les `QAction` du catalogue (`HMI/Interface/ActionCatalog.h`) :
 *        chaque outil et chaque commande principale n'existe qu'une fois, placée simultanément
 *        dans la barre d'outils, un menu et un menu contextuel sans duplication.
 *
 * Les icônes sont dessinées par code (`hmi::themeIcon`) et recolorées depuis les jetons ;
 * `refreshIcons` les régénère lors d'un changement de thème (`LOT-56` TACHE-06).
 */
class EditorActions : public QObject {
    Q_OBJECT

public:
    /// @param tokens Jetons du châssis d'édition, pour la première génération des icônes.
    explicit EditorActions(const DesignTokens& tokens, QObject* parent = nullptr);

    /// @return L'action portant @p id.
    [[nodiscard]] QAction* action(IconId id) const;
    /// @return L'action de l'outil @p tool (raccourci vers `action(editorActionForTool(tool))`).
    [[nodiscard]] QAction* toolAction(EditorTool tool) const;
    /// @return Le groupe exclusif des six actions d'outil.
    [[nodiscard]] QActionGroup* toolGroup() const noexcept {
        return _toolGroup;
    }

    /// Ajoute toutes les actions à @p toolBar, dans l'ordre du catalogue, avec un séparateur entre
    /// les outils et les commandes principales.
    void populateToolBar(QToolBar& toolBar) const;

    /// Applique la langue active : libellé de chaque action, et infobulle incluant son raccourci
    /// (jamais saisi séparément).
    void retranslateUi(const Localization& loc);

    /// Coche l'action de l'outil actif **sans** émettre `triggered` : resynchronisation depuis la
    /// touche dédiée de `GameViewport` (remappable, `EditorAction::TextureAssignTool`), même garde
    /// que `ToolPanel::setActiveTool`.
    void setActiveTool(EditorTool tool);

    /// Active/désactive les six commandes qui n'ont de sens qu'en édition (Enregistrer, Essayer,
    /// Annuler, Refaire, Grille, Recadrer). Le mode de rendu reste toujours actif : il s'applique
    /// aussi en essai et en jeu réel (`EX-REN-046`).
    void setEditingCommandsEnabled(bool enabled);

    /// Reconstruit les icônes depuis un nouveau jeu de jetons (bascule de thème, `LOT-56`
    /// TACHE-06) : sans quoi elles resteraient aux anciennes couleurs après le changement.
    void refreshIcons(const DesignTokens& tokens);

private:
    std::array<QAction*, EDITOR_ACTION_CATALOG_COUNT> _actions{};
    QActionGroup* _toolGroup;
};

}  // namespace hmi
