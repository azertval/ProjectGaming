#include "HMI/Editor/ToolPanel.h"

#include <QRadioButton>

#include "HMI/Localization/Localization.h"
#include "ui_ToolPanel.h"

namespace hmi {

ToolPanel::ToolPanel(QWidget* parent) : QWidget(parent), _ui(std::make_unique<Ui::ToolPanel>()) {
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
}

ToolPanel::~ToolPanel() = default;

void ToolPanel::retranslateUi(const Localization& loc) {
    _ui->paintRadio->setText(QString::fromStdString(loc.text("tool.brush")));
    _ui->rectangleRadio->setText(QString::fromStdString(loc.text("tool.rectangle")));
    _ui->selectionRadio->setText(QString::fromStdString(loc.text("tool.selection")));
}

}  // namespace hmi
