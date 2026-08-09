#pragma once

#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Ecs/Components/Animation.h"
#include "Core/Levels/Decor.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/DecorListModel.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/RenderLayer.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"

/**
 * @file HMI/Editor/TexturePanel.h
 * @brief Panneau « Textures » : jeu de skins courant et assignation par type (LOT-42).
 */

class QEvent;
class QModelIndex;
class QStandardItem;
class QStandardItemModel;
class QTimer;

namespace Ui {
class TexturePanel;
}

namespace core {
class LevelDraft;
}  // namespace core

namespace hmi {

class AssetThumbnailView;
class Localization;

/**
 * @brief Panneau d'habillage, organisé en **sections** extensibles (`EX-EDIT-042`, `EX-EDIT-024`).
 *
 * Premier — et unique — panneau du programme d'habillage : `LOT-44` (Fond), `LOT-45` (Objets),
 * `LOT-47` (Animations), `LOT-50` (Décors) et `LOT-51` (Calques) y ajoutent leur onglet plutôt que
 * de créer chacun leur dock. C'est la raison d'être de la structure en onglets dès ce lot : sans
 * elle, chaque lot suivant créerait le sien et l'éditeur finirait avec cinq panneaux d'habillage.
 *
 * Le panneau **ne possède pas** le catalogue : il agit sur celui que lui confie `setCatalog`, dont
 * `GameViewport` reste propriétaire — le rendu doit voir les mêmes assignations, immédiatement.
 * Toute modification est écrite sur disque (`skins.json`, au chemin déployé) et signalée par
 * `assignmentsChanged`, que le viewport et la palette consomment pour se rafraîchir.
 *
 * La logique alimentant l'arbre est **hors** du widget (`hmi::buildSkinRows`,
 * `hmi::applySkinAssignment`), donc testée sans Qt (`EX-NFR-010`). Le choix du fichier assigné à un
 * type se fait par vignettes (`hmi::AssetThumbnailView`, `LOT-43`), pas par saisie de nom.
 */
class TexturePanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Construit le panneau.
     * @param skinsDirectory       Dossier balayé pour peupler la liste des fichiers assignables.
     * @param catalogPath          Chemin d'écriture de `skins.json`.
     * @param backgroundsDirectory Dossier balayé pour peupler la grille de fonds (section « Fond »,
     *                             `LOT-44`).
     * @param objectsDirectory     Dossier balayé pour peupler la grille d'assets de la section
     *                             « Objets » (`LOT-45`).
     * @param decorsDirectory      Dossier balayé pour résoudre les vignettes de la section
     *                             « Décors » (`LOT-50` TACHE-04) et détecter les assets manquants.
     * @param parent               Widget parent.
     */
    explicit TexturePanel(std::filesystem::path skinsDirectory, std::filesystem::path catalogPath,
                          std::filesystem::path backgroundsDirectory,
                          std::filesystem::path objectsDirectory,
                          std::filesystem::path decorsDirectory, QWidget* parent = nullptr);
    ~TexturePanel() override;

    /**
     * @brief Confie au panneau le catalogue à éditer.
     * @param catalog Catalogue, non possédé, qui doit survivre au panneau.
     */
    void setCatalog(SkinCatalog* catalog);

    /**
     * @brief Synchronise la section « Fond » avec le niveau courant (`LOT-44`).
     *
     * À appeler après tout changement de brouillon (ouverture, undo/redo, chargement) pour que la
     * sélection affichée reste fidèle au niveau — l'inverse du sens habituel des signaux de ce
     * panneau (ici c'est l'appelant qui pousse l'état, la section ne le possède pas).
     * @param background Asset de fond du niveau courant (`core::LevelDraft::background`).
     * @param skinSet    Jeu de skins du niveau courant (`core::LevelDraft::skinSet`), distinct du
     *                   jeu courant d'édition (`currentSet`).
     */
    void setLevelProperties(const std::optional<std::string>& background,
                            const std::optional<std::string>& skinSet);

    /// @return Le nom du jeu de skins actuellement sélectionné.
    [[nodiscard]] const std::string& currentSet() const noexcept {
        return _currentSet;
    }

    /// Applique la langue active (titre des sections, en-têtes de colonnes, libellés de types).
    void retranslateUi(const Localization& loc);

