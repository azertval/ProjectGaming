// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/TrainingChartWidget.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>
#include <utility>

namespace hmi {

namespace {

// Palette CATEGORIELLE des courbes. Volontairement locale, et non tirée de `hmi::DesignTokens` :
// les jetons de design décrivent des ROLES de surface (fond, bordure, accent, texte), pas des
// séries de données. Quatre courbes superposées posent un autre problème -- rester distinguables
// entre elles -- auquel un rôle de surface ne répond pas : prendre `accent` pour deux d'entre
// elles les rendrait indiscernables, et il n'existe aucun jeton pour les deux autres.
constexpr QColor BEST_REWARD_COLOR(80, 200, 120);
constexpr QColor MEAN_REWARD_COLOR(120, 160, 220);
constexpr QColor MOVING_AVERAGE_COLOR(235, 235, 245);
constexpr QColor SUCCESS_RATE_COLOR(230, 180, 60);

constexpr int MARGIN = 6;
// Bande réservée aux valeurs de l'axe des récompenses, à gauche de la zone de tracé. Assez large
// pour « -100.0 » ; les récompenses du projet tiennent dans cet ordre de grandeur (voir
// `aisolver::RewardConfig`).
constexpr int AXIS_LABEL_WIDTH = 46;
// Bande réservée aux numéros de génération, sous la zone de tracé.
constexpr int AXIS_LABEL_HEIGHT = 14;

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

// Arrondi d'affichage : deux décimales sous 100, aucune au-delà. Une récompense de 1234,57 n'a pas
// deux décimales utiles, et les afficher vole la largeur de la bande d'axe.
[[nodiscard]] QString formatAxisValue(double value) {
    return std::abs(value) < 100.0 ? QString::number(value, 'f', 2)
                                   : QString::number(value, 'f', 0);
}

// Graduations de l'axe des récompenses : trois valeurs (bas, milieu, haut) et leurs traits
// horizontaux. Trois et pas davantage : le graphique fait quelques centaines de pixels de haut, et
// une grille plus dense masquerait les courbes qu'elle sert a lire.
void drawRewardAxis(QPainter& painter, const QRectF& plotArea, double minValue, double maxValue,
                    const QColor& gridColor, const QColor& textColor) {
    constexpr int TICK_COUNT = 3;
    const QFontMetrics metrics(painter.font());
    for (int tick = 0; tick < TICK_COUNT; ++tick) {
        const double ratio = static_cast<double>(tick) / static_cast<double>(TICK_COUNT - 1);
        const double y = plotArea.bottom() - ratio * plotArea.height();
        const double value = minValue + ratio * (maxValue - minValue);

        painter.setPen(QPen(gridColor, 1.0));
        painter.drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));

        painter.setPen(textColor);
        const QRectF labelRect(plotArea.left() - AXIS_LABEL_WIDTH,
                               y - static_cast<double>(metrics.height()) / 2.0,
                               AXIS_LABEL_WIDTH - 4.0, static_cast<double>(metrics.height()));
        painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, formatAxisValue(value));
    }
}

// Bornes de l'axe des générations, sous la zone de tracé : la première et la dernière affichées.
// La fenêtre glissante fait avancer la première, qu'un lecteur ne peut pas deviner autrement.
void drawGenerationAxis(QPainter& painter, const QRectF& plotArea, int firstIndex, int lastIndex,
                        const QColor& textColor) {
    painter.setPen(textColor);
    const QRectF axisRect(plotArea.left(), plotArea.bottom() + 2.0, plotArea.width(),
                          static_cast<double>(AXIS_LABEL_HEIGHT));
    painter.drawText(axisRect, Qt::AlignLeft | Qt::AlignVCenter, QString::number(firstIndex));
    painter.drawText(axisRect, Qt::AlignRight | Qt::AlignVCenter, QString::number(lastIndex));
}

}  // namespace

TrainingChartWidget::TrainingChartWidget(QWidget* parent) : QWidget(parent) {}

void TrainingChartWidget::addPoint(int generationIndex, double bestReward, double meanReward,
                                   double movingAverage, double successRate) {
    if (_bestReward.empty()) {
        _firstIndex = generationIndex;
    }
    _lastIndex = generationIndex;

    _bestReward.push_back(bestReward);
    _meanReward.push_back(meanReward);
    _movingAverage.push_back(movingAverage);
    _successRate.push_back(successRate);
    while (_bestReward.size() > MAX_POINTS) {
        _bestReward.pop_front();
        _meanReward.pop_front();
        _movingAverage.pop_front();
        _successRate.pop_front();
        // Le premier index affiche avance AVEC la fenetre : sans cela, l'axe continuerait
        // d'annoncer une generation depuis longtemps sortie du graphique.
        ++_firstIndex;
    }
    update();
}

