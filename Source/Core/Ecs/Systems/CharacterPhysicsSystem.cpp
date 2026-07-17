#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"

#include <algorithm>  // std::min (borne de vitesse de chute)

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/TileMap.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/SweptCollision.h"

namespace core {

CharacterPhysicsSystem::CharacterPhysicsSystem(PhysicsConfig config) : _config(config) {}

// Applique un pas de simulation aux personnages (voir en-tête).
void CharacterPhysicsSystem::update(World& world, const TileMap& tiles, const PlayerInput& input,
                                    float fixedDelta) {
    world.view<Player, Transform, Velocity, Collider>().each(
        [&](Entity, Player& player, Transform& transform, Velocity& velocity, Collider& collider) {
            //  Pour CHAQUE personnage :
            //   1. Vitesse horizontale voulue (pas d'inertie au MVP, on écrase X) :
            velocity.value.x = input.moveX * _config.moveSpeed;

            //   2. Gravité (y vers le bas → tomber = y positif), puis borne de chute :
            velocity.value.y += _config.gravity * fixedDelta;
            velocity.value.y = std::min(velocity.value.y, _config.maxFallSpeed);

            //   3. Déplacement voulu sur ce pas :
            const Vector2 delta = velocity.value * fixedDelta;

            //   4. Boîte du personnage (convention coin haut-gauche + taille, cf. Collider) :
            const Aabb box = Aabb::fromTopLeftSize(transform.position, collider.size);

            //   5. Résolution CONTINUE contre les tuiles solides :
            const SweepResult result = sweepAabb(box, delta, tiles);

            //   6. Applique la position résolue :
            transform.position = result.position;

            //   7. Annule la vitesse sur les axes bloqués (choc mur / sol / plafond) :
            if (result.normal.x != 0.0f) {
                velocity.value.x = 0.0f;
            }
            if (result.normal.y != 0.0f) {
                velocity.value.y = 0.0f;
            }

            //   8. État « au sol » = contact SOUS le personnage (normale vers le haut, y < 0) :
            player.grounded = (result.normal.y < 0.0f);
        });
}

}  // namespace core
