#include "Core/Time/FixedTimestep.h"

namespace core {

/**
 * @brief Construit un cadenceur.
 * @param fixedDeltaSeconds   Durée d'un pas de simulation, en secondes (défaut : 1/60).
 * @param maximumStepsPerCall Nombre maximal de pas restitués par appel à advance() (anti-spirale).
 */
FixedTimestep::FixedTimestep(float fixedDeltaSeconds, int maximumStepsPerCall)
    : _fixedDeltaSeconds(fixedDeltaSeconds),
      _maximumStepsPerCall(maximumStepsPerCall),
      _accumulator(0.0f) {
}

/**
 * @brief Accumule le temps écoulé et renvoie le nombre de pas fixes à exécuter.
 * @param elapsedSeconds Temps réel écoulé depuis le dernier appel, en secondes.
 * @return Nombre de pas de simulation à exécuter (0 si le seuil n'est pas atteint).
 */
int FixedTimestep::advance(float elapsedSeconds) {
    // Un temps écoulé négatif ou nul n'apporte aucun pas : on ignore les valeurs
    // aberrantes (horloge non monotone) plutôt que de reculer l'accumulateur.
    if (elapsedSeconds > 0.0f) {
        _accumulator += elapsedSeconds;
    }

    // Découpe l'accumulateur en pas fixes, en s'arrêtant au plafond pour ne pas
    // bloquer l'appelant sur une rafale de rattrapage.
    int steps = 0;
    while (_accumulator >= _fixedDeltaSeconds && steps < _maximumStepsPerCall) {
        _accumulator -= _fixedDeltaSeconds;
        ++steps;
    }

    // Plafond atteint : il reste plus de temps que ce que l'on peut rattraper.
    // On abandonne ce retard (reset de l'accumulateur) pour éviter la spirale de
    // la mort, où chaque frame accumulerait davantage qu'elle ne consomme.
    if (steps == _maximumStepsPerCall && _accumulator >= _fixedDeltaSeconds) {
        _accumulator = 0.0f;
    }

    return steps;
}

/**
 * @brief Durée d'un pas de simulation.
 * @return Le pas fixe, en secondes.
 */
float FixedTimestep::fixedDeltaSeconds() const {
    return _fixedDeltaSeconds;
}

/**
 * @brief Fraction de pas restant dans l'accumulateur, pour l'interpolation du rendu.
 * @return Un facteur dans l'intervalle [0, 1[.
 */
float FixedTimestep::interpolationAlpha() const {
    return _accumulator / _fixedDeltaSeconds;
}

}  // namespace core
