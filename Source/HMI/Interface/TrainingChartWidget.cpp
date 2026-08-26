// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/TrainingChartWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace hmi {

namespace {

// Une courbe, sa couleur ; successRate est dans [0,1] et dessinée sur sa propre échelle fixe
// (pas d'auto-échelle : un taux de réussite se lit toujours de 0% à 100%, quel que soit le run).
struct Series {
    const std::deque<double>* values;
    QColor color;
    bool fixedZeroToOneRange;
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

void TrainingChartWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), palette().base());

    if (_bestReward.size() < 2) {
        painter.setPen(palette().text().color());
        painter.drawText(rect(), Qt::AlignCenter, tr("Pas encore assez de données"));
        return;
    }

    // Marge fixe pour laisser respirer les courbes contre les bords du widget.
    constexpr int MARGIN = 6;
    const QRectF plotArea = rect().adjusted(MARGIN, MARGIN, -MARGIN, -MARGIN);

    const double minReward =
        std::min(*std::min_element(_bestReward.begin(), _bestReward.end()),
                 *std::min_element(_meanReward.begin(), _meanReward.end()));
    const double maxReward =
        std::max(*std::max_element(_bestReward.begin(), _bestReward.end()),
                 *std::max_element(_meanReward.begin(), _meanReward.end()));

    drawSeries(painter, plotArea, {&_bestReward, QColor(80, 200, 120), false}, minReward,
              maxReward);
    drawSeries(painter, plotArea, {&_meanReward, QColor(120, 160, 220), false}, minReward,
              maxReward);
    // Taux de réussite : échelle propre [0, 1], indépendante de celle des récompenses.
    drawSeries(painter, plotArea, {&_successRate, QColor(230, 180, 60), true}, 0.0, 1.0);
}

}  // namespace hmi
