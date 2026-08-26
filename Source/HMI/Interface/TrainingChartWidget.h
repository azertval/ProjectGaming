// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>
#include <deque>

/**
 * @file HMI/Interface/TrainingChartWidget.h
 * @brief Courbes live de l'onglet Entraînement du Mode IA (`LOT-ANNEXE-21`) : meilleure
 * récompense, récompense moyenne et taux de réussite au fil des générations/épisodes.
 *
 * Dessin `QPainter` minimal, sans dépendance à Qt Charts : trois séries à échelle automatique,
 * mises à jour point par point pendant un run, jamais rechargées en bloc.
 */

namespace hmi {

class TrainingChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrainingChartWidget(QWidget* parent = nullptr);

public slots:
    /// Ajoute un point (une génération/un épisode) aux trois courbes.
    void addPoint(double bestReward, double meanReward, double successRate);

    /// Efface les courbes (début d'un nouveau run).
    void clearChart();

    /// Libellés de légende (localisés, rejoués par `AiModeScreen::retranslateUi` comme le reste
    /// de l'écran) — un par courbe, dans l'ordre meilleure récompense / récompense moyenne / taux
    /// de réussite.
    void setSeriesLabels(const QString& bestReward, const QString& meanReward,
                         const QString& successRate);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    // Fenêtre glissante : un run à plusieurs milliers de générations ne doit ni faire grossir la
    // mémoire indéfiniment, ni écraser les points anciens sur le graphique (points trop serrés
    // pour rester lisibles) — les MAX_POINTS plus récents suffisent à montrer la tendance.
    static constexpr std::size_t MAX_POINTS = 500;

    std::deque<double> _bestReward;
    std::deque<double> _meanReward;
    std::deque<double> _successRate;

    QString _bestRewardLabel;
    QString _meanRewardLabel;
    QString _successRateLabel;
};

}  // namespace hmi
