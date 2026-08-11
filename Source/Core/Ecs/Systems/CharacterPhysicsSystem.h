#pragma once

#include "Core/Physics/PhysicsConfig.h"

/**
 * @file Core/Ecs/Systems/CharacterPhysicsSystem.h
 * @brief Système de physique du personnage : déplacement, gravité et collisions au pas fixe.
 */

namespace core {

class World;
class TileMap;
struct PlayerInput;

/**
 * @brief Fait évoluer le personnage jouable d'un pas fixe : intention horizontale, gravité
 *        continue, collisions **continues** contre les tuiles solides, et état « au sol ».
 *
 * Pour chaque entité portant ::core::Player, ::core::Transform, ::core::Velocity et
 * ::core::Collider, le système : applique la vitesse horizontale voulue (`EX-GP-010`), intègre la
 * gravité tant que le personnage n'est pas au sol (`EX-GP-012`), puis résout le déplacement par
 * balayage (`sweepAabb`, `EX-GP-014`) — aucune traversée de mur, glissement le long des surfaces.
 * Le saut (hauteur variable, coyote time, jump buffering, double saut, wall jump/slide, dash) est
 * également piloté par ce système à partir de `PlayerInput`/`PhysicsConfig` ; l'état `grounded`
 * conditionne ces mécaniques et est recalculé chaque pas.
 *
 * Contrairement au ::core::MovementSystem générique, ce système a besoin de la grille de collision
 * et de l'intention d'entrée : il expose donc sa propre signature d'`update`. Toute la logique vit
 * ici ; les composants restent des **données pures** (`EX-ARCH-011`). Déterministe au pas fixe
 * (`EX-NFR-002`).
 */
class CharacterPhysicsSystem {
public:
    /**
     * @brief Construit le système avec des réglages de physique donnés.
     * @param config Constantes de physique (vitesse, gravité, chute max). Valeurs par défaut si
     * omis.
     */
    explicit CharacterPhysicsSystem(PhysicsConfig config = {});

    /**
     * @brief Applique un pas de simulation à toutes les entités « personnage ».
     * @param world      Monde ECS (entités Player + Transform + Velocity + Collider).
     * @param tiles      Grille de collision : `isSolid(colonne, ligne)` désigne les tuiles
     * bloquantes.
     * @param input      Intention de déplacement de la frame (dissociée des touches).
     * @param fixedDelta Durée du pas de simulation, en secondes.
     */
    void update(World& world, const TileMap& tiles, const PlayerInput& input,
                float fixedDelta) const;

private:
    PhysicsConfig _config;
};

}  // namespace core