void TrainingChartWidget::clearChart() {
    _bestReward.clear();
    _meanReward.clear();
    _movingAverage.clear();
    _successRate.clear();
    _firstIndex = 0;
    _lastIndex = 0;
    update();
}

void TrainingChartWidget::setEmptyLabel(QString label) {
    _emptyLabel = std::move(label);
    update();
}

void TrainingChartWidget::setSeriesLabels(const QString& bestReward, const QString& meanReward,
                                          const QString& movingAverage,
                                          const QString& successRate) {
    _bestRewardLabel = bestReward;
    _meanRewardLabel = meanReward;
    _movingAverageLabel = movingAverage;
    _successRateLabel = successRate;
    update();
}

void TrainingChartWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), palette().base());
    painter.setPen(palette().text().color());

    const bool hasLegend = !_bestRewardLabel.isEmpty() || !_meanRewardLabel.isEmpty() ||
                           !_movingAverageLabel.isEmpty() || !_successRateLabel.isEmpty();
    const qreal legendHeight = hasLegend ? QFontMetrics(painter.font()).height() + 8.0 : 0.0;

    if (_bestReward.size() < 2) {
        painter.drawText(rect(), Qt::AlignCenter, _emptyLabel);
        return;
    }

    // Marge fixe pour laisser respirer les courbes contre les bords du widget ; la legende, si
    // presente, prend sa propre bande sous la zone de trace plutot que de chevaucher les courbes,
    // et les deux axes prennent la leur a gauche et en bas.
    const QRectF plotArea =
        rect().adjusted(MARGIN + AXIS_LABEL_WIDTH, MARGIN, -MARGIN,
                        -MARGIN - AXIS_LABEL_HEIGHT - static_cast<int>(legendHeight));
    if (plotArea.width() <= 1.0 || plotArea.height() <= 1.0) {
        return;  // widget trop petit pour porter ses axes : ne rien dessiner plutot que du bruit.
    }

    const auto extremesOf = [](const std::deque<double>& values) {
        const auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
        return std::pair{*minimum, *maximum};
    };
    const auto [bestMin, bestMax] = extremesOf(_bestReward);
    const auto [meanMin, meanMax] = extremesOf(_meanReward);
    const auto [averageMin, averageMax] = extremesOf(_movingAverage);
    // La moyenne mobile entre dans l'echelle : tracee hors bornes, elle sortirait du cadre.
    const double minReward = std::min({bestMin, meanMin, averageMin});
    const double maxReward = std::max({bestMax, meanMax, averageMax});

    const QColor gridColor = palette().mid().color();
    const QColor textColor = palette().text().color();
    drawRewardAxis(painter, plotArea, minReward, maxReward, gridColor, textColor);
    drawGenerationAxis(painter, plotArea, _firstIndex, _lastIndex, textColor);

    drawSeries(painter, plotArea, {&_bestReward, BEST_REWARD_COLOR, &_bestRewardLabel}, minReward,
               maxReward);
    drawSeries(painter, plotArea, {&_meanReward, MEAN_REWARD_COLOR, &_meanRewardLabel}, minReward,
               maxReward);
    drawSeries(painter, plotArea, {&_movingAverage, MOVING_AVERAGE_COLOR, &_movingAverageLabel},
               minReward, maxReward);
    // Taux de reussite : echelle propre [0, 1], independante de celle des recompenses.
    drawSeries(painter, plotArea, {&_successRate, SUCCESS_RATE_COLOR, &_successRateLabel}, 0.0,
               1.0);

    if (hasLegend) {
        // drawSeries a laisse la couleur de la derniere courbe sur le pinceau : le texte de la
        // legende doit rester dans la couleur de texte de la palette, pas dans une couleur de
        // courbe.
        painter.setPen(textColor);
        qreal x = plotArea.left();
        const qreal y = rect().bottom() - MARGIN;
        x = drawLegendEntry(painter, x, y, BEST_REWARD_COLOR, _bestRewardLabel);
        x = drawLegendEntry(painter, x, y, MEAN_REWARD_COLOR, _meanRewardLabel);
        x = drawLegendEntry(painter, x, y, MOVING_AVERAGE_COLOR, _movingAverageLabel);
        drawLegendEntry(painter, x, y, SUCCESS_RATE_COLOR, _successRateLabel);
    }
}

}  // namespace hmi
