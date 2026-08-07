#include "HMI/Editor/TexturePanel.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIcon>
#include <QImage>
#include <QList>
#include <QModelIndex>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>

#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

#include "HMI/Editor/AssetReferences.h"
#include "HMI/Editor/AssetThumbnailView.h"
#include "HMI/Editor/SkinAssignments.h"
#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/Localization/Localization.h"
#include "ui_TexturePanel.h"

namespace hmi {

namespace {

// Colonnes de l'arbre : le type, le fichier assigne, et son mode de decoupage.
constexpr int COLUMN_TYPE = 0;
constexpr int COLUMN_ASSET = 1;
constexpr int COLUMN_MODE = 2;
constexpr int COLUMN_COUNT = 3;

// Role portant le core::TileType d'une ligne (les en-tetes de section n'en ont pas).
constexpr int TILE_TYPE_ROLE = Qt::UserRole + 1;

// Cote des vignettes affichees dans l'arbre, en pixels d'ecran (repere visuel, pas une vignette
// de selection -- le choix se fait dans AssetPickerDialog, a une taille plus grande).
constexpr int ROW_ICON_SIZE = 20;

// Valeur affichee pour « aucun skin assigne ». Traduite : c'est une entree de liste comme une
// autre pour l'utilisateur.
[[nodiscard]] QString noneLabel(const Localization* loc) {
    return loc == nullptr ? QStringLiteral("(aucun)")
                          : QString::fromStdString(loc->text("textures.none"));
}

// Texte localise d'une cle (repli sur la cle si aucun catalogue -- ne survient pas en pratique).
[[nodiscard]] QString t(const Localization* loc, const char* key) {
    return loc != nullptr ? QString::fromStdString(loc->text(key)) : QString::fromLatin1(key);
}

// Convertit des pixels RGBA decodes en QImage. Meme conversion que PalettePanel/
// AssetThumbnailView : trop petite pour justifier une fonction partagee entre trois widgets
// independants.
[[nodiscard]] QImage toImage(const DecodedImage& decoded) {
    QImage image(decoded.width, decoded.height, QImage::Format_RGBA8888);
    for (int y = 0; y < decoded.height; ++y) {
        const std::uint32_t* const row =
            decoded.pixels.data() + static_cast<std::size_t>(y) * decoded.width;
        std::memcpy(image.scanLine(y), row, static_cast<std::size_t>(decoded.width) * 4);
    }
    return image;
}

// Editeur en liste fermee pour la colonne Mode (deux choix fixes) : le seul delegue par defaut de
// Qt offrirait une saisie libre, incoherente avec des valeurs enumerees.
class ChoiceDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/,
                          const QModelIndex& index) const override {
        auto* const combo = new QComboBox(parent);
        combo->addItems(index.data(Qt::UserRole).toStringList());
        return combo;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (auto* const combo = qobject_cast<QComboBox*>(editor)) {
            combo->setCurrentText(index.data(Qt::EditRole).toString());
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        if (auto* const combo = qobject_cast<QComboBox*>(editor)) {
            model->setData(index, combo->currentText(), Qt::EditRole);
        }
    }
};

// Dialogue de selection d'un asset a vignettes (LOT-43 TACHE-01), enveloppe minimale autour du
// widget partage : ce panneau ne connait que les skins, donc configure AssetThumbnailView pour
// cette semantique (dossier, famille, verificateur de references) sans que le widget lui-meme en
// sache quoi que ce soit.
class AssetPickerDialog : public QDialog {
public:
    AssetPickerDialog(std::filesystem::path directory, const Localization* loc, QWidget* parent)
        : QDialog(parent), _view(new AssetThumbnailView(this)) {
        setWindowTitle(t(loc, "textures.pick_asset"));
        _view->setAssetFamily(AssetFamily::TileSkin);
        if (loc != nullptr) {
            _view->retranslateUi(*loc);
        }
        _view->setDirectory(std::move(directory));

        auto* const buttons =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(_view, &AssetThumbnailView::assetActivated, this, [this] { accept(); });

        auto* const layout = new QVBoxLayout(this);
        layout->addWidget(_view);
        layout->addWidget(buttons);
        resize(480, 360);
    }

    void setReferenceChecker(AssetThumbnailView::ReferenceChecker checker) {
        _view->setReferenceChecker(std::move(checker));
    }
    void selectAsset(const std::string& fileName) {
        _view->selectAsset(fileName);
    }
    [[nodiscard]] std::string selectedAsset() const {
        return _view->selectedAsset();
    }

private:
    AssetThumbnailView* _view;
};

}  // namespace

