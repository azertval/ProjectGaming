// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QWidget>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Levels/Decor.h"
#include "HMI/Editor/DecorListModel.h"
#include "HMI/Editor/EditorTool.h"

/**
 * @file HMI/Editor/DecorsPanel.h
 * @brief Panneau « Décors » : tout ce qui touche au placement et à l'inspection des décors,
 * regroupé
 *        (`LOT-57`, amendement post-essai manuel). Layout dans `DecorsPanel.ui`.
 */

class QEvent;
class QStandardItem;
class QStandardItemModel;

namespace Ui {
class DecorsPanel;
}

namespace core {
class LevelDraft;
}  // namespace core

namespace hmi {

class AssetThumbnailView;
class Localization;

/**
 * @brief Panneau « Décors », qui regroupe deux choses jusqu'ici dispersées :
 *        - le **placement** de décors (`LOT-49`) : grille de vignettes sur `Assets/Decors/`, couche
 *          du *prochain* décor posé, aimantation — visible seulement quand l'outil Décor est actif
 * ;
 *        - l'**inspection** des décors déjà posés (`LOT-50`), déplacée du panneau Textures : liste
 *          groupée par couche, réordonnancement, changement de couche du décor *sélectionné*,
 *          suppression, recentrage caméra — toujours visible, indépendamment de l'outil actif.
 *
 * L'ancien panneau « Outils » ne portait déjà plus que le strict nécessaire de l'outil Décor depuis
 * `LOT-56` TACHE-04 (le choix d'outil s'était déplacé vers la barre d'outils) : ce panneau reprend
 * ce contenu plutôt que d'en créer un troisième, et y ajoute ce que son nom promettait déjà. La
 * barre d'outils de l'éditeur (`hmi::EditorActions`), elle, reste hors de ce panneau — barre
 * globale de la fenêtre principale, pas propre au mode Décors.
 *
 * Les deux sélecteurs de couche ciblent des états **distincts** (prochain décor posé vs. décor
 * sélectionné existant) : ce n'est pas un doublon, chacun est le contrôle le plus proche de son
 * propre geste (`EX-IHM-062`).
 */
class DecorsPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construit le panneau.
     * @param decorsDirectory Dossier balayé pour peupler la grille de placement et résoudre les
     *                        vignettes de l'inspecteur (`LOT-49`/`LOT-50`).
     * @param parent          Widget parent.
     */
    explicit DecorsPanel(std::filesystem::path decorsDirectory, QWidget* parent = nullptr);
    ~DecorsPanel() override;

    /// Applique la langue active à tous les libellés du panneau.
    void retranslateUi(const Localization& loc);

    /**
     * @brief Affiche ou masque le sélecteur de placement selon l'outil actif.
     *
     * Appelé à chaque changement d'outil (barre d'outils ou touche dédiée de « Texture par
     * instance », via `GameViewport::toolChanged`) : seul l'outil Décor a besoin de ce sélecteur.
     * L'inspecteur, lui, reste toujours visible (même principe que les inspecteurs du panneau
     * Textures, `LOT-57` TACHE-03).
     */
    void setActiveTool(hmi::EditorTool tool);

    /**
     * @brief Resynchronise l'inspecteur avec le brouillon et la sélection courants (`LOT-50`
     *        TACHE-04, déplacé de `TexturePanel::refreshDecors`).
     *
     * À appeler après tout changement de brouillon **et** après tout changement de sélection venu
     * du canevas (`hmi::GameViewport`) — c'est ce qui tient la sélection croisée sans qu'aucune des
     * deux vues n'en possède sa propre copie.
     * @param draft         Brouillon dont on lit `decors()`.
     * @param selectedIndex Décor actuellement sélectionné (rang dans `draft.decors()`), si un
     *                      l'est.
     */
    void refreshDecors(const core::LevelDraft& draft, std::optional<std::size_t> selectedIndex);

