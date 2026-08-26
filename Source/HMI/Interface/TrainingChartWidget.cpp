// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/TrainingChartWidget.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace hmi {

namespace {

// Une courbe, sa couleur, son libellé de légende ; successRate est dans [0,1] et dessinée sur sa
// propre échelle fixe (pas d'auto-échelle : un taux de réussite se lit toujours de 0% à 100%,
// quel que soit le run).
struct Series {
    const std::deque<double>* values;
    QColor color;
    const QString* label;
};

void drawSeries(QPainter& painter, const QRectF& plotArea, const Series& series, double minValue,
                double maxValue) {
    if (series.values->size() < 2) {
        return;
    }
    const double range = std::max(maxValue - minValue, 1e-6);
    QPainterPath path;
    const double stepX = plotArea.width() / static_cast<double>(series.values->size() - 1);
    for (std::size_t index = 0; index < series.values->size(); ++index) {
        const double value = (*series.values)[index];
        const double normalized = (value - minValue) / range;
        const double x = plotArea.left() + static_cast<double>(index) * stepX;
        const double y = plotArea.bottom() - normalized * plotArea.height();
        if (index == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }
    painter.setPen(QPen(series.color, 2.0));
    painter.drawPath(path);
}

// Ligne de légende (carré de couleur + libellé) sous la zone de tracé : un swatch par courbe
// nommée, dans l'ordre de passage, espacés horizontalement selon la largeur de leur propre texte.
qreal drawLegendEntry(QPainter& painter, qreal x, qreal y, const QColor& color,
                      const QString& label) {
    if (label.isEmpty()) {
        return x;
    }
    constexpr qreal SWATCH_SIZE = 10.0;
    constexpr qreal SWATCH_TEXT_GAP = 4.0;
    constexpr qreal ENTRY_GAP = 12.0;

    painter.fillRect(QRectF(x, y - SWATCH_SIZE, SWATCH_SIZE, SWATCH_SIZE), color);
    const QRectF textRect(x + SWATCH_SIZE + SWATCH_TEXT_GAP, y - SWATCH_SIZE, 1000.0, SWATCH_SIZE);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, label);
    const qreal textWidth = QFontMetrics(painter.font()).horizontalAdvance(label);
    return x + SWATCH_SIZE + SWATCH_TEXT_GAP + textWidth + ENTRY_GAP;
}

}  // namespace

TrainingChartWidget::TrainingChartWidget(QWidget* parent) : QWidget(parent) {}

void TrainingChartWidget::addPoint(double bestReward, double meanReward, double successRate) {
    _bestReward.push_back(bestReward);
    _meanReward.push_back(meanReward);
    _successRate.push_back(successRate);
    while (_bestReward.size() > MAX_POINTS) {
        _bestReward.pop_front();
        _meanReward.pop_front();
        _successRate.pop_front();
    }
    update();
}

void TrainingChartWidget::clearChart() {
    _bestReward.clear();
    _meanReward.clear();
    _successRate.clear();
    update();
}

void TrainingChartWidget::setSeriesLabels(const QString& bestReward, const QString& meanReward,
                                          const QString& successRate) {
    _bestRewardLabel = bestReward;
    _meanRewardLabel = meanReward;
    _successRateLabel = successRate;
    update();
}

void TrainingChartWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), palette().base());
    painter.setPen(palette().text().color());

    const bool hasLegend =
        !_bestRewardLabel.isEmpty() || !_meanRewardLabel.isEmpty() || !_successRateLabel.isEmpty();
    const qreal legendHeight = hasLegend ? QFontMetrics(painter.font()).height() + 8.0 : 0.0;

    if (_bestReward.size() < 2) {
        painter.drawText(rect(), Qt::AlignCenter, tr("Pas encore assez de données"));
        return;
    }

    // Marge fixe pour laisser respirer les courbes contre les bords du widget ; la légende, si
    // present, prend sa propre bande sous la zone de tracé plutôt que de chevaucher les courbes.
    constexpr int MARGIN = 6;
    const QRectF plotArea =
        rect().adjusted(MARGIN, MARGIN, -MARGIN, -MARGIN - static_cast<int>(legendHeight));

    const double minReward =
        std::min(*std::min_element(_bestReward.begin(), _bestReward.end()),
                 *std::min_element(_meanReward.begin(), _meanReward.end()));
    const double maxReward =
        std::max(*std::max_element(_bestReward.begin(), _bestReward.end()),
                 *std::max_element(_meanReward.begin(), _meanReward.end()));

    const QColor bestColor(80, 200, 120);
    const QColor meanColor(120, 160, 220);
    const QColor successColor(230, 180, 60);

    drawSeries(painter, plotArea, {&_bestReward, bestColor, &_bestRewardLabel}, minReward,
              maxReward);
    drawSeries(painter, plotArea, {&_meanReward, meanColor, &_meanRewardLabel}, minReward,
              maxReward);
    // Taux de réussite : échelle propre [0, 1], indépendante de celle des récompenses.
    drawSeries(painter, plotArea, {&_successRate, successColor, &_successRateLabel}, 0.0, 1.0);

    if (hasLegend) {
        // drawSeries a laisse la couleur de la derniere courbe sur le pinceau : le texte de la
        // legende doit rester dans la couleur de texte de la palette, pas dans une couleur de
        // courbe.
        painter.setPen(palette().text().color());
        qreal x = plotArea.left();
        const qreal y = rect().bottom() - MARGIN;
        x = drawLegendEntry(painter, x, y, bestColor, _bestRewardLabel);
        x = drawLegendEntry(painter, x, y, meanColor, _meanRewardLabel);
        drawLegendEntry(painter, x, y, successColor, _successRateLabel);
    }
}

}  // namespace hmi
