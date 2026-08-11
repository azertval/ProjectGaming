#include "HMI/Audio/AudioEngine.h"

#include <QAudioDevice>
#include <QMediaDevices>
#include <QSoundEffect>
#include <QString>
#include <QUrl>
#include <algorithm>

#include "HMI/Audio/AudioLog.h"

namespace hmi {

namespace {
[[nodiscard]] float clampVolume(float volume) noexcept {
    return std::clamp(volume, 0.0f, 1.0f);
}
}  // namespace

AudioEngine::AudioEngine() {
    // QMediaDevices::defaultAudioOutput() renvoie un QAudioDevice nul si aucun périphérique de
    // sortie n'est disponible (machine sans carte son, pilote absent) : c'est le seul signal
    // fiable avant toute tentative de lecture, vérifiable sans construire le moindre QSoundEffect.
    if (QMediaDevices::defaultAudioOutput().isNull()) {
        _muted = true;
        AUDIO_LOG_WARNING("Aucun peripherique de sortie audio detecte : moteur audio muet.");
    }
}

AudioEngine::AudioEngine(ForceMuted) : _muted(true) {}

AudioEngine::~AudioEngine() = default;

void AudioEngine::setVolume(float volume) {
    _volume = clampVolume(volume);
    for (auto& entry : _samples) {
        if (entry.second.effect) {
            entry.second.effect->setVolume(static_cast<double>(_volume));
        }
    }
}

void AudioEngine::preload(const std::string& id, const std::filesystem::path& file) {
    if (_muted) {
        return;
    }
    auto effect = std::make_unique<QSoundEffect>();
    effect->setSource(QUrl::fromLocalFile(QString::fromStdString(file.string())));
    effect->setVolume(static_cast<double>(_volume));
    _samples[id] = Sample{std::move(effect)};
}

void AudioEngine::play(const std::string& id) const {
    if (_muted) {
        return;
    }
    const auto it = _samples.find(id);
    if (it == _samples.end() || !it->second.effect) {
        return;
    }
    it->second.effect->play();
}

}  // namespace hmi
