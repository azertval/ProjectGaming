#include "Editor/ToolPanel.h"

#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>

namespace editor {

ToolPanel::ToolPanel(QWidget* parent) : QWidget(parent), _group(new QButtonGroup(this)) {
    auto* const paint = new QRadioButton(QStringLiteral("Pinceau"), this);
    auto* const rectangle = new QRadioButton(QStringLiteral("Rectangle"), this);
    auto* const selection = new QRadioButton(QStringLiteral("Sélection"), this);
    paint->setChecked(true);

    _group->addButton(paint, static_cast<int>(hmi::EditorTool::Paint));
    _group->addButton(rectangle, static_cast<int>(hmi::EditorTool::Rectangle));
    _group->addButton(selection, static_cast<int>(hmi::EditorTool::Selection));
    connect(_group, &QButtonGroup::idClicked, this,
            [this](int id) { emit toolSelected(static_cast<hmi::EditorTool>(id)); });

    auto* const layout = new QVBoxLayout(this);
    layout->addWidget(paint);
    layout->addWidget(rectangle);
    layout->addWidget(selection);
    layout->addStretch();
}

}  // namespace editor
