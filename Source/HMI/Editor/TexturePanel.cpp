#include "HMI/Editor/TexturePanel.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIcon>
#include <QImage>
#include <QItemSelectionModel>
#include <QList>
#include <QModelIndex>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Core/Ecs/AnimationClip.h"
#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Levels/LevelDraft.h"
#include "HMI/Editor/AssetLibrary.h"
#include "HMI/Editor/AssetReferences.h"
#include "HMI/Editor/AssetThumbnailView.h"
#include "HMI/Editor/DecorListModel.h"
#include "HMI/Editor/MechanismAnimationAssignments.h"
#include "HMI/Editor/SkinAssignments.h"
#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/MechanismVisuals.h"
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

// Colonnes du tableau des surcharges de la section « Objets » (LOT-45) : position et asset.
constexpr int OBJECTS_COLUMN_POSITION = 0;
constexpr int OBJECTS_COLUMN_ASSET = 1;
constexpr int OBJECTS_COLUMN_COUNT = 2;

// Colonnes du tableau de la section « Décors » (LOT-50 TACHE-04) : vignette+asset, couche.
constexpr int DECORS_COLUMN_ASSET = 0;
constexpr int DECORS_COLUMN_LAYER = 1;
constexpr int DECORS_COLUMN_COUNT = 2;

// Ordre des entrees du selecteur de couche de la section « Decors », identique a celui de
// ToolPanel (LOT-49) : Arriere-plan, Decor, Premier plan.
constexpr core::DecorLayer DECOR_LAYER_ORDER[] = {
    core::DecorLayer::Background, core::DecorLayer::Decor, core::DecorLayer::Foreground};

// Rang de core::DecorLayer dans DECOR_LAYER_ORDER (index du selecteur de couche).
int decorLayerComboIndex(core::DecorLayer layer) noexcept {
    switch (layer) {
        case core::DecorLayer::Background:
            return 0;
        case core::DecorLayer::Decor:
            return 1;
        case core::DecorLayer::Foreground:
            return 2;
    }
    return 1;
}

// Colonnes de l'arbre de la section « Animations » (LOT-47) : famille de mecanisme, fichier
// assigne, diagnostic des clips manquants.
constexpr int ANIMATIONS_COLUMN_TYPE = 0;
constexpr int ANIMATIONS_COLUMN_ASSET = 1;
constexpr int ANIMATIONS_COLUMN_DIAGNOSTIC = 2;
constexpr int ANIMATIONS_COLUMN_COUNT = 3;

// Intervalle de la minuterie d'apercu (LOT-47 TACHE-04) : temps reel (comme l'apercu d'edition de
// hmi::DraftRenderer), pas la determination du pas fixe -- suffisant pour "verifier le rythme"
// sans entrer en mode essai.
constexpr int ANIMATION_PREVIEW_INTERVAL_MS = 66;
// Facteur d'agrandissement de la vignette d'apercu (une case de 16x16 px serait illisible telle
// quelle dans le panneau).
constexpr int ANIMATION_PREVIEW_SCALE = 4;

// Libelle "(colonne, ligne)" d'une position de grille -- meme format que LinkPanel.cpp.
QString positionText(core::GridPosition position) {
    return QStringLiteral("(%1, %2)").arg(position.column).arg(position.row);
}

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

// Valeur affichee pour « jeu de skins par defaut » dans le selecteur du NIVEAU (section Fond,
// LOT-44) -- distincte de noneLabel : une chaine vide designe ici le jeu par defaut du catalogue,
// pas une absence d'assignation.
[[nodiscard]] QString defaultSetLabel(const Localization* loc) {
    return loc == nullptr ? QStringLiteral("(jeu par defaut)")
                          : QString::fromStdString(loc->text("textures.default_set"));
}

// Texte localise d'une cle (repli sur la cle si aucun catalogue -- ne survient pas en pratique).
[[nodiscard]] QString t(const Localization* loc, const char* key) {
    return loc != nullptr ? QString::fromStdString(loc->text(key)) : QString::fromLatin1(key);
}

