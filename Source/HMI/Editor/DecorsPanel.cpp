#include "HMI/Editor/DecorsPanel.h"

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QEvent>
#include <QHeaderView>
#include <QImage>
#include <QItemSelectionModel>
#include <QList>
#include <QModelIndex>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTableView>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <utility>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Editor/AssetLibrary.h"
#include "HMI/Editor/AssetThumbnailView.h"
#include "HMI/Editor/ThumbnailGeometry.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "ui_DecorsPanel.h"

namespace hmi {

namespace {

// Ordre des entrees des deux selecteurs de couche, synchronise avec core::DecorLayer -- une seule
// definition, partagee par le placement (prochain decor pose) et l'inspecteur (decor selectionne).
constexpr std::array<core::DecorLayer, 3> DECOR_LAYER_ORDER{
    core::DecorLayer::Background, core::DecorLayer::Decor, core::DecorLayer::Foreground};

// Rang de core::DecorLayer dans DECOR_LAYER_ORDER (index du selecteur de couche de l'inspecteur).
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

// Colonnes du tableau de l'inspecteur (LOT-50) : vignette+asset, couche.
constexpr int DECORS_COLUMN_ASSET = 0;
constexpr int DECORS_COLUMN_LAYER = 1;
constexpr int DECORS_COLUMN_COUNT = 2;

const int ROW_ICON_SIZE = editorDarkTokens().size.iconMedium;

// Convertit des pixels RGBA decodes en QImage. Meme conversion que PalettePanel/AssetThumbnailView/
// TexturePanel : trop petite pour justifier une fonction partagee entre widgets independants.
[[nodiscard]] QImage toImage(const DecodedImage& decoded) {
    QImage image(decoded.width, decoded.height, QImage::Format_RGBA8888);
    for (int y = 0; y < decoded.height; ++y) {
        const std::uint32_t* const row =
            decoded.pixels.data() + static_cast<std::size_t>(y) * decoded.width;
        std::memcpy(image.scanLine(y), row, static_cast<std::size_t>(decoded.width) * 4);
    }
    return image;
}

}  // namespace

DecorsPanel::DecorsPanel(std::filesystem::path decorsDirectory, QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::DecorsPanel>()),
      _decorView(nullptr),
      _decorsModel(new QStandardItemModel(0, DECORS_COLUMN_COUNT, this)),
      _decorsDirectory(std::move(decorsDirectory)) {
    _ui->setupUi(this);

    // Placement de decors (LOT-49 TACHE-04) : grille de vignettes sur Assets/Decors/ (aucune
    // entree "(aucun)" -- poser sans asset selectionne n'a pas de sens), plus le choix de couche du
    // *prochain* decor pose.
    _decorView = new AssetThumbnailView(this);
    _decorView->setAssetFamily(AssetFamily::Decor);
    _decorView->setNoneOptionVisible(false);
    _decorView->setDirectory(_decorsDirectory);
    _ui->decorPickerContainerLayout->addWidget(_decorView);
    connect(_decorView, &AssetThumbnailView::assetSelected, this,
            [this](const QString& fileName) { emit decorAssetSelected(fileName); });

    for (std::size_t i = 0; i < std::size(DECOR_LAYER_ORDER); ++i) {
        _ui->decorPlacementLayerCombo->addItem(QString());
    }
    _ui->decorPlacementLayerCombo->setCurrentIndex(
        1);  // core::DecorLayer::Decor, couche de reference
    connect(_ui->decorPlacementLayerCombo, &QComboBox::currentIndexChanged, this,
            [this](int index) {
                if (index >= 0 && index < static_cast<int>(std::size(DECOR_LAYER_ORDER))) {
                    emit decorLayerSelected(DECOR_LAYER_ORDER[index]);
                }
            });
    connect(_ui->decorSnapCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { emit decorSnapToGridChanged(checked); });

    // Inspecteur des decors poses (LOT-50 TACHE-04, deplace de TexturePanel) : liste groupee par
    // couche (hmi::buildDecorListRows), selection croisee avec le canevas, actions de
    // reordonnancement/changement de couche/suppression/centrage -- toutes passent par les
    // mutateurs de core::LevelDraft (annulables).
    _ui->decorsTable->setModel(_decorsModel);
    _ui->decorsTable->horizontalHeader()->setStretchLastSection(true);
    _ui->decorsTable->verticalHeader()->setVisible(false);
    connect(_ui->decorsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { onDecorsSelectionChanged(); });
    for (std::size_t i = 0; i < std::size(DECOR_LAYER_ORDER); ++i) {
        _ui->decorSelectedLayerCombo->addItem(QString());
    }
    connect(_ui->decorForwardButton, &QPushButton::clicked, this,
            [this] { onDecorForwardClicked(); });
    connect(_ui->decorBackwardButton, &QPushButton::clicked, this,
            [this] { onDecorBackwardClicked(); });
    connect(_ui->decorSelectedLayerCombo, &QComboBox::currentIndexChanged, this,
            [this](int index) { onDecorLayerComboChanged(index); });
    connect(_ui->decorRemoveButton, &QPushButton::clicked, this,
            [this] { onDecorRemoveClicked(); });
    connect(_ui->decorCenterButton, &QPushButton::clicked, this,
            [this] { onDecorCenterClicked(); });

    setActiveTool(hmi::EditorTool::Paint);  // outil actif par defaut (EditorActions/GameViewport).
}

DecorsPanel::~DecorsPanel() = default;

bool DecorsPanel::event(QEvent* event) {
    if (event->type() == QEvent::ScreenChangeInternal) {
        // Un deplacement vers un ecran d'echelle differente doit regenerer les vignettes de
        // l'inspecteur (LOT-56 TACHE-05) : le cache decode reste valable (memes fichiers), seule
        // la mise a l'echelle doit etre rejouee.
        reloadDecorThumbnails();
    }
    return QWidget::event(event);
}

void DecorsPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    for (std::size_t i = 0; i < std::size(DECOR_LAYER_ORDER); ++i) {
        _ui->decorPlacementLayerCombo->setItemText(static_cast<int>(i),
                                                   decorLayerLabel(DECOR_LAYER_ORDER[i]));
    }
    _ui->decorSnapCheckBox->setText(QString::fromStdString(loc.text("tool.decor_snap_to_grid")));
    _decorView->retranslateUi(loc);

