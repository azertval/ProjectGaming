#include "HMI/Editor/ToolPanel.h"

#include <QCheckBox>
#include <QComboBox>
#include <cstddef>
#include <iterator>
#include <utility>

#include "HMI/Editor/AssetThumbnailView.h"
#include "HMI/Localization/Localization.h"
#include "ui_ToolPanel.h"

namespace hmi {

namespace {
// Ordre des entrees du selecteur de couche (decorLayerCombo), synchronise avec core::DecorLayer.
constexpr core::DecorLayer DECOR_LAYER_ORDER[] = {
    core::DecorLayer::Background, core::DecorLayer::Decor, core::DecorLayer::Foreground};
}  // namespace

ToolPanel::ToolPanel(std::filesystem::path decorsDirectory, QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::ToolPanel>()), _decorView(nullptr) {
    _ui->setupUi(this);

    // Placement minimal de l'outil Decor (LOT-49 TACHE-04) : grille de vignettes sur Assets/Decors/
    // (aucune entree "(aucun)" -- poser sans asset selectionne n'a pas de sens), plus le choix de
    // couche. Meme patron que la section "Objets" de TexturePanel (LOT-45).
    _decorView = new AssetThumbnailView(this);
    _decorView->setAssetFamily(AssetFamily::Decor);
    _decorView->setNoneOptionVisible(false);
    _decorView->setDirectory(std::move(decorsDirectory));
    _ui->decorPickerContainerLayout->addWidget(_decorView);
    connect(_decorView, &AssetThumbnailView::assetSelected, this,
            [this](const QString& fileName) { emit decorAssetSelected(fileName); });

    for (std::size_t i = 0; i < std::size(DECOR_LAYER_ORDER); ++i) {
        _ui->decorLayerCombo->addItem(QString());
    }
    _ui->decorLayerCombo->setCurrentIndex(1);  // core::DecorLayer::Decor, couche de reference
    connect(_ui->decorLayerCombo, &QComboBox::currentIndexChanged, this,
            [this](int index) {
                if (index >= 0 && index < static_cast<int>(std::size(DECOR_LAYER_ORDER))) {
                    emit decorLayerSelected(DECOR_LAYER_ORDER[index]);
                }
            });

    connect(_ui->decorSnapCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { emit decorSnapToGridChanged(checked); });

    setActiveTool(hmi::EditorTool::Paint);  // outil actif par defaut (EditorActions/GameViewport).
}

ToolPanel::~ToolPanel() = default;

void ToolPanel::retranslateUi(const Localization& loc) {
    _ui->decorLayerCombo->setItemText(0, QString::fromStdString(loc.text("tool.decor_layer_background")));
    _ui->decorLayerCombo->setItemText(1, QString::fromStdString(loc.text("tool.decor_layer_decor")));
    _ui->decorLayerCombo->setItemText(2, QString::fromStdString(loc.text("tool.decor_layer_foreground")));
    _ui->decorSnapCheckBox->setText(QString::fromStdString(loc.text("tool.decor_snap_to_grid")));
    _decorView->retranslateUi(loc);
}

void ToolPanel::setActiveTool(hmi::EditorTool tool) {
    // Le selecteur de decors n'a de sens que pour l'outil Decor : le masquer sinon evite
    // d'encombrer les autres outils (meme principe que showTextureOverrides du DraftRenderer).
    _ui->decorPickerGroup->setVisible(tool == hmi::EditorTool::Decor);
}

}  // namespace hmi
