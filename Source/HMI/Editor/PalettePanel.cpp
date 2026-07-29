#include "HMI/Editor/PalettePanel.h"

#include <QItemSelectionModel>
#include <QModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>

#include <string>
#include <unordered_map>

#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Editor/TileTaxonomy.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Rôle de données portant le `core::TileType` d'une feuille (les en-têtes n'en ont pas).
constexpr int TILE_TYPE_ROLE = Qt::UserRole + 1;

// Libelle de taxonomie traduit. La table libelle -> cle vit dans TaxonomyLabels : point unique
// partage avec le panneau « Textures » (LOT-42). Deux copies divergeraient au premier libelle
// ajoute, et un panneau afficherait alors du francais dans une interface anglaise.
[[nodiscard]] QString localized(const Localization* loc, const std::string& label) {
    return QString::fromStdString(localizedTaxonomyLabel(loc, label));
}

// Crée une feuille sélectionnable portant son type de tuile.
[[nodiscard]] QStandardItem* makeLeaf(const TileEntry& entry, const Localization* loc) {
    auto* const item = new QStandardItem(localized(loc, entry.label));
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
        QStandardItem* const categoryItem = makeHeader(localized(_loc, category.label));
        for (const TileEntry& entry : category.tiles) {
            categoryItem->appendRow(makeLeaf(entry, _loc));
        }
        for (const TileSubgroup& subgroup : category.subgroups) {
            QStandardItem* const subgroupItem = makeHeader(localized(_loc, subgroup.label));
            for (const TileEntry& entry : subgroup.tiles) {
                subgroupItem->appendRow(makeLeaf(entry, _loc));
            }
            categoryItem->appendRow(subgroupItem);
        }
        _model->appendRow(categoryItem);
    }
}

void PalettePanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _model->clear();
    buildModel();
    _tree->expandAll();
}

void PalettePanel::onCurrentChanged(const QModelIndex& current) {
    const QVariant tileData = current.data(TILE_TYPE_ROLE);
    if (!tileData.isValid()) {
        return;  // en-tête (catégorie/sous-groupe) : pas un type sélectionnable.
    }
    _selected = static_cast<core::TileType>(tileData.toInt());
    emit tileSelected(_selected);
}

}  // namespace hmi
