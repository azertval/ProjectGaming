#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

class QSoundEffect;

/**
 * @file HMI/Audio/AudioEngine.h
 * @brief Socle de lecture audio : enveloppe fine autour de `QSoundEffect` (`EX-REN-047`).
 */

namespace hmi {

/**
 * @brief Lecture d'échantillons courts à faible latence, avec volume global et repli muet.
 *
 * Enveloppe fine autour de `QSoundEffect` (Qt Multimedia) : ouverture du périphérique de sortie à
 * la construction, un `QSoundEffect` par échantillon préchargé, arrêt propre à la destruction
 * (RAII, `EX-NFR-041`). Aucune exception ne franchit cette frontière (`EX-NFR-040`) : un
 * périphérique absent, un fichier introuvable ou une lecture demandée alors que le moteur est
 * « muet » ne font jamais rien de plus qu'un avertissement journalisé une seule fois.
 *
 * **Aucune dépendance dans `Core`** : cette classe vit exclusivement dans `HMI`
 * (`EX-ARCH-012`, `EX-NFR-010`). La simulation expose des transitions d'état ; c'est ce moteur,
 * piloté par `HMI/Audio/SoundTriggers` (LOT-60 TACHE-03), qui décide qu'une transition fait du
 * bruit.
 */
class AudioEngine {
public:
    /// Étiquette du constructeur qui force l'état « muet », sans tenter d'ouvrir de périphérique —
    /// réservé aux tests : le comportement en l'absence de périphérique doit être vérifiable sans
    /// dépendre du matériel audio réel de la machine qui exécute les tests.
    enum class ForceMuted { Yes };

    /// Ouvre le périphérique de sortie par défaut. Un échec (aucun périphérique) place le moteur en
    /// état muet et journalise un avertissement une seule fois — jamais une exception.
    AudioEngine();

    /// Construit un moteur déjà muet, sans toucher au périphérique réel (tests, `ForceMuted::Yes`).
    explicit AudioEngine(ForceMuted);

    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&&) = delete;
    AudioEngine& operator=(AudioEngine&&) = delete;

    /// @return true si le moteur ne produira aucun son (périphérique absent, ou forcé pour les
    ///         tests).
    [[nodiscard]] bool muted() const noexcept {
        return _muted;
    }

    /**
     * @brief Règle le volume global, appliqué immédiatement à tous les échantillons préchargés.
     * @param volume Volume voulu. Hors [0, 1], ramené à l'extrémité la plus proche — jamais
     *        propagé tel quel (une valeur négative ou explosée ne doit jamais atteindre
     *        `QSoundEffect`).
     */
    void setVolume(float volume);

    /// @return Le volume global courant, toujours dans [0, 1].
    [[nodiscard]] float volume() const noexcept {
        return _volume;
    }

    /**
     * @brief Précharge un échantillon sous un identifiant logique.
     *
     * À appeler au démarrage pour chaque son du catalogue (`HMI/Audio/SoundCatalog`, TACHE-02) :
     * `QSoundEffect` charge son fichier de façon asynchrone, jouer immédiatement après
     * construction ne produit rien. Précharger ici, jamais au premier déclenchement.
     * @param id Identifiant logique (ex. "saut"). Un second appel avec le même identifiant
     *        remplace l'échantillon précédent.
     * @param file Chemin du fichier WAV. Un fichier absent ne fait pas échouer l'appel : la
     *        lecture ultérieure de cet identifiant restera silencieuse.
     */
    void preload(const std::string& id, const std::filesystem::path& file);

    /**
     * @brief Joue l'échantillon préchargé sous cet identifiant.
     *
     * Sans effet si le moteur est muet, si @p id n'a jamais été préchargé, ou si le fichier
     * associé n'a pas pu être chargé — jamais une erreur pour l'appelant (`EX-NFR-040`).
     * @param id Identifiant préchargé par `preload`.
     */
    void play(const std::string& id) const;

private:
    struct Sample {
        std::unique_ptr<QSoundEffect> effect;
    };

    std::unordered_map<std::string, Sample> _samples;
    float _volume = 1.0f;
    bool _muted = false;
};

}  // namespace hmi
