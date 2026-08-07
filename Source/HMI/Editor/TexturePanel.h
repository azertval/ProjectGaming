#pragma once

#include <QPixmap>
#include <QWidget>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/SkinCatalog.h"

/**
 * @file HMI/Editor/TexturePanel.h
 * @brief Panneau « Textures » : jeu de skins courant et assignation par type (LOT-42).
 */

class QModelIndex;
class QStandardItem;
class QStandardItemModel;

namespace Ui {
class TexturePanel;
}

namespace hmi {

class Localization;

/**
 * @brief Panneau d'habillage, organisé en **sections** extensibles (`EX-EDIT-042`, `EX-EDIT-024`).
 *
 * Premier — et unique — panneau du programme d'habillage : `LOT-44` (Fond), `LOT-45` (Objets),
 * `LOT-47` (Animations) et `LOT-50` (Décors) y ajouteront leur onglet plutôt que de créer chacun
 * leur dock. C'est la raison d'être de la structure en onglets dès ce lot : sans elle, chaque lot
 * suivant créerait le sien et l'éditeur finirait avec cinq panneaux d'habillage.
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
     * @param skinsDirectory Dossier balayé pour peupler la liste des fichiers assignables.
     * @param catalogPath    Chemin d'écriture de `skins.json`.
     * @param parent         Widget parent.
     */
    explicit TexturePanel(std::filesystem::path skinsDirectory, std::filesystem::path catalogPath,
                          QWidget* parent = nullptr);
    ~TexturePanel() override;

    /**
     * @brief Confie au panneau le catalogue à éditer.
     * @param catalog Catalogue, non possédé, qui doit survivre au panneau.
     */
    void setCatalog(SkinCatalog* catalog);

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

signals:
    /// Émis après toute modification d'assignation ou changement de jeu courant, une fois le
    /// catalogue à jour — le rendu et la palette n'ont qu'à se redessiner.
    void assignmentsChanged();
    /// Émis quand l'utilisateur demande un rechargement à chaud des textures (`LOT-43` TACHE-03).
    void reloadRequested();

private:
    void rebuildTree();
    void onSetChanged(int index);
    void onItemChanged(QStandardItem* item);
    void onAssetColumnActivated(const QModelIndex& index);
    void save();
    /// Vignette d'un fichier de skin (vide pour « aucun »), décodée au premier besoin puis mise en
    /// cache — repli en damier magenta si absent/illisible (cohérent avec le rendu, `LOT-40`).
    [[nodiscard]] QPixmap thumbnailFor(const std::string& asset);

    std::unique_ptr<Ui::TexturePanel> _ui;
    QStandardItemModel* _model;
    std::filesystem::path _skinsDirectory;
    std::filesystem::path _catalogPath;
    SkinCatalog* _catalog = nullptr;  ///< Non possédé (propriété de `GameViewport`).
    std::string _currentSet;
    bool _updating = false;              ///< Garde de réentrance pendant la reconstruction du modèle.
    const Localization* _loc = nullptr;  ///< Catalogue courant (nul avant première retraduction).
    /// Fichier de skin (vide pour « aucun ») -> vignette décodée. Vidé par `reloadAssets`.
    std::unordered_map<std::string, QPixmap> _thumbnails;
};

}  // namespace hmi
