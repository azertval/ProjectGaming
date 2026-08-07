#include "HMI/Editor/ToolPanel.h"

#include <QRadioButton>
#include <QSignalBlocker>

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
}

ToolPanel::~ToolPanel() = default;

void ToolPanel::retranslateUi(const Localization& loc) {
    _ui->paintRadio->setText(QString::fromStdString(loc.text("tool.brush")));
    _ui->rectangleRadio->setText(QString::fromStdString(loc.text("tool.rectangle")));
    _ui->selectionRadio->setText(QString::fromStdString(loc.text("tool.selection")));
    _ui->linkRadio->setText(QString::fromStdString(loc.text("tool.link")));
    _ui->textureAssignRadio->setText(QString::fromStdString(loc.text("tool.texture_assign")));
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
        }
        return nullptr;
    }();
    if (button == nullptr || button->isChecked()) {
        return;
    }
    const QSignalBlocker blocker(button);
    button->setChecked(true);
}

}  // namespace hmi