    /**
     * @brief Recharge l'arbre depuis le disque après un rechargement à chaud (`LOT-43` TACHE-03).
     *
     * Vide le cache de vignettes des lignes (un fichier a pu être modifié hors de l'application)
     * puis reconstruit l'arbre : le catalogue lui-même est déjà à jour (rechargé en place par
     * l'appelant, `GameViewport::reloadAssets`), seuls le balayage du dossier et les vignettes
     * doivent être refaits.
     */
    void reloadAssets();

    /**
     * @brief Resynchronise la section « Objets » avec le brouillon courant (`LOT-45`).
     *
     * À appeler après tout changement de brouillon (peinture, undo/redo, chargement), comme
     * `LinkPanel::refresh` : reconstruit la liste des surcharges depuis @p draft, sans en garder de
     * copie entre deux appels.
     * @param draft Brouillon dont on lit `textureOverrides()`.
     */
    void refreshObjects(const core::LevelDraft& draft);

    /**
     * @brief Resynchronise la section « Décors » avec le brouillon et la sélection courants
     *        (`LOT-50` TACHE-04).
     *
     * À appeler après tout changement de brouillon (comme `refreshObjects`) **et** après tout
     * changement de sélection venu du canevas (`hmi::GameViewport`) — c'est ce qui tient la
     * sélection croisée sans qu'aucune des deux vues n'en possède sa propre copie
     * (« ne pas dupliquer l'état de sélection », TACHE-04).
     * @param draft         Brouillon dont on lit `decors()`.
     * @param selectedIndex Décor actuellement sélectionné (rang dans `draft.decors()`), si un
     *                      l'est.
     */
    void refreshDecors(const core::LevelDraft& draft, std::optional<std::size_t> selectedIndex);

    /**
     * @brief Resynchronise la case « Physique seul » de la section « Calques » (`LOT-51`) avec le
     *        mode de rendu courant, sans réémettre `physiqueOnlyToggled` — même garde que
     *        `setLevelProperties`. À appeler quand le mode change par un autre moyen que ce
     *        panneau (`F8`), pour que les deux entrées du même état ne divergent jamais.
     * @param mode Mode de rendu courant (`hmi::GameViewport::renderMode`).
     */
    void setRenderModeIndicator(RenderMode mode);

protected:
    /// Régénère les vignettes lors d'un changement d'écran (`QEvent::ScreenChangeInternal`) :
    /// l'échelle d'affichage a pu changer (`LOT-56` TACHE-05).
    bool event(QEvent* event) override;

signals:
    /// Émis après toute modification d'assignation ou changement de jeu courant, une fois le
    /// catalogue à jour — le rendu et la palette n'ont qu'à se redessiner.
    void assignmentsChanged();
    /// Émis quand l'utilisateur demande un rechargement à chaud des textures (`LOT-43` TACHE-03).
    void reloadRequested();
    /// Émis quand l'utilisateur choisit un fond dans la section « Fond » (`LOT-44`) : nom de
    /// l'asset, vide pour « aucun fond ».
    void backgroundChanged(const QString& fileName);
    /// Émis quand l'utilisateur choisit le jeu de skins **du niveau** (`LOT-44`), distinct du jeu
    /// courant d'édition : nom du jeu, vide pour « jeu par défaut ».
    void levelSkinSetChanged(const QString& setName);
    /// Émis quand l'utilisateur choisit un asset dans la grille « Objets » (`LOT-45`) : asset actif
    /// de l'outil « Texture par instance » ; vide si aucun n'est sélectionné.
    void textureOverrideAssetSelected(const QString& fileName);
    /// Émis quand la sélection change dans la liste des surcharges du niveau (surbrillance dans le
    /// viewport), ou `std::nullopt` si aucune ligne n'est sélectionnée.
    void textureOverrideSelectionChanged(std::optional<core::GridPosition> position);
    /// Émis par le bouton « Retirer » de la section « Objets » pour la ligne sélectionnée.
    void textureOverrideRemoveRequested(core::GridPosition position);
    /// Émis quand la sélection change dans la liste « Décors » (surbrillance dans le canevas,
    /// `LOT-50` TACHE-04), ou `std::nullopt` si aucune ligne n'est sélectionnée.
    void decorSelected(std::optional<std::size_t> index);
    /// Émis par le bouton « Avancer » pour le décor sélectionné (`hmi::LevelDraft::
    /// bringDecorForward`).
    void decorForwardRequested(std::size_t index);
    /// Émis par le bouton « Reculer » pour le décor sélectionné (`hmi::LevelDraft::
    /// sendDecorBackward`).
    void decorBackwardRequested(std::size_t index);
    /// Émis quand l'utilisateur choisit une nouvelle couche pour le décor sélectionné
    /// (`core::LevelDraft::setDecorLayer`).
    void decorLayerChangeRequested(std::size_t index, core::DecorLayer layer);
    /// Émis par le bouton « Supprimer » pour le décor sélectionné.
    void decorRemoveRequested(std::size_t index);
    /// Émis par le bouton « Centrer » : demande de cadrer la caméra sur le décor sélectionné.
    void decorCenterRequested(std::size_t index);