// Texte de diagnostic d'une ligne de la section « Animations » (LOT-47) : vide sans asset assigne
// -- rien a diagnostiquer -- « complet » si tous les clips attendus sont presents, sinon la liste
// des clips manquants.
[[nodiscard]] QString diagnosticText(const Localization* loc, const MechanismAnimationRow& row) {
    if (row.asset.empty()) {
        return QString();
    }
    if (row.missingClips.empty()) {
        return t(loc, "textures.animations_complete");
    }
    QStringList missing;
    for (const std::string& clipName : row.missingClips) {
        missing << QString::fromStdString(clipName);
    }
    return t(loc, "textures.animations_missing") + QStringLiteral(" : ") + missing.join(", ");
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
                           std::filesystem::path backgroundsDirectory,
                           std::filesystem::path objectsDirectory,
                           std::filesystem::path decorsDirectory, QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::TexturePanel>()),
      _model(new QStandardItemModel(this)),
      _backgroundView(nullptr),
      _objectView(nullptr),
      _objectsModel(new QStandardItemModel(0, OBJECTS_COLUMN_COUNT, this)),
      _animationsModel(new QStandardItemModel(0, ANIMATIONS_COLUMN_COUNT, this)),
      _animationPreviewTimer(new QTimer(this)),
      _decorsModel(new QStandardItemModel(0, DECORS_COLUMN_COUNT, this)),
      _skinsDirectory(std::move(skinsDirectory)),
      _catalogPath(std::move(catalogPath)),
      _backgroundsDirectory(std::move(backgroundsDirectory)),
      _objectsDirectory(std::move(objectsDirectory)),
      _decorsDirectory(std::move(decorsDirectory)) {
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

    // Section « Fond » (LOT-44) : grille de vignettes embarquee directement (pas un dialogue de
    // selection comme pour les skins) -- selectionner une case choisit le fond immediatement, avec
    // les memes controles d'import/renommage/suppression que la bibliotheque de skins (LOT-43).
    _backgroundView = new AssetThumbnailView(this);
    _backgroundView->setAssetFamily(AssetFamily::Background);
    _backgroundView->setDirectory(_backgroundsDirectory);
    _ui->backgroundContainerLayout->addWidget(_backgroundView);
    connect(_backgroundView, &AssetThumbnailView::assetSelected, this,
            [this](const QString& fileName) { emit backgroundChanged(fileName); });
    connect(_ui->levelSkinSetSelector, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0) {
            return;
        }
        const QString text = _ui->levelSkinSetSelector->itemText(index);
        emit levelSkinSetChanged(text == defaultSetLabel(_loc) ? QString() : text);
    });

    // Section « Objets » (LOT-45) : grille de vignettes pour choisir l'asset actif de l'outil
    // « Texture par instance » (aucune entree "(aucun)" -- selectionner une case vide n'a pas de
    // sens, on choisit toujours un asset avant d'assigner), plus le tableau des surcharges deja
    // posees sur le niveau, avec surbrillance croisee et retrait.
    _objectView = new AssetThumbnailView(this);
    _objectView->setAssetFamily(AssetFamily::Object);
    _objectView->setNoneOptionVisible(false);
    _objectView->setDirectory(_objectsDirectory);
    _ui->objectPickerContainerLayout->addWidget(_objectView);
    connect(_objectView, &AssetThumbnailView::assetSelected, this,
            [this](const QString& fileName) { emit textureOverrideAssetSelected(fileName); });

    _ui->objectsTable->setModel(_objectsModel);
    _ui->objectsTable->horizontalHeader()->setStretchLastSection(true);
    _ui->objectsTable->verticalHeader()->setVisible(false);
    connect(_ui->objectsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { onObjectsSelectionChanged(); });
    connect(_ui->objectsRemoveButton, &QPushButton::clicked, this,
            [this] { onObjectsRemoveClicked(); });

    // Section « Animations » (LOT-47) : asset anime par defaut de chaque famille de mecanisme a
    // etat, stocke comme un skin de type ordinaire (meme catalogue, meme jeu courant) -- avec le
    // diagnostic des clips manquants et un apercu qui rejoue le meme catalogue d'animation que le
    // jeu (hmi::AnimationCatalog), pas une reimplementation parallele.
    _ui->animationsTree->setModel(_animationsModel);
    _ui->animationsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(_ui->animationsTree, &QTreeView::doubleClicked, this,
            &TexturePanel::onAnimationsAssetActivated);
    connect(_ui->animationsTree->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this] { onAnimationsSelectionChanged(); });
    connect(_animationPreviewTimer, &QTimer::timeout, this, &TexturePanel::tickAnimationPreview);
    _animationPreviewTimer->start(ANIMATION_PREVIEW_INTERVAL_MS);

    // Section « Décors » (LOT-50 TACHE-04) : liste groupée par couche (hmi::buildDecorListRows),
    // sélection croisée avec le canevas, actions de réordonnancement/changement de
    // couche/suppression/centrage — toutes passent par les mutateurs de TACHE-01 (annulables).
    _ui->decorsTable->setModel(_decorsModel);
    _ui->decorsTable->horizontalHeader()->setStretchLastSection(true);
    _ui->decorsTable->verticalHeader()->setVisible(false);
    connect(_ui->decorsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { onDecorsSelectionChanged(); });
    for (std::size_t i = 0; i < std::size(DECOR_LAYER_ORDER); ++i) {
        _ui->decorLayerCombo->addItem(QString());
    }
    connect(_ui->decorForwardButton, &QPushButton::clicked, this,
            [this] { onDecorForwardClicked(); });
    connect(_ui->decorBackwardButton, &QPushButton::clicked, this,
            [this] { onDecorBackwardClicked(); });
    connect(_ui->decorLayerCombo, &QComboBox::currentIndexChanged, this,
            [this](int index) { onDecorLayerComboChanged(index); });
    connect(_ui->decorRemoveButton, &QPushButton::clicked, this,
            [this] { onDecorRemoveClicked(); });
    connect(_ui->decorCenterButton, &QPushButton::clicked, this,
            [this] { onDecorCenterClicked(); });
}