TexturePanel::TexturePanel(std::filesystem::path skinsDirectory, std::filesystem::path catalogPath,
                           QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::TexturePanel>()),
      _model(new QStandardItemModel(this)),
      _skinsDirectory(std::move(skinsDirectory)),
      _catalogPath(std::move(catalogPath)) {
    _ui->setupUi(this);
    _model->setColumnCount(COLUMN_COUNT);
    _ui->skinsTree->setModel(_model);
    _ui->skinsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Le mode se choisit dans une liste fermee (deux valeurs) ; le fichier se choisit desormais
    // par vignettes (LOT-43 TACHE-01), via un double-clic ouvrant AssetPickerDialog.
    _ui->skinsTree->setItemDelegateForColumn(COLUMN_MODE, new ChoiceDelegate(this));

    connect(_ui->setSelector, &QComboBox::currentIndexChanged, this, &TexturePanel::onSetChanged);
    connect(_model, &QStandardItemModel::itemChanged, this, &TexturePanel::onItemChanged);
    connect(_ui->skinsTree, &QTreeView::doubleClicked, this,
            &TexturePanel::onAssetColumnActivated);
    connect(_ui->reloadButton, &QPushButton::clicked, this,
            [this] { emit reloadRequested(); });
}

TexturePanel::~TexturePanel() = default;

void TexturePanel::setCatalog(SkinCatalog* catalog) {
    _catalog = catalog;
    _currentSet = catalog == nullptr ? std::string{} : catalog->defaultSetName();
    rebuildTree();
}

void TexturePanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->setLabel->setText(QString::fromStdString(loc.text("textures.skin_set")));
    _ui->reloadButton->setText(QString::fromStdString(loc.text("textures.reload")));
    _ui->reloadButton->setToolTip(QString::fromStdString(loc.text("textures.reload_tooltip")));
    _ui->sections->setTabText(0, QString::fromStdString(loc.text("textures.section_skins")));
    rebuildTree();
}

// Reconstruit entierement l'arbre depuis le catalogue et le contenu du dossier de skins.
void TexturePanel::rebuildTree() {
    // Garde de reentrance : peupler le modele emet itemChanged pour chaque cellule, ce qui
    // reecrirait le catalogue avec les valeurs qu'on vient d'y lire.
    _updating = true;
    _model->clear();
    _model->setColumnCount(COLUMN_COUNT);
    if (_loc != nullptr) {
        _model->setHorizontalHeaderLabels(
            QStringList{QString::fromStdString(_loc->text("textures.column_type")),
                        QString::fromStdString(_loc->text("textures.column_asset")),
                        QString::fromStdString(_loc->text("textures.column_mode"))});
    }

    // Selecteur de jeu, resynchronise sans emettre de changement de selection parasite.
    const QSignalBlocker blocker(_ui->setSelector);
    _ui->setSelector->clear();
    if (_catalog != nullptr) {
        for (const std::string& name : _catalog->setNames()) {
            _ui->setSelector->addItem(QString::fromStdString(name));
        }
        _ui->setSelector->setCurrentText(QString::fromStdString(_currentSet));
    }

    if (_catalog != nullptr) {
        for (const SkinSection& section : buildSkinRows(*_catalog, _currentSet)) {
            auto* const header =
                new QStandardItem(QString::fromStdString(localizedTaxonomyLabel(_loc, section.label)));
            header->setEditable(false);
            for (const SkinRow& row : section.rows) {
                auto* const typeItem = new QStandardItem(
                    QString::fromStdString(localizedTaxonomyLabel(_loc, row.typeLabel)));
                typeItem->setEditable(false);
                typeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                // Le fichier se choisit par vignettes (double-clic -> AssetPickerDialog), jamais en
                // saisie libre ni en liste texte (LOT-43 TACHE-01) : la cellule affiche le nom et
                // une vignette, mais n'est plus editable directement.
                auto* const assetItem = new QStandardItem(
                    row.asset.empty() ? noneLabel(_loc) : QString::fromStdString(row.asset));
                assetItem->setEditable(false);
                assetItem->setIcon(QIcon(thumbnailFor(row.asset)));
                assetItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                auto* const modeItem =
                    new QStandardItem(QString::fromLatin1(skinModeName(row.mode)));
                modeItem->setData(QStringList{QString::fromLatin1(skinModeName(SkinMode::Single)),
                                              QString::fromLatin1(
                                                  skinModeName(SkinMode::Bitmask16))},
                                  Qt::UserRole);
                modeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                header->appendRow(QList<QStandardItem*>{typeItem, assetItem, modeItem});
            }
            _model->appendRow(header);
        }
    }

    _ui->skinsTree->expandAll();
    _updating = false;
}

// Change le jeu de skins courant.
void TexturePanel::onSetChanged(int index) {
    if (_updating || _catalog == nullptr || index < 0) {
        return;
    }
    _currentSet = _ui->setSelector->itemText(index).toStdString();
    rebuildTree();
    emit assignmentsChanged();
}

