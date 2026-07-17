#pragma once

/**
 * @file Core/Physics/PlayerInput.h
 * @brief Intention de déplacement du personnage, neutre vis-à-vis de l'entrée physique.
 */

namespace core {

/**
 * @brief **Intention** de déplacement transmise à la physique, dissociée de toute touche.
 *
 * Contrat entre la couche d'entrée (`HMI`, qui traduit clavier/manette — `EX-CTRL-010`) et la
 * simulation (`Core`, qui l'applique). `Core` ne connaît **que** cette intention : il ignore les
 * touches, ce qui garde la simulation testable et permettra remappage/manette sans le modifier.
 * Donnée pure, transitoire (re-remplie chaque frame), passée au `CharacterPhysicsSystem`.
 */
struct PlayerInput {
    /// Intention de déplacement horizontal, normalisée dans l'intervalle [-1, 1] :
    /// -1 = vers la gauche, +1 = vers la droite, 0 = immobile. 
    float moveX = 0.0f;
};

}  // namespace core