TexturePanel::~TexturePanel() = default;

void TexturePanel::setCatalog(SkinCatalog* catalog) {
    _catalog = catalog;
    _currentSet = catalog == nullptr ? std::string{} : catalog->defaultSetName();
    rebuildTree();
    rebuildLevelSkinSetSelector();
    rebuildAnimationsTree();
}

void TexturePanel::setLevelProperties(const std::optional<std::string>& background,
                                      const std::optional<std::string>& skinSet) {
    _levelBackground = background;
    _levelSkinSet = skinSet;
    {
        // Synchronisation programmatique : ne doit jamais reemettre backgroundChanged, sous peine
        // de reboucler avec l'appelant (GameViewport::setLevelBackground -> draftChanged ->
        // setLevelProperties -> ...).
        const QSignalBlocker blocker(_backgroundView);
        _backgroundView->selectAsset(background.value_or(std::string{}));
    }
    rebuildLevelSkinSetSelector();  // bloque son propre signal en interne
}

void TexturePanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->setLabel->setText(QString::fromStdString(loc.text("textures.skin_set")));
    _ui->reloadButton->setText(QString::fromStdString(loc.text("textures.reload")));
    _ui->reloadButton->setToolTip(QString::fromStdString(loc.text("textures.reload_tooltip")));
    _ui->sections->setTabText(0, QString::fromStdString(loc.text("textures.section_skins")));
    _ui->sections->setTabText(1, QString::fromStdString(loc.text("textures.section_background")));
    _ui->sections->setTabText(2, QString::fromStdString(loc.text("textures.section_objects")));
    _ui->sections->setTabText(3, QString::fromStdString(loc.text("textures.section_animations")));
    _ui->sections->setTabText(4, QString::fromStdString(loc.text("textures.section_decors")));
    _ui->backgroundModeHintLabel->setText(
        QString::fromStdString(loc.text("textures.background_mode_hint")));
    _ui->backgroundLabel->setText(QString::fromStdString(loc.text("textures.background_label")));
    _ui->levelSkinSetLabel->setText(QString::fromStdString(loc.text("textures.level_skin_set")));
    _ui->levelSkinSetLabel->setToolTip(
        QString::fromStdString(loc.text("textures.level_skin_set_tooltip")));
    _ui->objectsAssetLabel->setText(QString::fromStdString(loc.text("textures.objects_asset_label")));
    _ui->objectsListLabel->setText(QString::fromStdString(loc.text("textures.objects_list_label")));
    _ui->objectsRemoveButton->setText(QString::fromStdString(loc.text("textures.objects_remove")));
    _objectsModel->setHorizontalHeaderLabels(
        {QString::fromStdString(loc.text("textures.objects_column_position")),
         QString::fromStdString(loc.text("textures.objects_column_asset"))});
    _ui->animationPreviewCaption->setText(
        QString::fromStdString(loc.text("textures.animations_preview")));
    _animationsModel->setHorizontalHeaderLabels(
        {QString::fromStdString(loc.text("textures.column_type")),
         QString::fromStdString(loc.text("textures.column_asset")),
         QString::fromStdString(loc.text("textures.animations_column_diagnostic"))});
    _ui->decorForwardButton->setText(QString::fromStdString(loc.text("textures.decors_forward")));
    _ui->decorBackwardButton->setText(QString::fromStdString(loc.text("textures.decors_backward")));
    _ui->decorCenterButton->setText(QString::fromStdString(loc.text("textures.decors_center")));
    _ui->decorRemoveButton->setText(QString::fromStdString(loc.text("textures.decors_remove")));
    _decorsModel->setHorizontalHeaderLabels(
        {QString::fromStdString(loc.text("textures.decors_column_asset")),
         QString::fromStdString(loc.text("textures.decors_column_layer"))});
    {
        const QSignalBlocker comboBlocker(_ui->decorLayerCombo);
        for (std::size_t i = 0; i < std::size(DECOR_LAYER_ORDER); ++i) {
            _ui->decorLayerCombo->setItemText(static_cast<int>(i),
                                              decorLayerLabel(DECOR_LAYER_ORDER[i]));
        }
    }
    _backgroundView->retranslateUi(loc);
    _objectView->retranslateUi(loc);
    rebuildTree();
    rebuildLevelSkinSetSelector();  // le libelle "jeu par defaut" vient de changer de langue
    rebuildObjectRows();
    rebuildAnimationsTree();
    rebuildDecorRows();  // les libelles de couche/le tooltip "asset manquant" dependent de _loc
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
    rebuildAnimationsTree();
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
    rebuildAnimationsTree();  // le type modifie peut etre l'une des six familles de mecanismes.
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

