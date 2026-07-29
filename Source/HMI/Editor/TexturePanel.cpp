#include "HMI/Editor/TexturePanel.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QHeaderView>
#include <QList>
#include <QModelIndex>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVariant>

#include <utility>

#include "HMI/Editor/SkinAssignments.h"
#include "HMI/Editor/TaxonomyLabels.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Localization/Localization.h"
#include "ui_TexturePanel.h"

namespace hmi {

namespace {

// Colonnes de l'arbre : le type, le fichier assigne, et son mode de decoupage.
constexpr int COLUMN_TYPE = 0;
constexpr int COLUMN_ASSET = 1;
constexpr int COLUMN_MODE = 2;
constexpr int COLUMN_COUNT = 3;

// Role portant le core::TileType d'une ligne (les en-tetes de section n'en ont pas).
constexpr int TILE_TYPE_ROLE = Qt::UserRole + 1;

// Valeur affichee pour « aucun skin assigne ». Traduite : c'est une entree de liste comme une
// autre pour l'utilisateur.
[[nodiscard]] QString noneLabel(const Localization* loc) {
    return loc == nullptr ? QStringLiteral("(aucun)")
                          : QString::fromStdString(loc->text("textures.none"));
}

// Editeur en liste fermee, dont les choix sont portes par la cellule (role Qt::UserRole).
// Le delegue par defaut de Qt offrirait un champ de saisie libre : l'utilisateur pourrait y taper
// un nom de fichier inexistant, ce que le cadrage du lot exclut explicitement (selection par
// balayage de dossier, jamais de saisie de chemin).
class ChoiceDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/,
                          const QModelIndex& index) const override {
        auto* const combo = new QComboBox(parent);
        combo->addItems(index.data(Qt::UserRole).toStringList());
        return combo;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (auto* const combo = qobject_cast<QComboBox*>(editor)) {
            combo->setCurrentText(index.data(Qt::EditRole).toString());
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        if (auto* const combo = qobject_cast<QComboBox*>(editor)) {
            model->setData(index, combo->currentText(), Qt::EditRole);
        }
    }
};

}  // namespace

TexturePanel::TexturePanel(std::filesystem::path skinsDirectory, std::filesystem::path catalogPath,
                           QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::TexturePanel>()),
      _model(new QStandardItemModel(this)),
      _skinsDirectory(std::move(skinsDirectory)),
      _catalogPath(std::move(catalogPath)) {
    _ui->setupUi(this);
    _model->setColumnCount(COLUMN_COUNT);
    _ui->skinsTree->setModel(_model);
    _ui->skinsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Fichier et mode se choisissent dans une liste fermee, jamais en saisie libre.
    _ui->skinsTree->setItemDelegateForColumn(COLUMN_ASSET, new ChoiceDelegate(this));
    _ui->skinsTree->setItemDelegateForColumn(COLUMN_MODE, new ChoiceDelegate(this));

    connect(_ui->setSelector, &QComboBox::currentIndexChanged, this, &TexturePanel::onSetChanged);
    connect(_model, &QStandardItemModel::itemChanged, this, &TexturePanel::onItemChanged);
}

TexturePanel::~TexturePanel() = default;

void TexturePanel::setCatalog(SkinCatalog* catalog) {
    _catalog = catalog;
    _currentSet = catalog == nullptr ? std::string{} : catalog->defaultSetName();
    rebuildTree();
}

void TexturePanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->setLabel->setText(QString::fromStdString(loc.text("textures.skin_set")));
    _ui->sections->setTabText(0, QString::fromStdString(loc.text("textures.section_skins")));
    rebuildTree();
}

