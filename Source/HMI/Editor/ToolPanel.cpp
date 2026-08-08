#include "HMI/Editor/ToolPanel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QSignalBlocker>
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

    // Les boutons radio frères sont exclusifs par défaut ; chacun émet l'outil correspondant.
    connect(_ui->paintRadio, &QRadioButton::toggled, this, [this](bool on) {
        if (on) {
            emit toolSelected(hmi::EditorTool::Paint);
        }
    });
    connect(_ui->rectangleRadio, &QRadioButton::toggled, this, [this](bool on) {
        if (on) {
            emit toolSelected(hmi::EditorTool::Rectangle);
        }
    });
    connect(_ui->selectionRadio, &QRadioButton::toggled, this, [this](bool on) {
        if (on) {
            emit toolSelected(hmi::EditorTool::Selection);
        }
    });
    connect(_ui->linkRadio, &QRadioButton::toggled, this, [this](bool on) {
        if (on) {
            emit toolSelected(hmi::EditorTool::Link);
        }
    });
    connect(_ui->textureAssignRadio, &QRadioButton::toggled, this, [this](bool on) {
        if (on) {
            emit toolSelected(hmi::EditorTool::TextureAssign);
        }
    });
    connect(_ui->decorRadio, &QRadioButton::toggled, this, [this](bool on) {
        updateDecorPickerVisibility();
        if (on) {
            emit toolSelected(hmi::EditorTool::Decor);
        }
    });

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

    updateDecorPickerVisibility();
}

ToolPanel::~ToolPanel() = default;

void ToolPanel::retranslateUi(const Localization& loc) {
    _ui->paintRadio->setText(QString::fromStdString(loc.text("tool.brush")));
    _ui->rectangleRadio->setText(QString::fromStdString(loc.text("tool.rectangle")));
    _ui->selectionRadio->setText(QString::fromStdString(loc.text("tool.selection")));
    _ui->linkRadio->setText(QString::fromStdString(loc.text("tool.link")));
    _ui->textureAssignRadio->setText(QString::fromStdString(loc.text("tool.texture_assign")));
    _ui->decorRadio->setText(QString::fromStdString(loc.text("tool.decor")));
    _ui->decorLayerCombo->setItemText(0, QString::fromStdString(loc.text("tool.decor_layer_background")));
    _ui->decorLayerCombo->setItemText(1, QString::fromStdString(loc.text("tool.decor_layer_decor")));
    _ui->decorLayerCombo->setItemText(2, QString::fromStdString(loc.text("tool.decor_layer_foreground")));
    _ui->decorSnapCheckBox->setText(QString::fromStdString(loc.text("tool.decor_snap_to_grid")));
    _decorView->retranslateUi(loc);
}

void ToolPanel::setActiveTool(hmi::EditorTool tool) {
    QRadioButton* const button = [this, tool]() -> QRadioButton* {
        switch (tool) {
            case hmi::EditorTool::Paint:
                return _ui->paintRadio;
            case hmi::EditorTool::Rectangle:
                return _ui->rectangleRadio;
            case hmi::EditorTool::Selection:
                return _ui->selectionRadio;
            case hmi::EditorTool::Link:
                return _ui->linkRadio;
            case hmi::EditorTool::TextureAssign:
                return _ui->textureAssignRadio;
            case hmi::EditorTool::Decor:
                return _ui->decorRadio;
        }
        return nullptr;
    }();
    if (button == nullptr || button->isChecked()) {
        return;
    }
    const QSignalBlocker blocker(button);
    button->setChecked(true);
    updateDecorPickerVisibility();
}

void ToolPanel::updateDecorPickerVisibility() {
    // Le selecteur de decors n'a de sens que pour l'outil Decor : le masquer sinon evite
    // d'encombrer les autres outils (meme principe que showTextureOverrides du DraftRenderer).
    _ui->decorPickerGroup->setVisible(_ui->decorRadio->isChecked());
}

}  // namespace hmi