// Reconstruit la liste du selecteur de jeu de skins DU NIVEAU (section Fond), a partir des jeux du
// catalogue plus l'entree "jeu par defaut" ; reapplique la derniere valeur poussee par
// setLevelProperties, sans reemettre levelSkinSetChanged.
void TexturePanel::rebuildLevelSkinSetSelector() {
    const QSignalBlocker blocker(_ui->levelSkinSetSelector);
    _ui->levelSkinSetSelector->clear();
    _ui->levelSkinSetSelector->addItem(defaultSetLabel(_loc));
    if (_catalog != nullptr) {
        for (const std::string& name : _catalog->setNames()) {
            _ui->levelSkinSetSelector->addItem(QString::fromStdString(name));
        }
    }
    const QString target =
        _levelSkinSet ? QString::fromStdString(*_levelSkinSet) : defaultSetLabel(_loc);
    const int index = _ui->levelSkinSetSelector->findText(target);
    _ui->levelSkinSetSelector->setCurrentIndex(index >= 0 ? index : 0);
}

void TexturePanel::reloadAssets() {
    _thumbnails.clear();
    rebuildTree();
    rebuildLevelSkinSetSelector();
    _backgroundView->clearCache();
    _backgroundView->refresh();
    // La selection a pu etre perdue au rafraichissement (fichier renomme/supprime hors
    // application) : la resynchroniser avec la propriete du niveau, sans reemettre le signal.
    const QSignalBlocker blocker(_backgroundView);
    _backgroundView->selectAsset(_levelBackground.value_or(std::string{}));
    _objectView->clearCache();
    _objectView->refresh();
    // Un fichier d'animation a pu changer hors application : le diagnostic et l'apercu doivent
    // relire le disque, pas une copie memorisee (LOT-43 TACHE-03, LOT-47).
    _animationPreviewSheet = QImage();
    _animationPreviewDescription.reset();
    rebuildAnimationsTree();
    // Le contenu de Assets/Decors/ a pu changer (asset renomme/ajoute/supprime hors application) :
    // les vignettes sont redecodees ; le statut "asset manquant" se remettra a jour au prochain
    // refreshDecors (LOT-50 TACHE-04) -- meme limite acceptee que _objectRows ci-dessus, dont le
    // contenu n'est pas non plus revalide ici.
    _decorThumbnails.clear();
    rebuildDecorRows();
}

