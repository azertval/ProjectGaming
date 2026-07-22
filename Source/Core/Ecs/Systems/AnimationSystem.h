#pragma once

#include "Core/Ecs/ISystem.h"

/**
 * @file Core/Ecs/Systems/AnimationSystem.h
 * @brief Système dérivant le clip/l'image d'animation du personnage de son état physique.
 */

namespace core {

/**
 * @brief Fait évoluer l'état d'animation (`core::Animation`) de chaque personnage, au pas fixe.
 *
 * Pour chaque entité possédant `core::Player`, `core::Velocity` et `core::Animation` : détermine
 * le clip actif à partir de `Player::grounded` et de la vitesse horizontale (en l'air → `Jump` ;
 * au sol et en mouvement → `Run` ; au sol et immobile → `Idle`), puis avance l'image courante avec
 * bouclage selon le nombre d'images du clip (`core::IDLE_FRAME_COUNT`, etc.).
 *
 * Un changement de clip réinitialise immédiatement l'image à 0 (pas de transition progressive).
 * Toute la logique vit ici ; les composants restent des **données pures** (`EX-ARCH-011`).
 * Déterministe au pas fixe (`EX-NFR-002`), testable sans GPU (`EX-NFR-010`).
 *
 * Doit s'exécuter **après** `CharacterPhysicsSystem` dans la boucle d'un écran : il lit l'état
 * `grounded` calculé par la physique pour le **même** pas.
 */
class AnimationSystem : public ISystem {
public:
    /**
     * @brief Applique un pas de simulation d'animation à tous les personnages.
     * @param world      Monde ECS (entités Player + Velocity + Animation).
     * @param fixedDelta Durée du pas de simulation, en secondes.
     */
    void update(World& world, float fixedDelta) override;
};

}  // namespace core
