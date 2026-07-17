#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"

#include <algorithm>  // std::min / std::max (borne de chute, coupe de saut)

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
            //   Orientation courante (sert de direction de dash par défaut).
            if (input.moveX != 0.0f) {
                player.facing = (input.moveX > 0.0f) ? 1.0f : -1.0f;
            }
            //   0. Minuteries de game feel, décomptées au pas fixe (EX-CTRL-011, EX-NFR-002) :
            //      - coyote time : rechargé au sol, décompté en l'air (sauter juste après un bord)
            //      ;
            if (player.grounded) {
                player.coyoteTimer = _config.coyoteTime;
                player.airJumpsRemaining = _config.airJumps;  // recharge du double saut (EX-GP-015)
                player.dashAvailable = true;                  // recharge du dash (EX-GP-017)
            } else {
                player.coyoteTimer = std::max(0.0f, player.coyoteTimer - fixedDelta);
            }
            //      - jump buffering : rechargé à l'appui, décompté sinon (saut pré-appuyé honoré).
            if (input.jumpPressed) {
                player.jumpBufferTimer = _config.jumpBufferTime;
            } else {
                player.jumpBufferTimer = std::max(0.0f, player.jumpBufferTimer - fixedDelta);
            }
            // Verrou horizontal du wall jump : l'éjection persiste tant qu'il n'est pas écoulé.
            player.wallJumpLockTimer = std::max(0.0f, player.wallJumpLockTimer - fixedDelta);

            //   0d. Dash (EX-GP-017) : ruée directionnelle si disponible et pas déjà en dash.
            //       Direction = (moveX, moveY) normalisée (8 directions) ; à défaut l'orientation.
            if (input.dashPressed && player.dashAvailable && player.dashTimer <= 0.0f) {
                Vector2 direction{input.moveX, input.moveY};
                if (direction.x == 0.0f && direction.y == 0.0f) {
                    direction = Vector2{player.facing, 0.0f};
                }
                velocity.value = direction.normalized() * _config.dashSpeed;
                player.dashTimer = _config.dashDuration;
                player.dashAvailable = false;  // consommé, rechargé au prochain contact du sol
            }

            if (player.dashTimer > 0.0f) {
                // Pendant le dash : vitesse maintenue (posée au départ), gravité et entrée de
                // déplacement suspendues (trajectoire nette) ; le balayage l'arrête sur un mur.
                player.dashTimer = std::max(0.0f, player.dashTimer - fixedDelta);
            } else {
                //   0a. Saut, y négatif = montée (EX-GP-011). Sources d'autorisation, dans l'ordre
                //   :
                //       sol/coyote, puis MUR (wall jump, EX-GP-016), puis saut AÉRIEN (double saut,
                //       EX-GP-015). On consomme le buffer au déclenchement.
                if (player.jumpBufferTimer > 0.0f) {
                    if (player.coyoteTimer > 0.0f) {
                        velocity.value.y = -_config.jumpSpeed;
                        player.coyoteTimer = 0.0f;  // consomme (pas de re-saut dans la fenêtre)
                        player.jumpBufferTimer = 0.0f;
                    } else if (player.wallDirection != 0.0f) {
                        // Éjection en diagonale OPPOSÉE au mur ; verrouille le contrôle horizontal
                        // pour que la vitesse d'éjection porte le personnage loin du mur.
                        velocity.value.x = -player.wallDirection * _config.wallJumpSpeedX;
                        velocity.value.y = -_config.wallJumpSpeedY;
                        player.wallJumpLockTimer = _config.wallJumpLockTime;
                        player.jumpBufferTimer = 0.0f;
                    } else if (player.airJumpsRemaining > 0) {
                        velocity.value.y = -_config.jumpSpeed;
                        --player.airJumpsRemaining;  // consomme un saut aérien
                        player.jumpBufferTimer = 0.0f;
                    }
                }
                //   0b. Hauteur variable : bouton relâché pendant la montée → on plafonne la
                //   vitesse
                //       ascendante à `jumpCutFactor × jumpSpeed` (relâcher tôt = petit saut). Le
                //       max n'a aucun effet en chute (velocity.y > 0). Gravité inchangée
                //       (EX-GP-011).
                if (!input.jumpHeld) {
                    velocity.value.y =
                        std::max(velocity.value.y, -_config.jumpSpeed * _config.jumpCutFactor);
                }

                //   1. Vitesse horizontale voulue (pas d'inertie). Pendant le verrou de wall jump,
                //   on
                //      CONSERVE la vitesse d'éjection (contrôle horizontal suspendu).
                if (player.wallJumpLockTimer <= 0.0f) {
                    velocity.value.x = input.moveX * _config.moveSpeed;
                }

                //   2. Gravité (y vers le bas → tomber = y positif), puis borne de chute :
                velocity.value.y += _config.gravity * fixedDelta;
                velocity.value.y = std::min(velocity.value.y, _config.maxFallSpeed);

                //   2b. Wall slide : contre un mur, en l'air et en descente → chute ralentie
                //   (EX-GP-016).
                if (player.wallDirection != 0.0f && !player.grounded && velocity.value.y > 0.0f) {
                    velocity.value.y = std::min(velocity.value.y, _config.wallSlideSpeed);
                }
            }  // fin de la physique normale (hors dash)

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

            //   9. Contact MURAL : en l'air, blocage horizontal alors qu'on POUSSE vers le mur
            //      (EX-GP-016). wallDirection = sens du mur (opposé à la normale du contact).
            player.wallDirection = 0.0f;
            if (!player.grounded && result.normal.x != 0.0f) {
                const float wallDir = -result.normal.x;
                if (input.moveX * wallDir > 0.0f) {  // l'intention va bien vers le mur
                    player.wallDirection = wallDir;
                }
            }
        });
}

}  // namespace core
