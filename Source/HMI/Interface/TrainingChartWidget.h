// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>
#include <deque>

/**
 * @file HMI/Interface/TrainingChartWidget.h
 * @brief Courbes live de l'onglet Entraînement du Mode IA (`LOT-ANNEXE-21`, `LOT-73`) : meilleure
 * récompense, récompense moyenne, moyenne mobile et taux de réussite au fil des
 * générations/épisodes.
 *
 * Dessin `QPainter` minimal, sans dépendance à Qt Charts : quatre séries à échelle automatique,
 * mises à jour point par point pendant un run, jamais rechargées en bloc.
 *
 * Le graphique **se lit** : il porte ses graduations, les valeurs extrêmes de son axe des
 * récompenses et l'intervalle de générations couvert. Sans elles, une courbe qui monte n'apprend
 * que le signe d'une progression, jamais son ampleur — et deux runs ne se comparent pas.
 */

namespace hmi {

class TrainingChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrainingChartWidget(QWidget* parent = nullptr);

public slots:
    /**
     * @brief Ajoute un point (une génération/un épisode) aux quatre courbes.
     * @param generationIndex Index de génération/d'épisode, porté par l'axe des abscisses.
     * @param bestReward      Meilleure récompense du lot.
     * @param meanReward      Récompense moyenne du lot.
     * @param movingAverage   Moyenne mobile de la meilleure récompense
     *                        (`aisolver::TrainingStatsDerived`), la seule courbe sur laquelle une
     *                        progression lente se distingue du bruit d'un lot à l'autre.
     * @param successRate     Taux de réussite du lot, dans `[0, 1]`.
     */
    void addPoint(int generationIndex, double bestReward, double meanReward, double movingAverage,
                  double successRate);

    /// Efface les courbes (début d'un nouveau run).
    void clearChart();

    /// Libellé affiché tant qu'il n'y a pas assez de points pour tracer une courbe.
    ///
    /// Fourni par l'appelant, jamais via `tr()` : l'application traduit par son propre catalogue
    /// (`Source/Elements/Localization/*.lang`) et déploie Qt sans ses traductions
    /// (`windeployqt --no-translations`) — un `tr()` resterait donc en français en anglais.
    void setEmptyLabel(QString label);

    /// Libellés de légende (localisés, rejoués par `AiModeScreen::retranslateUi` comme le reste de
    /// l'écran) — un par courbe, dans l'ordre meilleure récompense / récompense moyenne / moyenne
    /// mobile / taux de réussite.
    void setSeriesLabels(const QString& bestReward, const QString& meanReward,
                         const QString& movingAverage, const QString& successRate);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    // Fenêtre glissante : un run à plusieurs milliers de générations ne doit ni faire grossir la
    // mémoire indéfiniment, ni écraser les points anciens sur le graphique (points trop serrés
    // pour rester lisibles) — les MAX_POINTS plus récents suffisent à montrer la tendance.
    static constexpr std::size_t MAX_POINTS = 500;

    std::deque<double> _bestReward;
    std::deque<double> _meanReward;
    std::deque<double> _movingAverage;
    std::deque<double> _successRate;
    /// Index de génération du premier et du dernier point encore affichés : la fenêtre glissante
    /// fait avancer le premier, que la seule taille des files ne suffirait pas à retrouver.
    int _firstIndex = 0;
    int _lastIndex = 0;

    QString _emptyLabel;
    QString _bestRewardLabel;
    QString _meanRewardLabel;
    QString _movingAverageLabel;
    QString _successRateLabel;
};

}  // namespace hmi
