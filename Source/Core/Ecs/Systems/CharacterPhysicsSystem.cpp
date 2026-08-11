#include "Core/Ecs/Systems/CharacterPhysicsSystem.h"

#include <algorithm>  // std::min / std::max (borne de chute, coupe de saut)
#include <cmath>      // std::abs (détection de l'apex)
#include <string>

#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/TileMap.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PhysicsLog.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/SlopeGeometry.h"
#include "Core/Physics/SweptCollision.h"

namespace core {

CharacterPhysicsSystem::CharacterPhysicsSystem(PhysicsConfig config) : _config(config) {}

// Applique un pas de simulation aux personnages (voir en-tête).
void CharacterPhysicsSystem::update(World& world, const TileMap& tiles, const PlayerInput& input,
                                    float fixedDelta) const {
    world.view<Player, Transform, Velocity, Collider>().each(
        [&](Entity, Player& player, Transform& transform, Velocity& velocity, Collider& collider) {
            resolveVelocity(player, velocity, input, fixedDelta);
            resolveCollisionAndState(player, transform, velocity, collider, tiles, input,
                                     fixedDelta);
        });
}

bool CharacterPhysicsSystem::applyDash(Player& player, Velocity& velocity, const PlayerInput& input,
                                       float fixedDelta) const {
    //   0d. Dash (EX-GP-017) : ruée directionnelle si disponible et pas déjà en dash.
    //       Direction = (moveX, moveY) normalisée (8 directions) ; à défaut l'orientation.
    //       Refusé si le budget de dashs du tableau est épuisé (EX-GP-024 ; -1 = illimité).
    if (input.dashPressed && player.dashAvailable && player.dashTimer <= 0.0F &&
        player.dashesRemaining != 0) {
        Vector2 direction{input.moveX, input.moveY};
        if (direction.x == 0.0F && direction.y == 0.0F) {
            direction = Vector2{player.facing, 0.0F};
        }
        velocity.value = direction.normalized() * _config.dashSpeed;
        player.dashTimer = _config.dashDuration;
        player.dashAvailable = false;  // consommé, rechargé au prochain contact du sol
        if (player.dashesRemaining > 0) {
            --player.dashesRemaining;  // décompte le budget (si limité)
        }
    }

    if (player.dashTimer <= 0.0F) {
        return false;
    }
    // Pendant le dash : vitesse maintenue (posée au départ), gravité et entrée de déplacement
    // suspendues (trajectoire nette) ; le balayage l'arrête sur un mur.
    player.dashTimer = std::max(0.0F, player.dashTimer - fixedDelta);
    return true;
}

void CharacterPhysicsSystem::applyJump(Player& player, Velocity& velocity) const {
    //   0a. Saut, y négatif = montée (EX-GP-011). Sources d'autorisation, dans l'ordre :
    //       sol/coyote, puis MUR (wall jump, EX-GP-016), puis saut AÉRIEN (double saut,
    //       EX-GP-015). On consomme le buffer au déclenchement.
    //       Refusé si le budget de sauts du tableau est épuisé (EX-GP-024 ; -1 illimité).
    if (player.jumpBufferTimer <= 0.0F || player.jumpsRemaining == 0) {
        return;
    }
    bool jumped = true;
    if (player.coyoteTimer > 0.0F) {
        velocity.value.y = -_config.jumpSpeed;
        player.coyoteTimer = 0.0F;  // consomme (pas de re-saut dans la fenêtre)
    } else if (player.wallDirection != 0.0F) {
        // Éjection en diagonale OPPOSÉE au mur ; verrouille le contrôle horizontal pour que la
        // vitesse d'éjection porte le personnage loin du mur.
        velocity.value.x = -player.wallDirection * _config.wallJumpSpeedX;
        velocity.value.y = -_config.wallJumpSpeedY;
        player.wallJumpLockTimer = _config.wallJumpLockTime;
    } else if (player.airJumpsRemaining > 0) {
        velocity.value.y = -_config.jumpSpeed;
        --player.airJumpsRemaining;  // consomme un saut aérien
    } else {
        jumped = false;  // aucune source de saut disponible
    }
    if (jumped) {
        player.jumpBufferTimer = 0.0F;  // consomme le buffer
        if (player.jumpsRemaining > 0) {
            --player.jumpsRemaining;  // décompte le budget (si limité)
        }
    }
}

void CharacterPhysicsSystem::resolveVelocity(Player& player, Velocity& velocity,
                                             const PlayerInput& input, float fixedDelta) const {
    //   Orientation courante (sert de direction de dash par défaut).
    if (input.moveX != 0.0F) {
        player.facing = (input.moveX > 0.0F) ? 1.0F : -1.0F;
    }
    //   0. Minuteries de game feel, décomptées au pas fixe (EX-CTRL-011, EX-NFR-002) :
    //      - coyote time : rechargé au sol, décompté en l'air (sauter juste après un bord) ;
    if (player.grounded) {
        player.coyoteTimer = _config.coyoteTime;
        player.airJumpsRemaining = _config.airJumps;  // recharge du double saut (EX-GP-015)
        player.dashAvailable = true;                  // recharge du dash (EX-GP-017)
    } else {
        player.coyoteTimer = std::max(0.0F, player.coyoteTimer - fixedDelta);
    }
    //      - jump buffering : rechargé à l'appui, décompté sinon (saut pré-appuyé honoré).
    if (input.jumpPressed) {
        player.jumpBufferTimer = _config.jumpBufferTime;
    } else {
        player.jumpBufferTimer = std::max(0.0F, player.jumpBufferTimer - fixedDelta);
    }
    // Verrou horizontal du wall jump : l'éjection persiste tant qu'il n'est pas écoulé.
    player.wallJumpLockTimer = std::max(0.0F, player.wallJumpLockTimer - fixedDelta);

    if (applyDash(player, velocity, input, fixedDelta)) {
        return;
    }

    applyJump(player, velocity);
    //   0b. Hauteur variable : bouton relâché pendant la montée → on plafonne la vitesse
    //       ascendante à `jumpCutFactor × jumpSpeed` (relâcher tôt = petit saut). Le max n'a
    //       aucun effet en chute (velocity.y > 0). Gravité inchangée (EX-GP-011).
    if (!input.jumpHeld) {
        velocity.value.y = std::max(velocity.value.y, -_config.jumpSpeed * _config.jumpCutFactor);
    }

    //   1. Vitesse horizontale voulue (pas d'inertie). Pendant le verrou de wall jump, on
    //      CONSERVE la vitesse d'éjection (contrôle horizontal suspendu).
    if (player.wallJumpLockTimer <= 0.0F) {
        velocity.value.x = input.moveX * _config.moveSpeed;
    }

    //   2. Gravité EFFECTIVE (EX-GP-018), y vers le bas → tomber = y positif :
    //      base pour la montée ; × chute (plus rapide) ; × fast-fall si « bas » ;
    //      × apex (flottement) quand la vitesse verticale est faible.
    float effectiveGravity = _config.gravity;
    if (velocity.value.y > 0.0F) {  // en chute
        effectiveGravity *= _config.fallGravityMultiplier;
        if (input.moveY > 0.0F) {  // « bas » maintenu → chute accélérée
            effectiveGravity *= _config.fastFallMultiplier;
        }
    }
    if (std::abs(velocity.value.y) < _config.apexThreshold) {  // proche de l'apex
        effectiveGravity *= _config.apexGravityMultiplier;
    }
    //   2bis. Chute NEWTONIENNE (EX-GP-019) : force nette = poids (masse × gravité effective)
    //   MOINS une trainee proportionnelle a la vitesse ; la vitesse terminale EMERGE de cet
    //   equilibre (poids = trainee) au lieu d'un plafond arbitraire — une masse plus grande tombe
    //   plus vite (trainee relativement plus faible). La MONTEE du saut reste a gravite simple,
    //   inchangee (EX-GP-011) : le ressenti de saut deja regle en LOT-11 n'est pas affecte par ce
    //   lot.
    if (velocity.value.y >= 0.0F) {
        const float netAcceleration =
            effectiveGravity - ((_config.fallDragCoefficient * velocity.value.y) / player.mass);
        velocity.value.y += netAcceleration * fixedDelta;
    } else {
        velocity.value.y += effectiveGravity * fixedDelta;
    }

    //   2b. Wall slide : contre un mur, en l'air et en descente → chute ralentie (EX-GP-016).
    if (player.wallDirection != 0.0F && !player.grounded && velocity.value.y > 0.0F) {
        velocity.value.y = std::min(velocity.value.y, _config.wallSlideSpeed);
    }
}

void CharacterPhysicsSystem::resolveCollisionAndState(Player& player, Transform& transform,
                                                      Velocity& velocity, const Collider& collider,
                                                      const TileMap& tiles,
                                                      const PlayerInput& input,
                                                      float fixedDelta) const {
    //   3. Déplacement voulu sur ce pas :
    const Vector2 delta = velocity.value * fixedDelta;

    //   4. Boîte du personnage (convention coin haut-gauche + taille, cf. Collider) :
    const Aabb box = Aabb::fromTopLeftSize(transform.position, collider.size);

    //   5. Résolution CONTINUE contre les tuiles solides :
    const SweepResult result = sweepAabb(box, delta, tiles);

    //   6. Applique la position résolue (mur/sol/plafond, grille classique — une pente
    //      n'y est jamais solide, voir isSolid) :
    transform.position = result.position;

    //   6bis. Suivi de pente (EX-GP-003) : cale la position verticale sur la surface d'une pente
    //   si le bord bas du personnage l'a franchie ce pas (marche dessus ou tombe dessus) — jamais
    //   si `velocity.value.y < 0` (vient de sauter, voir resolveSlopeFollow). Ajout APRÈS la
    //   résolution grille classique, sans la modifier : isole le risque de régression sur la
    //   physique déjà testée (murs/sols plats).
    bool onSlope = false;
    bool ceilingSlopeBlocked = false;
    {
        const Aabb newBox = Aabb::fromTopLeftSize(transform.position, collider.size);
        const SlopeFollowResult follow = resolveSlopeFollow(box, newBox, velocity.value.y, tiles);
        if (follow.grounded) {
            transform.position.y = follow.bottomY - collider.size.y;
            velocity.value.y = 0.0F;
            onSlope = true;
            // Événement rare et notable (pas par-frame) : un calage de sol via une tuile de
            // PLAFOND ne devrait arriver que pour un atterrissage sur sa face du haut en tombant
            // depuis au-dessus (voir le garde-fou de `resolveSlopeFollow` contre un chevauchement
            // résiduel après un blocage par en dessous, EX-GP-007).
            const int centerColumn =
                static_cast<int>(std::floor((newBox.min.x + newBox.max.x) * 0.5F));
            const int landedRow = static_cast<int>(std::floor(follow.bottomY - 1e-4F));
            if (centerColumn >= 0 && centerColumn < tiles.width() && landedRow >= 0 &&
                landedRow < tiles.height() && isCeilingSlope(tiles.tile(centerColumn, landedRow))) {
                PHYSICS_LOG_TRACE(
                    "Calage au sol sur une tuile de plafond (face du haut) : colonne=" +
                    std::to_string(centerColumn) + " ligne=" + std::to_string(landedRow) +
                    " bottomY=" + std::to_string(follow.bottomY));
            }
        }
    }

    //   6ter. Pente/arrondi de PLAFOND (EX-GP-006) : miroir de 6bis, bloque le bord haut si une
    //   silhouette de plafond a été franchie en sautant — jamais de suivi de déplacement latéral
    //   (contrairement au sol), seulement un blocage (bonk), comme un plafond classique. Vérifié
    //   APRÈS le suivi de sol (indépendant : l'un agit sur le bord bas en tombant, l'autre sur le
    //   bord haut en montant, jamais simultanément).
    //
    //   Suivi d'ascension (EX-GP-007) : `resolveCeilingSlopeFollow` a besoin de l'étendue
    //   horizontale couverte par la boîte depuis le DÉBUT de la montée courante, pas seulement ce
    //   pas — marcher tout en sautant peut faire franchir le seuil vertical de blocage sur un pas
    //   où la colonne pertinente n'est plus couverte par la boîte, alors qu'elle l'était sur un
    //   pas antérieur de la MÊME montée (le seuil peut être manqué de peu sur plusieurs pas
    //   successifs avant d'être enfin atteint). Remis à l'étendue courante dès que le personnage
    //   était au sol ou ne montait pas avant ce pas — sans cette remise, une vieille montée
    //   laisserait une trace non pertinente.
    if (player.grounded || velocity.value.y >= 0.0F) {
        player.ascentSweepMinX = box.min.x;
        player.ascentSweepMaxX = box.max.x;
    }
    if (!onSlope) {
        const Aabb newBox = Aabb::fromTopLeftSize(transform.position, collider.size);
        const CeilingSlopeFollowResult ceilingFollow =
            resolveCeilingSlopeFollow(box.min.y, player.ascentSweepMinX, player.ascentSweepMaxX,
                                      newBox, velocity.value.y, tiles);
        if (ceilingFollow.blocked) {
            transform.position.y = ceilingFollow.topY;
            velocity.value.y = 0.0F;
            ceilingSlopeBlocked = true;
        }
        player.ascentSweepMinX = (std::min)(player.ascentSweepMinX, newBox.min.x);
        player.ascentSweepMaxX = (std::max)(player.ascentSweepMaxX, newBox.max.x);
    }

    //   7. Annule la vitesse sur les axes bloqués (choc mur / sol / plafond) :
    if (result.normal.x != 0.0F) {
        velocity.value.x = 0.0F;
    }
    if (!onSlope && !ceilingSlopeBlocked && result.normal.y != 0.0F) {
        velocity.value.y = 0.0F;
    }

    //   8. État « au sol » = contact SOUS le personnage (normale vers le haut, y < 0), ou calé
    //      sur une pente :
    player.grounded = onSlope || (result.normal.y < 0.0F);

    //   9. Contact MURAL : en l'air, blocage horizontal alors qu'on POUSSE vers le mur
    //      (EX-GP-016). wallDirection = sens du mur (opposé à la normale du contact).
    player.wallDirection = 0.0F;
    if (!player.grounded && result.normal.x != 0.0F) {
        const float wallDir = -result.normal.x;
        if (input.moveX * wallDir > 0.0F) {  // l'intention va bien vers le mur
            player.wallDirection = wallDir;
        }
    }
}

}  // namespace core
