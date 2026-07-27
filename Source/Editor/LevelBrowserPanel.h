#pragma once

#include <filesystem>

#include <QString>
#include <QWidget>

/**
 * @file Editor/LevelBrowserPanel.h
 * @brief Panneau « Niveaux » : liste, recherche et gestion des fichiers de niveaux (LOT-36).
 */

class QLineEdit;
class QListView;
class QModelIndex;
class QSortFilterProxyModel;
class QStandardItemModel;

namespace editor {

/**
 * @brief Panneau de gestion des niveaux : liste filtrable + créer/renommer/dupliquer/supprimer.
 *
 * Liste les fichiers `.json` d'un dossier (`EX-IHM-020`), avec **recherche** incrémentale
 * (`QSortFilterProxyModel`) pour rester lisible quel que soit leur nombre. Les opérations
 * (`EX-IHM-021`) délèguent à `editor::LevelFileOperations` (couche pure, validée/testée) ; les
 * erreurs sont signalées à l'utilisateur, jamais silencieuses. Un double-clic (ou « Ouvrir ») émet
 * `levelOpenRequested` — le garde-fou des modifications non enregistrées est appliqué par
 * l'appelant (`MainWindow`).
 */
class LevelBrowserPanel : public QWidget {
    Q_OBJECT

public:
    explicit LevelBrowserPanel(std::filesystem::path levelsDir, QWidget* parent = nullptr);

    /// Recharge la liste depuis le dossier (après une opération ou un changement externe).
    void refresh();

signals:
    /// Émis quand l'utilisateur demande l'ouverture d'un niveau (chemin absolu du fichier).
    void levelOpenRequested(const QString& path);

private:
    void onNew();
    void onRename();
    void onDuplicate();
    void onDelete();
    void onActivated(const QModelIndex& index);

    /// Chemin du niveau sélectionné, ou chemin vide si aucune sélection.
    [[nodiscard]] std::filesystem::path selectedPath() const;

    std::filesystem::path _dir;
    QListView* _view;
    QStandardItemModel* _model;
    QSortFilterProxyModel* _proxy;
    QLineEdit* _search;
};

}  // namespace editor