    _ui->decorForwardButton->setText(QString::fromStdString(loc.text("decors.forward")));
    _ui->decorBackwardButton->setText(QString::fromStdString(loc.text("decors.backward")));
    _ui->decorCenterButton->setText(QString::fromStdString(loc.text("decors.center")));
    _ui->decorRemoveButton->setText(QString::fromStdString(loc.text("decors.remove")));
    _decorsModel->setHorizontalHeaderLabels(
        {QString::fromStdString(loc.text("decors.column_asset")),
         QString::fromStdString(loc.text("decors.column_layer"))});
    {
        const QSignalBlocker comboBlocker(_ui->decorSelectedLayerCombo);
        for (std::size_t i = 0; i < std::size(DECOR_LAYER_ORDER); ++i) {
            _ui->decorSelectedLayerCombo->setItemText(static_cast<int>(i),
                                                      decorLayerLabel(DECOR_LAYER_ORDER[i]));
        }
    }
    rebuildDecorRows();  // les libelles de couche/le tooltip "asset manquant" dependent de _loc
}

void DecorsPanel::setActiveTool(hmi::EditorTool tool) {
    // Le selecteur de placement n'a de sens que pour l'outil Decor : le masquer sinon evite
    // d'encombrer les autres outils (meme principe que showTextureOverrides du DraftRenderer).
    // L'inspecteur, lui, reste toujours visible (LOT-57 TACHE-03, meme principe que les
    // inspecteurs Objets/Decors du panneau Textures).
    _ui->decorPickerGroup->setVisible(tool == hmi::EditorTool::Decor);
}

