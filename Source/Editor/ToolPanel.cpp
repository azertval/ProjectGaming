#include "Editor/ToolPanel.h"

#include <QRadioButton>

#include "ui_ToolPanel.h"

namespace editor {

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

}  // namespace editor