    /// Émis quand une case de la section « Calques » change (`LOT-51`, `EX-EDIT-044`) : calque
    /// concerné et nouvelle visibilité — mode d'inspection éditeur, sans effet sur `GameSession`.
    void layerVisibilityChanged(RenderLayer layer, bool visible);
    /// Émis par le bouton « Tout afficher » de la section « Calques » (`LOT-51` TACHE-03).
    void showAllLayersRequested();
    /// Émis quand la case « Physique seul » de la section « Calques » change : demande de bascule
    /// du mode de rendu, **la même bascule que `F8`** vue depuis ce panneau (`LOT-51`) — pas un
    /// troisième mode.
    void physiqueOnlyToggled(bool enabled);

private:
    void rebuildTree();
    void onSetChanged(int index);
    void onItemChanged(QStandardItem* item);
    void onAssetColumnActivated(const QModelIndex& index);
    void save();
    /// Vignette d'un fichier de skin (vide pour « aucun »), décodée au premier besoin puis mise en
    /// cache — repli en damier magenta si absent/illisible (cohérent avec le rendu, `LOT-40`).
    [[nodiscard]] QPixmap thumbnailFor(const std::string& asset);
    /// Reconstruit la liste du sélecteur de jeu de skins **du niveau** (section « Fond »), à partir
    /// des jeux du catalogue, plus l'entrée « jeu par défaut ».
    void rebuildLevelSkinSetSelector();
    /// Reconstruit le tableau des surcharges depuis `_objectRows` (tri stable par position, déjà
    /// posé par `refreshObjects`).
    void rebuildObjectRows();
    void onObjectsSelectionChanged();
    void onObjectsRemoveClicked();
    /// Reconstruit l'arbre de la section « Animations » (`LOT-47` TACHE-04) depuis le catalogue et
    /// le diagnostic de clips manquants (`hmi::buildMechanismAnimationRows`).
    void rebuildAnimationsTree();
    /// Ouvre le selecteur a vignettes sur un double-clic dans la colonne Fichier de l'arbre
    /// « Animations » (meme patron que `onAssetColumnActivated`, un mecanisme est un type de tuile
    /// comme un autre pour `hmi::SkinCatalog`).
    void onAnimationsAssetActivated(const QModelIndex& index);
    /// Charge l'apercu (spritesheet decodee + description d'animation) de la ligne selectionnee.
    void onAnimationsSelectionChanged();
    /// Avance l'horloge d'apercu d'un pas de minuterie et rafraichit la vignette (`LOT-47`
    /// TACHE-04) : lecture en temps reel du meme catalogue que le jeu (`hmi::AnimationCatalog`),
    /// pas une reimplementation parallele.
    void tickAnimationPreview();
    /// Vignette d'un asset de décor, décodée au premier besoin puis mise en cache (`LOT-50`
    /// TACHE-04) — même repli en damier magenta que `thumbnailFor` si absent/illisible.
    [[nodiscard]] QPixmap decorThumbnailFor(const std::string& asset);
    /// Libellé localisé d'une couche de décor (mêmes trois libellés que le sélecteur de l'outil
    /// Décor, `ToolPanel`).
    [[nodiscard]] QString decorLayerLabel(core::DecorLayer layer) const;
    /// Reconstruit le tableau de la section « Décors » depuis `_decorRows` (déjà groupées/triées
    /// par `refreshDecors`) et resélectionne `_selectedDecorIndex`, **sans** réémettre
    /// `decorSelected` (resynchronisation programmatique, même garde que `setLevelProperties`).
    void rebuildDecorRows();
    /// Active/désactive les boutons d'action et resynchronise le sélecteur de couche avec la ligne
    /// actuellement sélectionnée dans le tableau — factorisé entre `rebuildDecorRows` (silencieux)
    /// et `onDecorsSelectionChanged` (qui émet en plus `decorSelected`).
    void updateDecorActionButtons();
    /// Rang du décor porté par la ligne actuellement sélectionnée du tableau « Décors » (donnée
    /// stockée sur l'item lors de `rebuildDecorRows`), ou `std::nullopt` si aucune sélection.
    [[nodiscard]] std::optional<std::size_t> selectedDecorRowIndex() const;
    void onDecorsSelectionChanged();
    void onDecorForwardClicked();
    void onDecorBackwardClicked();
    void onDecorLayerComboChanged(int index);
    void onDecorRemoveClicked();
    void onDecorCenterClicked();

