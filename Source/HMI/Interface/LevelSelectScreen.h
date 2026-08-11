#pragma once

#include <QString>
#include <QWidget>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * @file HMI/Interface/LevelSelectScreen.h
 * @brief Écran de sélection de niveau côté joueur (LOT-59 TACHE-06, EX-IHM-005). Mise en page dans
 *        `LevelSelectScreen.ui` (Qt Designer).
 */

namespace Ui {
class LevelSelectScreen;
}

namespace hmi {

class Localization;
class Progression;

/// État d'un tableau de séquence dans la liste (`LevelSelectScreen::setSequenceLevels`) --
/// résolu une fois par `hmi::isLevelUnlocked`/`Progression::isCompleted`, jamais recalculé par ce
/// widget (qui ne connaît ni `Progression` ni la règle de déverrouillage).
enum class LevelSelectState {
    Completed,
    Current,  ///< Jouable, pas encore terminé (le tableau atteint).
    Locked,
};

/**
 * @brief Écran (page du `QStackedWidget`, pas un recouvrement -- atteint depuis le menu, jamais en
 *        jeu) listant les tableaux de la séquence démo avec leur état, plus les niveaux
 *        **personnels** du dossier (créés dans l'éditeur, hors séquence et hors progression).
 *
 * N'émet que des intentions ; `MainWindow` **revalide** tout choix de tableau de séquence via
 * `hmi::isLevelUnlocked` avant de le lancer (défense en profondeur, `EX-IHM-005`) -- cet écran
 * grise déjà les tableaux verrouillés (`Qt::ItemIsEnabled`), mais n'est pas l'unique garde.
 */
class LevelSelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit LevelSelectScreen(QWidget* parent = nullptr);
    ~LevelSelectScreen() override;

    /// Applique la langue active aux libellés (titre, onglets, bouton, suffixes d'état) --
    /// réapplique aussi l'état déjà connu (dernier appel à `setSequenceLevels`) pour que les
    /// suffixes restent traduits si la langue change pendant que l'écran est affiché.
    void retranslateUi(const Localization& loc);

    /// Remplit l'onglet Séquence : une ligne par tableau de @p sequence, dans l'ordre, avec son
    /// état (`hmi::isLevelUnlocked`/`Progression::isCompleted`) -- terminé, atteint (jouable, non
    /// terminé) ou verrouillé (grisé, non activable, ni au clic ni au clavier/manette).
    void setSequenceLevels(const std::vector<std::string>& sequence,
                           const Progression& progression);
    /// Remplit l'onglet Niveaux personnels : une ligne par chemin de @p levels
    /// (`hmi::LevelFileOperations::list()`).
    void setPersonalLevels(const std::vector<std::filesystem::path>& levels);

    /// Donne le focus clavier au bouton Retour.
    void focusDefaultAction();

signals:
    void backRequested();
    /// Un tableau de la séquence a été activé (double-clic/Entrée). @p levelName est le nom de
    /// fichier tel que passé à `setSequenceLevels`. Jamais émis pour un tableau verrouillé (item
    /// désactivé, `Qt::ItemIsEnabled` absent) -- `MainWindow` revalide quand même.
    void sequenceLevelChosen(QString levelName);
    /// Un niveau personnel a été activé. @p path est son chemin complet.
    void personalLevelChosen(QString path);

private:
    /// Réaffiche `_sequenceEntries` dans `sequenceList`, factorisé entre `setSequenceLevels` et
    /// `retranslateUi` (même contenu, suffixes d'état traduits différemment).
    void rebuildSequenceList(const Localization& loc);

    std::unique_ptr<Ui::LevelSelectScreen> _ui;
    /// Dernier état connu de la séquence (`setSequenceLevels`), pour réappliquer les suffixes
    /// d'état traduits sur un changement de langue (`retranslateUi`) sans redemander l'appelant.
    std::vector<std::pair<std::string, LevelSelectState>> _sequenceEntries;
    const Localization* _loc = nullptr;
};

}  // namespace hmi
