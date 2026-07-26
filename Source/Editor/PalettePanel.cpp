#include "Editor/PalettePanel.h"

#include <QItemSelectionModel>
#include <QModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>

#include "Editor/TileTaxonomy.h"

namespace editor {

namespace {

// Rôle de données portant le `core::TileType` d'une feuille (les en-têtes n'en ont pas).
constexpr int TILE_TYPE_ROLE = Qt::UserRole + 1;

// Crée une feuille sélectionnable portant son type de tuile.
[[nodiscard]] QStandardItem* makeLeaf(const TileEntry& entry) {
    auto* const item = new QStandardItem(QString::fromStdString(entry.label));
    item->setEditable(false);
    item->setData(static_cast<int>(entry.type), TILE_TYPE_ROLE);
    return item;
}

// Crée un en-tête (catégorie/sous-groupe) : affiché, mais non sélectionnable comme tuile.
[[nodiscard]] QStandardItem* makeHeader(const QString& label) {
    auto* const item = new QStandardItem(label);
    item->setEditable(false);
    item->setFlags(Qt::ItemIsEnabled);  // ni sélectionnable, ni porteur de type.
    return item;
}

}  // namespace

PalettePanel::PalettePanel(QWidget* parent)
    : QWidget(parent), _tree(new QTreeView(this)), _model(new QStandardItemModel(this)) {
    _tree->setHeaderHidden(true);
    _tree->setModel(_model);
    _tree->setSelectionMode(QAbstractItemView::SingleSelection);

    buildModel();
    _tree->expandAll();

    connect(_tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) { onCurrentChanged(current); });

    auto* const layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_tree);
}

void PalettePanel::buildModel() {
    for (const TileCategory& category : tileTaxonomy()) {
        QStandardItem* const categoryItem = makeHeader(QString::fromStdString(category.label));
        for (const TileEntry& entry : category.tiles) {
            categoryItem->appendRow(makeLeaf(entry));
        }
        for (const TileSubgroup& subgroup : category.subgroups) {
            QStandardItem* const subgroupItem = makeHeader(QString::fromStdString(subgroup.label));
            for (const TileEntry& entry : subgroup.tiles) {
                subgroupItem->appendRow(makeLeaf(entry));
            }
            categoryItem->appendRow(subgroupItem);
        }
        _model->appendRow(categoryItem);
    }
}

void PalettePanel::onCurrentChanged(const QModelIndex& current) {
    const QVariant tileData = current.data(TILE_TYPE_ROLE);
    if (!tileData.isValid()) {
        return;  // en-tête (catégorie/sous-groupe) : pas un type sélectionnable.
    }
    _selected = static_cast<core::TileType>(tileData.toInt());
    emit tileSelected(_selected);
}

}  // namespace editor
