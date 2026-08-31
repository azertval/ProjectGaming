// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Ai/EvaluationWorker.h"

#include <QMetaType>
#include <optional>
#include <utility>

/**
 * @file HMI/Ai/EvaluationWorker.cpp
 * @brief Voir `EvaluationWorker.h` — mince habillage `QThread` autour de `hmi::evaluateModel`.
 */

namespace hmi {

EvaluationWorker::EvaluationWorker(EvaluationRequest request, QObject* parent)
    : QObject(parent), _request(std::move(request)) {
    qRegisterMetaType<EvaluationOutcome>();
}

void EvaluationWorker::requestStop() {
    _stopRequested.store(true);
}

void EvaluationWorker::run() {
    const std::optional<EvaluationOutcome> outcome =
        evaluateModel(_request, [this](int completed, int total) {
            emit progress(completed, total);
            return !_stopRequested.load();
        });
    if (!outcome.has_value()) {
        emit failed();
        return;
    }
    emit finished(*outcome);
}

}  // namespace hmi