// Reconstruit entierement l'arbre depuis le catalogue et le contenu du dossier de skins.
void TexturePanel::rebuildTree() {
    // Garde de reentrance : peupler le modele emet itemChanged pour chaque cellule, ce qui
    // reecrirait le catalogue avec les valeurs qu'on vient d'y lire.
    _updating = true;
    _model->clear();
    _model->setColumnCount(COLUMN_COUNT);
    if (_loc != nullptr) {
        _model->setHorizontalHeaderLabels(
            QStringList{QString::fromStdString(_loc->text("textures.column_type")),
                        QString::fromStdString(_loc->text("textures.column_asset")),
                        QString::fromStdString(_loc->text("textures.column_mode"))});
    }

    // Liste des fichiers assignables, par balayage : aucune saisie de chemin par l'utilisateur.
    _assets = listSkinAssets(_skinsDirectory);

    // Selecteur de jeu, resynchronise sans emettre de changement de selection parasite.
    const QSignalBlocker blocker(_ui->setSelector);
    _ui->setSelector->clear();
    if (_catalog != nullptr) {
        for (const std::string& name : _catalog->setNames()) {
            _ui->setSelector->addItem(QString::fromStdString(name));
        }
        _ui->setSelector->setCurrentText(QString::fromStdString(_currentSet));
    }

    if (_catalog != nullptr) {
        for (const SkinSection& section : buildSkinRows(*_catalog, _currentSet)) {
            auto* const header =
                new QStandardItem(QString::fromStdString(localizedTaxonomyLabel(_loc, section.label)));
            header->setEditable(false);
            for (const SkinRow& row : section.rows) {
                auto* const typeItem = new QStandardItem(
                    QString::fromStdString(localizedTaxonomyLabel(_loc, row.typeLabel)));
                typeItem->setEditable(false);
                typeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                // Le fichier et le mode sont des listes fermees : l'utilisateur choisit, il ne
                // saisit pas. Les valeurs possibles sont portees par l'item (delegue de Qt).
                QStringList assetChoices{noneLabel(_loc)};
                for (const std::string& asset : _assets) {
                    assetChoices << QString::fromStdString(asset);
                }
                auto* const assetItem = new QStandardItem(
                    row.asset.empty() ? noneLabel(_loc) : QString::fromStdString(row.asset));
                assetItem->setData(assetChoices, Qt::UserRole);
                assetItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                auto* const modeItem =
                    new QStandardItem(QString::fromLatin1(skinModeName(row.mode)));
                modeItem->setData(QStringList{QString::fromLatin1(skinModeName(SkinMode::Single)),
                                              QString::fromLatin1(
                                                  skinModeName(SkinMode::Bitmask16))},
                                  Qt::UserRole);
                modeItem->setData(static_cast<int>(row.type), TILE_TYPE_ROLE);

                header->appendRow(QList<QStandardItem*>{typeItem, assetItem, modeItem});
            }
            _model->appendRow(header);
        }
    }

    _ui->skinsTree->expandAll();
    _updating = false;
}

// Change le jeu de skins courant.
void TexturePanel::onSetChanged(int index) {
    if (_updating || _catalog == nullptr || index < 0) {
        return;
    }
    _currentSet = _ui->setSelector->itemText(index).toStdString();
    rebuildTree();
    emit assignmentsChanged();
}

// Applique la modification d'une cellule (fichier ou mode) au catalogue, puis l'enregistre.
void TexturePanel::onItemChanged(QStandardItem* item) {
    if (_updating || _catalog == nullptr || item == nullptr) {
        return;
    }
    const QVariant typeData = item->data(TILE_TYPE_ROLE);
    if (!typeData.isValid()) {
        return;  // en-tete de section : rien a assigner.
    }
    const auto type = static_cast<core::TileType>(typeData.toInt());

    // L'etat cible est lu sur la LIGNE entiere, pas sur la seule cellule modifiee : changer le mode
    // ne doit pas effacer le fichier, ni l'inverse.
    QStandardItem* const parent = item->parent();
    if (parent == nullptr) {
        return;
    }
    const int row = item->row();
    const QStandardItem* const assetItem = parent->child(row, COLUMN_ASSET);
    const QStandardItem* const modeItem = parent->child(row, COLUMN_MODE);
    if (assetItem == nullptr || modeItem == nullptr) {
        return;
    }

    const QString assetText = assetItem->text();
    const std::string asset = assetText == noneLabel(_loc) ? std::string{} : assetText.toStdString();
    const SkinMode mode =
        skinModeFromName(modeItem->text().toStdString()).value_or(SkinMode::Single);

    applySkinAssignment(*_catalog, _currentSet, type, asset, mode);
    save();
    emit assignmentsChanged();
}

// Enregistre le catalogue au chemin deploye, comme l'enregistrement d'un niveau.
void TexturePanel::save() {
    if (_catalog == nullptr) {
        return;
    }
    if (!_catalog->saveToFile(_catalogPath)) {
        // Echec recuperable : l'assignation reste active en memoire et se voit a l'ecran, elle ne
        // survivra simplement pas au redemarrage.
        GRAPHICS_LOG_WARNING("Panneau Textures : enregistrement de " + _catalogPath.string() +
                             " impossible.");
    }
}

}  // namespace hmi