void DecorsPanel::refreshDecors(const core::LevelDraft& draft,
                                std::optional<std::size_t> selectedIndex) {
    _decorRows = buildDecorListRows(draft.decors(), listAssetFiles(_decorsDirectory));
    _selectedDecorIndex = selectedIndex;
    rebuildDecorRows();
}

void DecorsPanel::reloadDecorThumbnails() {
    // Le contenu de Assets/Decors/ a pu changer (asset renomme/ajoute/supprime hors application) :
    // les vignettes sont redecodees ; le statut "asset manquant" se remettra a jour au prochain
    // refreshDecors.
    _decorThumbnails.clear();
    rebuildDecorRows();
}

QPixmap DecorsPanel::decorThumbnailFor(const std::string& asset) {
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

    const qreal scale = devicePixelRatioF();
    const int pixelSize = thumbnailPixelSize(ROW_ICON_SIZE, scale);
    QPixmap pixmap = QPixmap::fromImage(
        source.scaled(pixelSize, pixelSize, Qt::KeepAspectRatio, Qt::FastTransformation));
    pixmap.setDevicePixelRatio(scale);
    _decorThumbnails.emplace(asset, pixmap);
    return pixmap;
}

QString DecorsPanel::decorLayerLabel(core::DecorLayer layer) const {
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

void DecorsPanel::rebuildDecorRows() {
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
            assetItem->setToolTip(_loc != nullptr
                                      ? QString::fromStdString(_loc->text("decors.missing_asset"))
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

void DecorsPanel::updateDecorActionButtons() {
    const bool hasSelection = _selectedDecorIndex.has_value();
    _ui->decorForwardButton->setEnabled(hasSelection);
    _ui->decorBackwardButton->setEnabled(hasSelection);
    _ui->decorSelectedLayerCombo->setEnabled(hasSelection);
    _ui->decorRemoveButton->setEnabled(hasSelection);
    _ui->decorCenterButton->setEnabled(hasSelection);

    const QSignalBlocker comboBlocker(_ui->decorSelectedLayerCombo);
    if (!hasSelection) {
        return;
    }
    const auto found =
        std::find_if(_decorRows.begin(), _decorRows.end(),
                     [this](const DecorListRow& row) { return row.index == *_selectedDecorIndex; });
    if (found == _decorRows.end()) {
        return;
    }
    _ui->decorSelectedLayerCombo->setCurrentIndex(decorLayerComboIndex(found->layer));
}

std::optional<std::size_t> DecorsPanel::selectedDecorRowIndex() const {
    const QModelIndexList selected = _ui->decorsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return std::nullopt;
    }
    const QStandardItem* const item =
        _decorsModel->item(selected.first().row(), DECORS_COLUMN_ASSET);
    if (item == nullptr) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(item->data(Qt::UserRole).toULongLong());
}

void DecorsPanel::onDecorsSelectionChanged() {
    _selectedDecorIndex = selectedDecorRowIndex();
    updateDecorActionButtons();
    emit decorSelected(_selectedDecorIndex);
}

void DecorsPanel::onDecorForwardClicked() {
    if (_selectedDecorIndex) {
        emit decorForwardRequested(*_selectedDecorIndex);
    }
}

void DecorsPanel::onDecorBackwardClicked() {
    if (_selectedDecorIndex) {
        emit decorBackwardRequested(*_selectedDecorIndex);
    }
}

void DecorsPanel::onDecorLayerComboChanged(int index) {
    if (!_selectedDecorIndex || index < 0 ||
        index >= static_cast<int>(std::size(DECOR_LAYER_ORDER))) {
        return;
    }
    emit decorLayerChangeRequested(*_selectedDecorIndex, DECOR_LAYER_ORDER[index]);
}

void DecorsPanel::onDecorRemoveClicked() {
    if (_selectedDecorIndex) {
        emit decorRemoveRequested(*_selectedDecorIndex);
    }
}

void DecorsPanel::onDecorCenterClicked() {
    if (_selectedDecorIndex) {
        emit decorCenterRequested(*_selectedDecorIndex);
    }
}

}  // namespace hmi