    std::unique_ptr<Ui::TexturePanel> _ui;
    QStandardItemModel* _model;
    AssetThumbnailView* _backgroundView;  ///< Grille de vignettes de `Assets/Backgrounds/` (LOT-44).
    AssetThumbnailView* _objectView;      ///< Grille de vignettes de `Assets/Objects/` (LOT-45).
    QStandardItemModel* _objectsModel;    ///< Modèle du tableau des surcharges (LOT-45).
    QStandardItemModel* _animationsModel;  ///< Modèle de l'arbre de la section « Animations » (LOT-47).
    QTimer* _animationPreviewTimer;        ///< Fait avancer l'aperçu de la section « Animations ».
    /// Spritesheet décodée de l'asset actuellement prévisualisé (vide si aucune sélection/asset
    /// illisible) — décodée une fois par sélection, pas à chaque image de la minuterie.
    QImage _animationPreviewSheet;
    /// Description d'animation de l'asset prévisualisé, ou `std::nullopt` si absente/invalide/non
    /// sélectionnée : l'aperçu reste alors sur l'image entière de `_animationPreviewSheet`.
    std::optional<AnimationDescription> _animationPreviewDescription;
    /// Horloge d'aperçu, avancée par `_animationPreviewTimer` (temps réel, comme l'aperçu
    /// d'édition de `hmi::DraftRenderer` — la détermination n'a pas de sens hors simulation).
    core::Animation _animationPreviewAnimation;
    QStandardItemModel* _decorsModel;  ///< Modèle du tableau de la section « Décors » (`LOT-50`).
    std::filesystem::path _skinsDirectory;
    std::filesystem::path _catalogPath;
    std::filesystem::path _backgroundsDirectory;
    std::filesystem::path _objectsDirectory;
    std::filesystem::path _decorsDirectory;
    /// Surcharges affichées par le tableau, indexées comme ses lignes (`LOT-45`).
    std::vector<core::TileTextureOverride> _objectRows;
    /// Lignes affichées par le tableau « Décors », déjà groupées/triées (`LOT-50` TACHE-04).
    std::vector<DecorListRow> _decorRows;
    /// Décor actuellement sélectionné (rang dans `core::LevelDraft::decors()`), poussé par
    /// `refreshDecors` — jamais possédé ici au sens propre, seulement reflété (source unique :
    /// `hmi::GameViewport`).
    std::optional<std::size_t> _selectedDecorIndex;
    /// Fichier de décor -> vignette décodée (`LOT-50`). Vidé par `reloadAssets`.
    std::unordered_map<std::string, QPixmap> _decorThumbnails;
    SkinCatalog* _catalog = nullptr;  ///< Non possédé (propriété de `GameViewport`).
    std::string _currentSet;
    /// Dernières valeurs poussées par `setLevelProperties` (LOT-44) : reappliquées après tout
    /// rechargement/reconstruction qui perdrait la sélection courante des widgets (grille, combo).
    std::optional<std::string> _levelBackground;
    std::optional<std::string> _levelSkinSet;
    bool _updating = false;              ///< Garde de réentrance pendant la reconstruction du modèle.
    const Localization* _loc = nullptr;  ///< Catalogue courant (nul avant première retraduction).
    /// Fichier de skin (vide pour « aucun ») -> vignette décodée. Vidé par `reloadAssets`.
    std::unordered_map<std::string, QPixmap> _thumbnails;
};

}  // namespace hmi