    /**
     * @brief Recharge les vignettes de l'inspecteur après un rechargement à chaud (`LOT-43`
     *        TACHE-03, déplacé de `TexturePanel::reloadAssets`).
     *
     * Le contenu de `Assets/Decors/` a pu changer hors application : les vignettes sont redécodées,
     * le statut « asset manquant » se remet à jour au prochain `refreshDecors`.
     */
    void reloadDecorThumbnails();

protected:
    /// Régénère les vignettes de l'inspecteur lors d'un changement d'écran
    /// (`QEvent::ScreenChangeInternal`) : l'échelle d'affichage a pu changer (`LOT-56` TACHE-05,
    /// même patron que `TexturePanel::event`). La grille de placement (`AssetThumbnailView`) gère
    /// sa propre régénération, pas besoin de la relayer ici.
    bool event(QEvent* event) override;

signals:
    /// Émis quand l'utilisateur choisit un asset dans la grille de placement (`LOT-49`) : asset
    /// actif de l'outil Décor ; vide si aucun n'est sélectionné.
    void decorAssetSelected(const QString& fileName);
    /// Émis quand l'utilisateur choisit la couche du **prochain** décor posé (`LOT-49`,
    /// `EX-DEC-002`).
    void decorLayerSelected(core::DecorLayer layer);
    /// Émis quand l'utilisateur active/désactive l'aimantation sur la grille du geste de décors
    /// (`LOT-50` TACHE-02) — optionnelle et jamais imposée par défaut (`EX-DEC-001`).
    void decorSnapToGridChanged(bool enabled);
    /// Émis quand la sélection change dans l'inspecteur (surbrillance dans le canevas, `LOT-50`
    /// TACHE-04), ou `std::nullopt` si aucune ligne n'est sélectionnée.
    void decorSelected(std::optional<std::size_t> index);
    /// Émis par le bouton « Avancer » pour le décor sélectionné (`core::LevelDraft::
    /// bringDecorForward`).
    void decorForwardRequested(std::size_t index);
    /// Émis par le bouton « Reculer » pour le décor sélectionné (`core::LevelDraft::
    /// sendDecorBackward`).
    void decorBackwardRequested(std::size_t index);
    /// Émis quand l'utilisateur choisit une nouvelle couche pour le décor **sélectionné**
    /// (`core::LevelDraft::setDecorLayer`).
    void decorLayerChangeRequested(std::size_t index, core::DecorLayer layer);
    /// Émis par le bouton « Supprimer » pour le décor sélectionné.
    void decorRemoveRequested(std::size_t index);
    /// Émis par le bouton « Centrer » : demande de cadrer la caméra sur le décor sélectionné.
    void decorCenterRequested(std::size_t index);

private:
    /// Vignette d'un asset de décor, décodée au premier besoin puis mise en cache — repli en
    /// damier magenta si absent/illisible (cohérent avec le rendu, `LOT-40`).
    [[nodiscard]] QPixmap decorThumbnailFor(const std::string& asset);
    /// Libellé localisé d'une couche de décor, réutilisé par les deux sélecteurs.
    [[nodiscard]] QString decorLayerLabel(core::DecorLayer layer) const;
    /// Reconstruit le tableau de l'inspecteur depuis `_decorRows` (déjà groupées/triées par
    /// `refreshDecors`) et resélectionne `_selectedDecorIndex`, **sans** réémettre `decorSelected`
    /// (resynchronisation programmatique).
    void rebuildDecorRows();
    /// Active/désactive les boutons d'action et resynchronise le sélecteur de couche de
    /// l'inspecteur avec la ligne actuellement sélectionnée — factorisé entre `rebuildDecorRows`
    /// (silencieux) et `onDecorsSelectionChanged` (qui émet en plus `decorSelected`).
    void updateDecorActionButtons();
    /// Rang du décor porté par la ligne actuellement sélectionnée de l'inspecteur, ou
    /// `std::nullopt` si aucune sélection.
    [[nodiscard]] std::optional<std::size_t> selectedDecorRowIndex() const;
    void onDecorsSelectionChanged();
    void onDecorForwardClicked();
    void onDecorBackwardClicked();
    void onDecorLayerComboChanged(int index);
    void onDecorRemoveClicked();
    void onDecorCenterClicked();

    std::unique_ptr<Ui::DecorsPanel> _ui;
    AssetThumbnailView* _decorView;    ///< Grille de vignettes de placement (`Assets/Decors/`).
    QStandardItemModel* _decorsModel;  ///< Modèle du tableau de l'inspecteur.
    std::filesystem::path _decorsDirectory;
    /// Lignes affichées par l'inspecteur, déjà groupées/triées (`LOT-50` TACHE-04).
    std::vector<DecorListRow> _decorRows;
    /// Décor actuellement sélectionné (rang dans `core::LevelDraft::decors()`), poussé par
    /// `refreshDecors` — jamais possédé ici au sens propre, seulement reflété (source unique :
    /// `hmi::GameViewport`).
    std::optional<std::size_t> _selectedDecorIndex;
    /// Fichier de décor -> vignette décodée. Vidé par `reloadDecorThumbnails`.
    std::unordered_map<std::string, QPixmap> _decorThumbnails;
    const Localization* _loc = nullptr;  ///< Catalogue courant (nul avant première retraduction).
};

}  // namespace hmi