// Applique la modification d'une cellule (fichier ou mode) au catalogue, puis l'enregistre.
void TexturePanel::onItemChanged(QStandardItem* item) {
    if (_updating || _catalog == nullptr || item == nullptr) {
        return;
    }
    const QVariant typeData = item->data(TILE_TYPE_ROLE);
    if (!typeData.isValid()) {
        return;  // en-tete de section : rien a assigner.
    }
    const auto type = static_cast<core::TileType>(typeData.toInt());

    // L'etat cible est lu sur la LIGNE entiere, pas sur la seule cellule modifiee : changer le mode
    // ne doit pas effacer le fichier, ni l'inverse.
    QStandardItem* const parent = item->parent();
    if (parent == nullptr) {
        return;
    }
    const int row = item->row();
    const QStandardItem* const assetItem = parent->child(row, COLUMN_ASSET);
    const QStandardItem* const modeItem = parent->child(row, COLUMN_MODE);
    if (assetItem == nullptr || modeItem == nullptr) {
        return;
    }

    const QString assetText = assetItem->text();
    const std::string asset = assetText == noneLabel(_loc) ? std::string{} : assetText.toStdString();
    const SkinMode mode =
        skinModeFromName(modeItem->text().toStdString()).value_or(SkinMode::Single);

    applySkinAssignment(*_catalog, _currentSet, type, asset, mode);
    save();
    emit assignmentsChanged();
}

// Ouvre le selecteur a vignettes sur un double-clic dans la colonne Fichier (LOT-43 TACHE-01).
// Le resultat est applique via _model->setData(..., Qt::EditRole), qui declenche itemChanged ->
// onItemChanged exactement comme l'ancien delegue en liste texte : aucun cablage supplementaire.
void TexturePanel::onAssetColumnActivated(const QModelIndex& index) {
    if (_catalog == nullptr || index.column() != COLUMN_ASSET) {
        return;
    }
    if (!index.data(TILE_TYPE_ROLE).isValid()) {
        return;  // en-tete de section : rien a choisir.
    }

    const QString currentText = index.data(Qt::DisplayRole).toString();
    const std::string currentAsset =
        currentText == noneLabel(_loc) ? std::string{} : currentText.toStdString();

    AssetPickerDialog dialog(_skinsDirectory, _loc, this);
    // Le panneau ne connait que les jeux de skins : c'est lui qui sait CE QUI peut referencer un
    // asset, le widget partage n'en sait rien (voir sa doc de classe).
    dialog.setReferenceChecker([this](const std::string& fileName) {
        std::vector<std::string> lines;
        for (const AssetReference& reference : findSkinCatalogReferences(*_catalog, fileName)) {
            lines.push_back(reference.setName + " (" + reference.typeName + ")");
        }
        return lines;
    });
    dialog.selectAsset(currentAsset);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const std::string chosen = dialog.selectedAsset();
    const QString displayText = chosen.empty() ? noneLabel(_loc) : QString::fromStdString(chosen);
    _model->setData(index, displayText, Qt::EditRole);
}

// Vignette d'un fichier de skin, decodee au premier besoin puis mise en cache par nom (dossier
// fixe pour ce panneau, _skinsDirectory).
QPixmap TexturePanel::thumbnailFor(const std::string& asset) {
    const auto cached = _thumbnails.find(asset);
    if (cached != _thumbnails.end()) {
        return cached->second;
    }

    QImage source;
    const std::optional<DecodedImage> decoded =
        asset.empty() ? std::nullopt : decodeImageFile(_skinsDirectory / asset);
    if (decoded) {
        source = toImage(*decoded);
    } else {
        // « Aucun » assigne ou fichier illisible : meme repli que le rendu (LOT-40), pour que la
        // ligne ne prenne pas silencieusement l'apparence d'un skin different.
        const ProceduralAtlasImage missing = buildMissingTextureImage();
        source = toImage(DecodedImage{missing.width, missing.height, missing.pixels});
    }

    const QPixmap pixmap = QPixmap::fromImage(source.scaled(
        ROW_ICON_SIZE, ROW_ICON_SIZE, Qt::KeepAspectRatio, Qt::FastTransformation));
    _thumbnails.emplace(asset, pixmap);
    return pixmap;
}

void TexturePanel::reloadAssets() {
    _thumbnails.clear();
    rebuildTree();
}

// Enregistre le catalogue au chemin deploye, comme l'enregistrement d'un niveau.
void TexturePanel::save() {
    if (_catalog == nullptr) {
        return;
    }
    if (!_catalog->saveToFile(_catalogPath)) {
        // Echec recuperable : l'assignation reste active en memoire et se voit a l'ecran, elle ne
        // survivra simplement pas au redemarrage.
        GRAPHICS_LOG_WARNING("Panneau Textures : enregistrement de " + _catalogPath.string() +
                             " impossible.");
    }
}

}  // namespace hmi