// Resynchronise la section « Objets » avec le brouillon courant (LOT-45).
void TexturePanel::refreshObjects(const core::LevelDraft& draft) {
    _objectRows = draft.textureOverrides();
    // Tri stable par position (colonne puis ligne) : un grand niveau peut porter beaucoup de
    // surcharges, une liste dans l'ordre d'insertion serait illisible.
    std::stable_sort(_objectRows.begin(), _objectRows.end(),
                     [](const core::TileTextureOverride& lhs, const core::TileTextureOverride& rhs) {
                         if (lhs.position.column != rhs.position.column) {
                             return lhs.position.column < rhs.position.column;
                         }
                         return lhs.position.row < rhs.position.row;
                     });
    rebuildObjectRows();
}

// Reconstruit le tableau depuis _objectRows.
void TexturePanel::rebuildObjectRows() {
    _objectsModel->removeRows(0, _objectsModel->rowCount());
    for (const core::TileTextureOverride& override : _objectRows) {
        auto* const positionItem = new QStandardItem(positionText(override.position));
        auto* const assetItem = new QStandardItem(QString::fromStdString(override.assetName));
        positionItem->setEditable(false);
        assetItem->setEditable(false);
        _objectsModel->appendRow(QList<QStandardItem*>{positionItem, assetItem});
    }
    _ui->objectsRemoveButton->setEnabled(false);
}

void TexturePanel::onObjectsSelectionChanged() {
    const QModelIndexList selected = _ui->objectsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        _ui->objectsRemoveButton->setEnabled(false);
        emit textureOverrideSelectionChanged(std::nullopt);
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _objectRows.size()) {
        _ui->objectsRemoveButton->setEnabled(false);
        emit textureOverrideSelectionChanged(std::nullopt);
        return;
    }
    _ui->objectsRemoveButton->setEnabled(true);
    emit textureOverrideSelectionChanged(_objectRows[static_cast<std::size_t>(row)].position);
}

void TexturePanel::onObjectsRemoveClicked() {
    const QModelIndexList selected = _ui->objectsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _objectRows.size()) {
        return;
    }
    emit textureOverrideRemoveRequested(_objectRows[static_cast<std::size_t>(row)].position);
}

// Vignette d'un asset de decor, decodee au premier besoin puis mise en cache (LOT-50 TACHE-04) --
// meme repli en damier magenta que thumbnailFor si absent/illisible.
QPixmap TexturePanel::decorThumbnailFor(const std::string& asset) {
    const auto cached = _decorThumbnails.find(asset);
    if (cached != _decorThumbnails.end()) {
        return cached->second;
    }

    QImage source;
    const std::optional<DecodedImage> decoded =
        asset.empty() ? std::nullopt : decodeImageFile(_decorsDirectory / asset);
    if (decoded) {
        source = toImage(*decoded);
    } else {
        const ProceduralAtlasImage missing = buildMissingTextureImage();
        source = toImage(DecodedImage{missing.width, missing.height, missing.pixels});
    }

    const QPixmap pixmap = QPixmap::fromImage(source.scaled(
        ROW_ICON_SIZE, ROW_ICON_SIZE, Qt::KeepAspectRatio, Qt::FastTransformation));
    _decorThumbnails.emplace(asset, pixmap);
    return pixmap;
}

