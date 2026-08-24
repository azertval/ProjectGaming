// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AiSolver/Training/Advanced/ReplayBuffer.h"

#include "Core/Diagnostics/Assert.h"

namespace aisolver::training {

ReplayBuffer::ReplayBuffer(std::size_t capacity) : _capacity(capacity) {
    PROJECTGAMING_ASSERT(_capacity > 0,
                         "ReplayBuffer : la capacite doit etre strictement positive");
    _transitions.reserve(_capacity);
}

void ReplayBuffer::push(Transition transition) {
    if (_transitions.size() < _capacity) {
        _transitions.push_back(std::move(transition));
        return;
    }
    // Capacite atteinte : ecrase la transition la plus ancienne (tampon en anneau).
    _transitions[_writeIndex] = std::move(transition);
    _writeIndex = (_writeIndex + 1) % _capacity;
}

std::vector<Transition> ReplayBuffer::sample(std::size_t batchSize, Rng& rng) const {
    PROJECTGAMING_ASSERT(!_transitions.empty(), "ReplayBuffer::sample : le tampon est vide");

    std::vector<Transition> batch;
    batch.reserve(batchSize);
    const int lastIndex = static_cast<int>(_transitions.size()) - 1;
    for (std::size_t i = 0; i < batchSize; ++i) {
        const int index = rng.nextInt(0, lastIndex);
        batch.push_back(_transitions[static_cast<std::size_t>(index)]);
    }
    return batch;
}

}  // namespace aisolver::training
