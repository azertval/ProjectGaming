// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include "Core/Physics/AabbVsAabb.h"
#include "Core/Physics/PhysicsLog.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/SlopeGeometry.h"
#include "Core/Physics/SweptCollision.h"

namespace core {

namespace {

// Tolerance de contact "repose sur le dessus" d'une plateforme -- meme ordre de grandeur que
// core::BlockController::PUSH_TOUCH_TOLERANCE (bords qui se touchent, pas qui se chevauchent).
constexpr float PLATFORM_REST_TOLERANCE = 0.05F;

// Chevauchement horizontal entre deux boites (aire strictement positive sur l'axe X).
[[nodiscard]] bool overlapsHorizontally(const Aabb& a, const Aabb& b) {
    return a.min.x < b.max.x && a.max.x > b.min.x;
}

// box repose-t-elle sur le DESSUS de platformBox (chevauchement horizontal, bord bas de box au
// contact du bord haut de platformBox) ? Duplique core::restsOnTopOfPlatform (Gameplay,
// PlatformController.h) plutot que d'y inclure une dependance : Ecs/Systems ne depend jamais de
// Gameplay (sens inverse de la dependance etablie, PlatformController -> Ecs/Physics).
[[nodiscard]] bool restsOnTopOf(const Aabb& box, const Aabb& platformBox) {
    if (!overlapsHorizontally(box, platformBox)) {
        return false;
    }
    return std::abs(box.max.y - platformBox.min.y) <= PLATFORM_REST_TOLERANCE;
}

}  // namespace

CharacterPhysicsSystem::CharacterPhysicsSystem(PhysicsConfig config) : _config(config) {}

// Applique un pas de simulation aux personnages (voir en-tête).
void CharacterPhysicsSystem::update(World& world, const TileMap& tiles, const PlayerInput& input,
                                    float fixedDelta,
                                    const std::vector<PlatformSample>& platforms) const {
    world.view<Player, Transform, Velocity, Collider>().each(
        [&](Entity, Player& player, Transform& transform, Velocity& velocity, Collider& collider) {
            player.squished = false;
            applyPlatformPortage(player, transform, collider, tiles, platforms);
            const Aabb stepStartBox = Aabb::fromTopLeftSize(transform.position, collider.size);
            resolveVelocity(player, velocity, input, fixedDelta);
            resolveCollisionAndState(player, transform, velocity, collider, tiles, input,
                                     fixedDelta);
            resolvePlatformCollision(player, transform, velocity, stepStartBox, platforms);
        });
}

// Porte le personnage avec la plateforme sur laquelle il repose, avant que sa propre physique ne
// s'applique (voir en-tête). Le "reposait sur la plateforme" se redérive purement de la géométrie
// (box courante contre previousBox de l'échantillon) : aucun état à mémoriser d'un pas à l'autre,
// player.grounded (calculé au pas précédent, par la grille OU une plateforme) suffit à savoir
// qu'un contact existait quelque part.
void CharacterPhysicsSystem::applyPlatformPortage(
    Player& player, Transform& transform, const Collider& collider, const TileMap& tiles,
    const std::vector<PlatformSample>& platforms) const {
    if (!player.grounded || platforms.empty()) {
        return;
    }
    const Aabb box = Aabb::fromTopLeftSize(transform.position, collider.size);
    for (const PlatformSample& sample : platforms) {
        if (!restsOnTopOf(box, sample.previousBox)) {
            continue;
        }
        transform.position += sample.currentBox.min - sample.previousBox.min;
        break;  // une seule plateforme peut porter le personnage a la fois
    }

    // Ecrasement (EX-GP-026) : la translation ci-dessus vient-elle d'embarquer le personnage dans
    // une tuile solide (plateforme montante contre un plafond) ? Decision de cadrage : mortel,
    // signale via Player::squished plutot que de bloquer la plateforme (qui resterait fonction
    // pure du numero de pas, EX-NFR-002).
    const Aabb carriedBox = Aabb::fromTopLeftSize(transform.position, collider.size);
    const int firstColumn = static_cast<int>(std::floor(carriedBox.min.x));
    const int lastColumn = static_cast<int>(std::ceil(carriedBox.max.x)) - 1;
    const int firstRow = static_cast<int>(std::floor(carriedBox.min.y));
    const int lastRow = static_cast<int>(std::ceil(carriedBox.max.y)) - 1;
    for (int row = firstRow; row <= lastRow && !player.squished; ++row) {
        for (int column = firstColumn; column <= lastColumn; ++column) {
            if (!tiles.inBounds(column, row) || !tiles.isSolid(column, row)) {
                continue;
            }
            const bool overlaps = carriedBox.min.x < static_cast<float>(column) + 1.0F &&
                                  carriedBox.max.x > static_cast<float>(column) &&
                                  carriedBox.min.y < static_cast<float>(row) + 1.0F &&
                                  carriedBox.max.y > static_cast<float>(row);
            if (overlaps) {
                player.squished = true;
                break;
            }
        }
    }
}

// Resout la collision continue contre chaque plateforme, apres le balayage sur grille (voir
// en-tete). Meme composition que hmi::GameSession::resolveReducedBlockCollision : la resolution la
// PLUS STRICTE (la plus proche du depart) l'emporte par axe, entre toutes les plateformes.
void CharacterPhysicsSystem::resolvePlatformCollision(
    Player& player, Transform& transform, Velocity& velocity, const Aabb& stepStartBox,
    const std::vector<PlatformSample>& platforms) const {
    if (platforms.empty()) {
        return;
    }
    const Vector2 delta = transform.position - stepStartBox.min;
    if (delta.x == 0.0F && delta.y == 0.0F) {
        return;
    }
    Vector2 bestPosition = transform.position;
    Vector2 bestNormal{};
    for (const PlatformSample& sample : platforms) {
        const SweepResult result = sweepAabbVsAabb(stepStartBox, delta, sample.currentBox);
        if (result.normal.x != 0.0F && std::abs(result.position.x - stepStartBox.min.x) <
                                           std::abs(bestPosition.x - stepStartBox.min.x)) {
            bestPosition.x = result.position.x;
            bestNormal.x = result.normal.x;
        }
        if (result.normal.y != 0.0F && std::abs(result.position.y - stepStartBox.min.y) <
                                           std::abs(bestPosition.y - stepStartBox.min.y)) {
            bestPosition.y = result.position.y;
            bestNormal.y = result.normal.y;
        }
    }
    if (bestNormal.x == 0.0F && bestNormal.y == 0.0F) {
        return;
    }
    transform.position = bestPosition;
    if (bestNormal.x != 0.0F) {
        velocity.value.x = 0.0F;
    }
    if (bestNormal.y != 0.0F) {
        velocity.value.y = 0.0F;
        if (bestNormal.y < 0.0F) {
            player.grounded = true;  // pose sur le dessus d'une plateforme
        }
    }
}

bool CharacterPhysicsSystem::applyDash(Player& player, Velocity& velocity, const PlayerInput& input,
                                       float fixedDelta) const {
    //   0c. Jump-cancel du dash (combo, EX-GP-061) : un saut demandé PENDANT un dash BOOSTÉ actif
    //       l'interrompt immédiatement et EN CONSERVE la vitesse horizontale (hyper-dash), au lieu
    //       d'attendre son expiration normale. Rend la main à resolveVelocity() (return false) pour
    //       qu'applyJump() s'exécute ce même pas -- au contact d'un mur, il choisira de lui-même un
    //       WALL JUMP (même ordre de priorité qu'un saut hors dash, EX-GP-016), sans code dédié.
    //       Restreint au dash BOOSTÉ (`dashIsBoosted`, EX-GP-056) : un dash NORMAL pendant lequel un
    //       saut est pressé continue de se comporter EXACTEMENT comme avant ce lot (le saut reste
    //       simplement bufferisé, honoré à l'expiration naturelle du dash) -- aucun contenu
    //       existant ne peut produire de dash boosté, donc aucune régression possible ici.
    if (player.dashTimer > 0.0F && player.dashIsBoosted && player.jumpBufferTimer > 0.0F &&
        player.jumpsRemaining != 0) {
        player.comboChainCount = (player.comboWindowTimer > 0.0F) ? player.comboChainCount + 1 : 1;
        player.comboWindowTimer = _config.comboWindowTime;
        const float bonus =
            (std::min)(_config.comboSpeedBonus * static_cast<float>(player.comboChainCount),
                       _config.comboSpeedCap);
        const float sign = velocity.value.x >= 0.0F ? 1.0F : -1.0F;
        player.dashJumpMomentumX = velocity.value.x + sign * bonus;
        player.dashJumpLockTimer = _config.dashJumpLockTime;
        player.dashTimer = 0.0F;
        return false;
    }

    //   0d-pound. Ground pound (EX-GP-058) : le bouton de dash visé PUREMENT vers le bas, en l'air,
    //       ARME un ground pound -- mais SEULEMENT si le personnage n'a plus aucune charge de dash
    //       (`dashChargesRemaining <= 0`) : tant qu'une charge existe, cette combinaison reste un
    //       dash vertical normal comme avant ce lot (aucune régression sur un dash intentionnel
    //       vers le bas, ex. descente rapide d'un puits dans un niveau existant). Ground pound
    //       n'est donc atteignable que là où, avant ce lot, appuyer sur dash sans charge ne faisait
    //       strictement rien -- une pure addition, jamais une réinterprétation d'un geste déjà
    //       significatif.
    if (!player.grounded && input.dashPressed && input.moveX == 0.0F && input.moveY > 0.0F &&
        player.dashTimer <= 0.0F && player.dashChargesRemaining <= 0) {
        player.groundPounding = true;
        return false;
    }

    //   0d. Dash (EX-GP-017) : ruée directionnelle si disponible et pas déjà en dash.
    //       Direction = (moveX, moveY) normalisée (8 directions) ; à défaut l'orientation (celle
    //       figée par la charge si un boost est banké, EX-GP-056, sinon l'orientation courante).
    //       Refusé si le budget de dashs du tableau est épuisé (EX-GP-024 ; -1 = illimité).
    if (input.dashPressed && player.dashChargesRemaining > 0 && player.dashTimer <= 0.0F &&
        player.dashesRemaining != 0) {
        Vector2 direction{input.moveX, input.moveY};
        if (direction.x == 0.0F && direction.y == 0.0F) {
            direction =
                Vector2{player.dashBoostReady ? player.dashBoostFacing : player.facing, 0.0F};
        }
        const bool boosted = player.dashBoostReady;
        const float speed =
            boosted ? _config.dashSpeed * _config.dashBoostSpeedMultiplier : _config.dashSpeed;
        velocity.value = direction.normalized() * speed;
        player.dashTimer = boosted ? _config.dashDuration * _config.dashBoostDurationMultiplier
                                   : _config.dashDuration;
        player.dashIsBoosted = boosted;
        --player.dashChargesRemaining;  // consommée, rechargée au prochain contact du sol
        if (player.dashesRemaining > 0) {
            --player.dashesRemaining;  // décompte le budget (si limité)
        }
        player.dashBoostReady = false;  // la charge est consommée, qu'elle serve ou non au boost
        player.dashChargeTimer = 0.0F;
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
        player.justJumped = true;
    }
}

void CharacterPhysicsSystem::resolveVelocity(Player& player, Velocity& velocity,
                                             const PlayerInput& input, float fixedDelta) const {
    // Front de saut : remis a faux au debut de CE pas, positionne par applyJump() plus bas s'il y
    // a effectivement saut. Ainsi un lecteur externe (hmi::GameEvents) qui echantillonne player
    // juste apres update() voit vrai UNIQUEMENT sur le pas ou le saut se declenche.
    player.justJumped = false;

    //   Orientation D'AVANT ce pas, capturée pour la charge de dash ci-dessous (avant sa propre
    //   mise à jour juste en dessous) -- voir son commentaire.
    const float facingBeforeInput = player.facing;

    //   Orientation courante (sert de direction de dash par défaut) : mise à jour EXACTEMENT comme
    //   avant ce lot, sans aucune dépendance à la charge de dash ci-dessous (aucune régression sur
    //   le sprite/la caméra/la direction d'un dash hors charge active).
    if (input.moveX != 0.0F) {
        player.facing = (input.moveX > 0.0F) ? 1.0F : -1.0F;
    }

    //   Charge de dash (EX-GP-056) : maintenir la direction OPPOSÉE à l'orientation D'AVANT ce pas
    //   ET le bouton de dash (`input.dashHeld`) charge un boost. Comparée à
    //   `dashChargeReferenceFacing`, une COPIE figée au début de la charge -- jamais à `facing`
    //   lui-même (mis à jour ci-dessus, immédiatement) : sinon `facing` « rattraperait » l'entrée
    //   opposée dès ce même pas et rendrait l'opposition indétectable sur plus d'un pas.
    //   `dashHeld` est une garde DÉLIBÉRÉE : sans elle, un simple changement de direction pendant un
    //   déplacement normal (aucune intention de dash) suffirait à amorcer la charge -- un premier
    //   essai sans cette garde a cassé la séquence `demo-final` (une inversion de direction anodine
    //   y suffisait à armer un boost qui changeait ensuite la vitesse/durée du dash suivant, sans
    //   rapport avec une intention de charge). Maintenir le bouton de dash pendant ~0,25 s tout en
    //   marchant à l'opposé n'arrive jamais par accident.
    if (player.dashChargeTimer <= 0.0F && !player.dashBoostReady) {
        if (input.dashHeld && input.moveX != 0.0F &&
            ((input.moveX > 0.0F) != (facingBeforeInput > 0.0F))) {
            player.dashChargeReferenceFacing = facingBeforeInput;
            player.dashChargeTimer = fixedDelta;
        }
    } else if (!player.dashBoostReady) {
        const bool stillOpposing =
            input.dashHeld && input.moveX != 0.0F &&
            ((input.moveX > 0.0F) != (player.dashChargeReferenceFacing > 0.0F));
        if (stillOpposing) {
            player.dashChargeTimer += fixedDelta;
            if (player.dashChargeTimer >= _config.dashChargeHoldTime) {
                player.dashBoostReady = true;
                player.dashBoostFacing = player.dashChargeReferenceFacing;
            }
        } else {
            player.dashChargeTimer = 0.0F;
        }
    }
    //   0. Minuteries de game feel, décomptées au pas fixe (EX-CTRL-011, EX-NFR-002) :
    //      - coyote time : rechargé au sol, décompté en l'air (sauter juste après un bord) ;
    if (player.grounded) {
        player.coyoteTimer = _config.coyoteTime;
        player.airJumpsRemaining = _config.airJumps;        // double saut (EX-GP-015)
        player.dashChargesRemaining = _config.dashCharges;  // charges de dash (EX-GP-017)
        player.groundPounding = false;                      // fin du ground pound (EX-GP-058)
        player.comboChainCount = 0;                          // combo remis à zéro au sol (EX-GP-061)
        player.comboWindowTimer = 0.0F;
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
    // Verrou horizontal du jump-cancel de dash (EX-GP-061) : la vitesse conservée persiste tant
    // qu'il n'est pas écoulé (même patron que le verrou de wall jump ci-dessus).
    player.dashJumpLockTimer = std::max(0.0F, player.dashJumpLockTimer - fixedDelta);
    // Fenêtre de combo (jump-cancels rapprochés) et d'héritage de momentum après une poussée
    // renforcée (EX-GP-061) : décomptées au pas fixe, comme les minuteries ci-dessus.
    player.comboWindowTimer = std::max(0.0F, player.comboWindowTimer - fixedDelta);
    player.pushMomentumWindowTimer = std::max(0.0F, player.pushMomentumWindowTimer - fixedDelta);

    if (applyDash(player, velocity, input, fixedDelta)) {
        return;
    }

    //   0e. Ground pound (EX-GP-058) : armé par applyDash() ci-dessus (bouton de dash visé
    //       purement vers le bas, en l'air) ; impose ici la vitesse de chute jusqu'à l'atterrissage
    //       (gravité suspendue, comme le dash -- remis à faux au contact du sol, voir plus haut).
    if (player.groundPounding) {
        velocity.value.x = 0.0F;
        velocity.value.y = _config.groundPoundSpeed;
        return;
    }

    applyJump(player, velocity);
    //   0b. Hauteur variable : bouton relâché pendant la montée → on plafonne la vitesse
    //       ascendante à `jumpCutFactor × jumpSpeed` (relâcher tôt = petit saut). Le max n'a
    //       aucun effet en chute (velocity.y > 0). Gravité inchangée (EX-GP-011).
    if (!input.jumpHeld) {
        velocity.value.y = std::max(velocity.value.y, -_config.jumpSpeed * _config.jumpCutFactor);
    }

    //   1. Vitesse horizontale voulue (pas d'inertie). Pendant le verrou de wall jump, on CONSERVE
    //      sa vitesse d'éjection (contrôle horizontal suspendu) -- PRIORITAIRE sur le verrou de
    //      jump-cancel (EX-GP-061) : un jump-cancel qui aboutit à un wall jump (contact mural,
    //      applyJump() ci-dessus) doit garder l'éjection du mur, pas la vitesse brute du dash
    //      annulé. Sinon (jump-cancel sans wall jump), on CONSERVE la vitesse du dash interrompu.
    if (player.wallJumpLockTimer > 0.0F) {
        // rien à faire : velocity.value.x porte déjà l'éjection posée par applyJump().
    } else if (player.dashJumpLockTimer > 0.0F) {
        velocity.value.x = player.dashJumpMomentumX;
    } else {
        velocity.value.x = input.moveX * _config.moveSpeed;
    }
    //   1bis. Momentum hérité d'une poussée renforcée récente (EX-GP-057/EX-GP-061) : APRÈS que la
    //   vitesse horizontale de base soit fixée ci-dessus (quelle que soit sa source), ajoute une
    //   fraction de la vitesse du bloc poussé si un saut vient de se déclencher ce pas et que la
    //   fenêtre est encore ouverte. Consommé (fenêtre remise à zéro) : un seul héritage par poussée.
    if (player.justJumped && player.pushMomentumWindowTimer > 0.0F) {
        velocity.value.x += player.pushMomentumVelocityX * _config.momentumCarryRatio;
        player.pushMomentumWindowTimer = 0.0F;
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
