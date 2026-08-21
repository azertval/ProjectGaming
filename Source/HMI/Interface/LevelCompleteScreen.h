// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>
#include <memory>

/**
 * @file HMI/Interface/LevelCompleteScreen.h
 * @brief Écran de fin de niveau et de fin de séquence (LOT-59 TACHE-03). Mise en page dans
 *        `LevelCompleteScreen.ui` (Qt Designer).
 */

namespace Ui {
class LevelCompleteScreen;
}

namespace hmi {

class Localization;

/**
 * @brief Recouvrement affiché à la réussite d'un tableau (`EX-GP-030`), même patron de
 *        superposition que `PauseScreen` (widget frère de `MainWindow::_editorContainer`, jamais
 *        une page du `QStackedWidget`).
 *
 * Un seul écran paramétré pour les deux cas décrits par la tâche (« confondre ou non les deux
 * écrans est une décision d'implémentation ») : `configure()` choisit entre fin de **tableau**
 * (titre nommé, boutons Continuer/Rejouer/Retour) et fin de **séquence** (titre de fin, seul
 * Retour au menu -- `Continuer` n'a alors rien où mener). `ScreenId` n'a donc pas de troisième
 * état : les deux cas partagent `ScreenId::NiveauTermine`.
 */
class LevelCompleteScreen : public QWidget {
    Q_OBJECT

public:
    explicit LevelCompleteScreen(QWidget* parent = nullptr);
    ~LevelCompleteScreen() override;

    /// Configure l'écran avant affichage : @p sequenceComplete choisit la variante (fin de
    /// séquence si le tableau réussi était le dernier, `GameViewport::isLastGameLevel`) ;
    /// @p levelName est le nom du tableau réussi (ignoré en fin de séquence). Réapplique aussitôt
    /// les textes si `retranslateUi` a déjà été appelé.
    void configure(bool sequenceComplete, const QString& levelName);

    /// Renseigne le bilan du tableau (`LOT-68`) : durée mise en forme, morts, sauts. Appelée avant
    /// `configure` ou après, indifféremment — les deux écrivent des libellés distincts.
    void setRunSummary(const QString& elapsed, int deaths, int jumps);
    /// Applique la langue active aux libellés, y compris le titre dynamique (dernière
    /// configuration passée à `configure`, langue par défaut au tout premier appel).
    void retranslateUi(const Localization& loc);
    /// Donne le focus clavier au premier bouton pertinent pour la variante courante (« Continuer »
    /// en fin de tableau, « Retour au menu » en fin de séquence, où c'est le seul bouton visible).
    void focusDefaultAction();

signals:
    void continueRequested();
    void replayRequested();
    void returnToMenuRequested();

private:
    /// Réapplique titre + libellés depuis `_loc` (si déjà connu) et l'état courant
    /// (`_sequenceComplete`/`_levelName`) -- factorisé entre `configure` et `retranslateUi`.
    void applyTexts();

    std::unique_ptr<Ui::LevelCompleteScreen> _ui;
    const Localization* _loc = nullptr;
    bool _sequenceComplete = false;
    QString _levelName;
    /// Bilan du tableau (`LOT-68`), retenu pour être réécrit à chaque changement de langue —
    /// comme `_levelName` : les libellés sont traduits, les chiffres non.
    QString _elapsed;
    int _deaths = 0;
    int _jumps = 0;
};

}  // namespace hmi
