#include "HMI/Editor/LinkPanel.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTableView>
#include <QVBoxLayout>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

constexpr int COLUMN_COUNT = 3;

// Libellé "(colonne, ligne)" d'une position de grille, pour affichage dans le tableau.
QString positionText(core::GridPosition position) {
    return QStringLiteral("(%1, %2)").arg(position.column).arg(position.row);
}

}  // namespace

LinkPanel::LinkPanel(QWidget* parent)
    : QWidget(parent),
      _table(new QTableView(this)),
      _model(new QStandardItemModel(0, COLUMN_COUNT, this)),
      _deleteButton(new QPushButton(this)) {
    _table->setModel(_model);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->verticalHeader()->setVisible(false);
    _model->setHorizontalHeaderLabels(
        {QStringLiteral("Type"), QStringLiteral("Déclencheur"), QStringLiteral("Cible")});

    _deleteButton->setText(QStringLiteral("Supprimer"));
    _deleteButton->setEnabled(false);

    connect(_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { onSelectionChanged(); });
    connect(_deleteButton, &QPushButton::clicked, this, [this] { onDeleteClicked(); });

    auto* const layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_table);
    layout->addWidget(_deleteButton);
}

void LinkPanel::refresh(const core::LevelDraft& draft) {
    _rows = buildLinkRows(draft);
    rebuildRows();
}

void LinkPanel::rebuildRows() {
    _model->removeRows(0, _model->rowCount());
    for (const LinkRow& row : _rows) {
        const bool mechanism = row.kind == LinkKind::Mechanism;
        const QString typeLabel =
            _loc != nullptr
                ? QString::fromStdString(
                      _loc->text(mechanism ? "link.type.mechanism" : "link.type.danger"))
                : (mechanism ? QStringLiteral("Mécanisme") : QStringLiteral("Danger commuté"));
        auto* const typeItem = new QStandardItem(typeLabel);
        auto* const triggerItem = new QStandardItem(positionText(row.trigger));
        auto* const targetItem = new QStandardItem(positionText(row.target));
        typeItem->setEditable(false);
        triggerItem->setEditable(false);
        targetItem->setEditable(false);
        _model->appendRow({typeItem, triggerItem, targetItem});
    }
    _deleteButton->setEnabled(false);
}

void LinkPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _model->setHorizontalHeaderLabels({
        QString::fromStdString(loc.text("link.column.type")),
        QString::fromStdString(loc.text("link.column.trigger")),
        QString::fromStdString(loc.text("link.column.target")),
    });
    _deleteButton->setText(QString::fromStdString(loc.text("link.delete_button")));
    rebuildRows();  // réapplique les libellés de type (mécanisme/danger) traduits.
}

void LinkPanel::onSelectionChanged() {
    const QModelIndexList selected = _table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        _deleteButton->setEnabled(false);
        emit linkSelected(std::nullopt);
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _rows.size()) {
        _deleteButton->setEnabled(false);
        emit linkSelected(std::nullopt);
        return;
    }
    _deleteButton->setEnabled(true);
    const LinkRow& selectedRow = _rows[static_cast<std::size_t>(row)];
    emit linkSelected(std::make_pair(selectedRow.trigger, selectedRow.target));
}

void LinkPanel::onDeleteClicked() {
    const QModelIndexList selected = _table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _rows.size()) {
        return;
    }
    emit deleteRequested(_rows[static_cast<std::size_t>(row)].target);
}

}  // namespace hmi