// Nom localise d'une couche de decor, pour la colonne Couche et le selecteur de changement de
// couche -- meme trois libelles que le selecteur de l'outil Decor (ToolPanel, LOT-49).
QString TexturePanel::decorLayerLabel(core::DecorLayer layer) const {
    if (_loc == nullptr) {
        return QString();
    }
    switch (layer) {
        case core::DecorLayer::Background:
            return QString::fromStdString(_loc->text("tool.decor_layer_background"));
        case core::DecorLayer::Decor:
            return QString::fromStdString(_loc->text("tool.decor_layer_decor"));
        case core::DecorLayer::Foreground:
            return QString::fromStdString(_loc->text("tool.decor_layer_foreground"));
    }
    return QString();
}

// Resynchronise la section « Decors » avec le brouillon et la selection courants (LOT-50 TACHE-04).
void TexturePanel::refreshDecors(const core::LevelDraft& draft,
                                 std::optional<std::size_t> selectedIndex) {
    _decorRows = buildDecorListRows(draft.decors(), listAssetFiles(_decorsDirectory));
    _selectedDecorIndex = selectedIndex;
    rebuildDecorRows();
}

// Reconstruit le tableau depuis _decorRows, et reselectionne _selectedDecorIndex sans reemettre
// decorSelected (evite de reboucler avec l'appelant, meme garde que setLevelProperties).
void TexturePanel::rebuildDecorRows() {
    const QSignalBlocker tableBlocker(_ui->decorsTable->selectionModel());
    _decorsModel->removeRows(0, _decorsModel->rowCount());
    int selectedRow = -1;
    for (const DecorListRow& row : _decorRows) {
        auto* const assetItem = new QStandardItem(decorThumbnailFor(row.assetName),
                                                   QString::fromStdString(row.assetName));
        auto* const layerItem = new QStandardItem(decorLayerLabel(row.layer));
        assetItem->setEditable(false);
        layerItem->setEditable(false);
        if (row.assetMissing) {
            // Signale ce que le damier magenta signale deja dans le canevas (EX-NFR-040) : rouge
            // dans la liste, pour reperer tous les assets manquants d'un coup d'oeil.
            assetItem->setForeground(QColor(220, 60, 60));
            assetItem->setToolTip(
                _loc != nullptr ? QString::fromStdString(_loc->text("textures.decors_missing_asset"))
                                : QString());
        }
        assetItem->setData(static_cast<qulonglong>(row.index), Qt::UserRole);
        const int newRow = _decorsModel->rowCount();
        _decorsModel->appendRow(QList<QStandardItem*>{assetItem, layerItem});
        if (_selectedDecorIndex && *_selectedDecorIndex == row.index) {
            selectedRow = newRow;
        }
    }
    if (selectedRow >= 0) {
        _ui->decorsTable->selectRow(selectedRow);
    }
    updateDecorActionButtons();
}

// Active/desactive les boutons d'action et resynchronise le selecteur de couche (voir en-tete) --
// factorise entre rebuildDecorRows (silencieux) et onDecorsSelectionChanged (qui emet en plus).
void TexturePanel::updateDecorActionButtons() {
    const bool hasSelection = _selectedDecorIndex.has_value();
    _ui->decorForwardButton->setEnabled(hasSelection);
    _ui->decorBackwardButton->setEnabled(hasSelection);
    _ui->decorLayerCombo->setEnabled(hasSelection);
    _ui->decorRemoveButton->setEnabled(hasSelection);
    _ui->decorCenterButton->setEnabled(hasSelection);

    const QSignalBlocker comboBlocker(_ui->decorLayerCombo);
    if (!hasSelection) {
        return;
    }
    const auto found = std::find_if(_decorRows.begin(), _decorRows.end(),
                                    [this](const DecorListRow& row) {
                                        return row.index == *_selectedDecorIndex;
                                    });
    if (found == _decorRows.end()) {
        return;
    }
    _ui->decorLayerCombo->setCurrentIndex(decorLayerComboIndex(found->layer));
}

