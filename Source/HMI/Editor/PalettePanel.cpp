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

#include "HMI/Editor/TileTaxonomy.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Rôle de données portant le `core::TileType` d'une feuille (les en-têtes n'en ont pas).
constexpr int TILE_TYPE_ROLE = Qt::UserRole + 1;

// Table libellé (français, source de la taxonomie) -> clé de traduction (anglais). La taxonomie
// reste une logique pure sans dépendance i18n ; la palette résout ses libellés ici.
[[nodiscard]] QString localized(const Localization* loc, const std::string& label) {
    static const std::unordered_map<std::string, std::string> keys = {
        {"Tuile", "palette.cat.tile"},
        {"Interactif", "palette.cat.interactive"},
        {"Piège", "palette.cat.trap"},
        {"Jalon", "palette.cat.marker"},
        {"Pente", "palette.sub.slope"},
        {"Arrondi", "palette.sub.rounded"},
        {"Concave", "palette.sub.concave"},
        {"Bloc poussable", "palette.sub.block"},
        {"Directionnel", "palette.sub.directional"},
        {"Vide (gomme)", "palette.tile.empty"},
        {"Plein", "palette.tile.solid"},
        {"Montée droite", "palette.tile.rise_right"},
        {"Montée gauche", "palette.tile.rise_left"},
        {"Plafond droite", "palette.tile.ceiling_right"},
        {"Plafond gauche", "palette.tile.ceiling_left"},
        {"Demi (½)", "palette.tile.half"},
        {"Quart (¼)", "palette.tile.quarter"},
        {"Interrupteur", "palette.tile.switch"},
        {"Plaque de pression", "palette.tile.plate"},
        {"Porte", "palette.tile.door"},
        {"Danger", "palette.tile.danger"},
        {"Danger mobile", "palette.tile.danger_moving"},
        {"Danger commuté", "palette.tile.danger_switched"},
        {"Danger clignotant", "palette.tile.danger_blink"},
        {"Pics vers le haut", "palette.tile.spikes_up"},
        {"Pics vers le bas", "palette.tile.spikes_down"},
        {"Pics vers la gauche", "palette.tile.spikes_left"},
        {"Pics vers la droite", "palette.tile.spikes_right"},
        {"Entrée", "palette.tile.entry"},
        {"Sortie", "palette.tile.exit"},
    };
    if (loc != nullptr) {
        if (const auto found = keys.find(label); found != keys.end()) {
            return QString::fromStdString(loc->text(found->second));
        }
    }
    return QString::fromStdString(label);  // repli : libellé source (français).
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
