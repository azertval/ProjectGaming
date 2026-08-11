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
struct Player;
struct Transform;
struct Velocity;
struct Collider;

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
    /// Étapes 0/0a-0d/1/2/2b/2bis de update() (voir son .cpp) : à partir de l'intention d'entrée
    /// et des minuteries de game feel du personnage, détermine la vitesse voulue pour ce pas
    /// (saut, dash, gravité effective, wall slide) -- ne touche ni `Transform` ni les tuiles,
    /// seule la résolution de collision (ci-dessous) en a besoin.
    void resolveVelocity(Player& player, Velocity& velocity, const PlayerInput& input,
                         float fixedDelta) const;
    /// Partie « 0d. Dash » de resolveVelocity() (voir son .cpp) : déclenche un dash si demandé et
    /// disponible, puis décompte sa durée en cours. @return true si le dash occupe encore ce pas
    /// (auquel cas la gravité/le saut/le déplacement horizontal normaux, gérés par
    /// resolveVelocity() APRÈS cet appel, ne doivent pas s'appliquer).
    [[nodiscard]] bool applyDash(Player& player, Velocity& velocity, const PlayerInput& input,
                                 float fixedDelta) const;
    /// Partie « 0a. Saut » de resolveVelocity() : consomme le buffer de saut si une source
    /// l'autorise (sol/coyote, mur, ou saut aérien, dans cet ordre).
    void applyJump(Player& player, Velocity& velocity) const;
    /// Étapes 3 à 9 de update() : à partir de la vitesse voulue (résolue par resolveVelocity()),
    /// balaie le déplacement contre les tuiles solides puis les pentes (sol/plafond), et met à
    /// jour l'état « au sol »/« contact mural » qui en découle.
    void resolveCollisionAndState(Player& player, Transform& transform, Velocity& velocity,
                                  const Collider& collider, const TileMap& tiles,
                                  const PlayerInput& input, float fixedDelta) const;

    PhysicsConfig _config;
};

}  // namespace core