// Ligne selectionnee -> rang du decor (donnee stockee sur l'item, meme patron que positionText
// pour les surcharges), ou std::nullopt si aucune selection valide.
std::optional<std::size_t> TexturePanel::selectedDecorRowIndex() const {
    const QModelIndexList selected = _ui->decorsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return std::nullopt;
    }
    const QStandardItem* const item = _decorsModel->item(selected.first().row(), DECORS_COLUMN_ASSET);
    if (item == nullptr) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(item->data(Qt::UserRole).toULongLong());
}

void TexturePanel::onDecorsSelectionChanged() {
    _selectedDecorIndex = selectedDecorRowIndex();
    updateDecorActionButtons();
    emit decorSelected(_selectedDecorIndex);
}

void TexturePanel::onDecorForwardClicked() {
    if (_selectedDecorIndex) {
        emit decorForwardRequested(*_selectedDecorIndex);
    }
}

void TexturePanel::onDecorBackwardClicked() {
    if (_selectedDecorIndex) {
        emit decorBackwardRequested(*_selectedDecorIndex);
    }
}

void TexturePanel::onDecorLayerComboChanged(int index) {
    if (!_selectedDecorIndex || index < 0 ||
        index >= static_cast<int>(std::size(DECOR_LAYER_ORDER))) {
        return;
    }
    emit decorLayerChangeRequested(*_selectedDecorIndex, DECOR_LAYER_ORDER[index]);
}

void TexturePanel::onDecorRemoveClicked() {
    if (_selectedDecorIndex) {
        emit decorRemoveRequested(*_selectedDecorIndex);
    }
}

void TexturePanel::onDecorCenterClicked() {
    if (_selectedDecorIndex) {
        emit decorCenterRequested(*_selectedDecorIndex);
    }
}

// Reconstruit l'arbre de la section « Animations » (LOT-47 TACHE-04) : logique hors du widget
// (hmi::buildMechanismAnimationRows), meme patron que rebuildTree pour les skins.
void TexturePanel::rebuildAnimationsTree() {
    _animationsModel->removeRows(0, _animationsModel->rowCount());
    if (_loc != nullptr) {
        _animationsModel->setHorizontalHeaderLabels(
            {QString::fromStdString(_loc->text("textures.column_type")),
             QString::fromStdString(_loc->text("textures.column_asset")),
             QString::fromStdString(_loc->text("textures.animations_column_diagnostic"))});
    }
    if (_catalog == nullptr) {
        return;
    }

    for (const MechanismAnimationRow& row : buildMechanismAnimationRows(*_catalog, _currentSet,
                                                                        _skinsDirectory)) {
        auto* const typeItem =
            new QStandardItem(QString::fromStdString(localizedTaxonomyLabel(_loc, row.typeLabel)));
        typeItem->setEditable(false);
        typeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

        auto* const assetItem = new QStandardItem(
            row.asset.empty() ? noneLabel(_loc) : QString::fromStdString(row.asset));
        assetItem->setEditable(false);
        assetItem->setIcon(QIcon(thumbnailFor(row.asset)));
        assetItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

        auto* const diagnosticItem = new QStandardItem(diagnosticText(_loc, row));
        diagnosticItem->setEditable(false);
        diagnosticItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

        _animationsModel->appendRow(QList<QStandardItem*>{typeItem, assetItem, diagnosticItem});
    }
}

// Ouvre le selecteur a vignettes sur un double-clic dans la colonne Fichier de l'arbre
// « Animations » (meme patron que onAssetColumnActivated : un mecanisme est un type de tuile
// comme un autre pour hmi::SkinCatalog, seul le mode SkinMode::Single a un sens ici, EX-REN-046).
void TexturePanel::onAnimationsAssetActivated(const QModelIndex& index) {
    if (_catalog == nullptr || index.column() != ANIMATIONS_COLUMN_ASSET) {
        return;
    }
    const QVariant typeData = index.data(TILE_TYPE_ROLE);
    if (!typeData.isValid()) {
        return;
    }
    const auto type = static_cast<core::TileType>(typeData.toInt());

    const QString currentText = index.data(Qt::DisplayRole).toString();
    const std::string currentAsset =
        currentText == noneLabel(_loc) ? std::string{} : currentText.toStdString();

    AssetPickerDialog dialog(_skinsDirectory, _loc, this);
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
    // Toujours SkinMode::Single : un mecanisme a etat n'a pas de voisinage a raccorder (LOT-47
    // TACHE-01, meme famille que dangers/blocs dans la section « Skins »).
    applySkinAssignment(*_catalog, _currentSet, type, dialog.selectedAsset(), SkinMode::Single);
    save();
    rebuildTree();  // le meme catalogue alimente aussi la section « Skins ».
    rebuildAnimationsTree();
    emit assignmentsChanged();
}

// Charge l'apercu (spritesheet decodee + description d'animation) de la ligne selectionnee.
void TexturePanel::onAnimationsSelectionChanged() {
    _animationPreviewSheet = QImage();
    _animationPreviewDescription.reset();
    _animationPreviewAnimation = core::Animation{};

    const QModelIndexList selected = _ui->animationsTree->selectionModel()->selectedRows();
    if (selected.isEmpty() || _catalog == nullptr) {
        return;
    }
    const QVariant typeData = selected.first().data(TILE_TYPE_ROLE);
    if (!typeData.isValid()) {
        return;
    }
    const auto type = static_cast<core::TileType>(typeData.toInt());
    const std::optional<SkinEntry> entry = _catalog->resolve(_currentSet, type);
    if (!entry || entry->asset.empty()) {
        return;
    }

    const std::optional<DecodedImage> decoded = decodeImageFile(_skinsDirectory / entry->asset);
    if (!decoded) {
        return;
    }
    _animationPreviewSheet = toImage(*decoded);

    const AnimationDescriptionResult result = AnimationCatalog::loadFromFile(
        _skinsDirectory / AnimationCatalog::descriptorFileName(entry->asset));
    if (!result.ok()) {
        return;  // asset non anime (ou description invalide) : l'apercu reste sur l'image entiere.
    }
    _animationPreviewDescription = result.description;
    _animationPreviewAnimation.clips =
        std::make_shared<core::ClipSet>(_animationPreviewDescription->clips);
    // Etat "au repos" (porte fermee, interrupteur/plaque relache, danger inoffensif...) : la
    // transition elle-meme se voit en jeu, l'apercu montre l'apparence de base (LOT-47 TACHE-04).
    const std::string clipName = mechanismTargetClip(type, false).value_or(std::string{});
    const int clipIndex = _animationPreviewDescription->clips.indexOf(clipName);
    _animationPreviewAnimation.clipIndex = clipIndex >= 0 ? clipIndex : 0;
}

// Avance l'horloge d'apercu d'un pas de minuterie et rafraichit la vignette (LOT-47 TACHE-04).
void TexturePanel::tickAnimationPreview() {
    if (_animationPreviewSheet.isNull()) {
        _ui->animationPreviewLabel->clear();
        return;
    }

    QImage frame;
    if (_animationPreviewDescription && _animationPreviewAnimation.clips) {
        core::advanceAnimation(_animationPreviewAnimation,
                               static_cast<float>(ANIMATION_PREVIEW_INTERVAL_MS) / 1000.0f);
        const core::AtlasRegion region = AnimationCatalog::currentFrameRegion(
            *_animationPreviewDescription, _animationPreviewAnimation);
        frame = _animationPreviewSheet.copy(region.x, region.y, region.width, region.height);
    } else {
        frame = _animationPreviewSheet;
    }

    const QPixmap pixmap = QPixmap::fromImage(frame.scaled(
        frame.width() * ANIMATION_PREVIEW_SCALE, frame.height() * ANIMATION_PREVIEW_SCALE,
        Qt::KeepAspectRatio, Qt::FastTransformation));
    _ui->animationPreviewLabel->setPixmap(pixmap);
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
