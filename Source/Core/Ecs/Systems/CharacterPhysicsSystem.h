#pragma once

#include <vector>

#include "Core/Physics/PhysicsConfig.h"
#include "Core/Physics/PlatformSample.h"

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
 *
 * **Plateformes mobiles** (`TileType::MovingPlatform`, `EX-GP-026`, optionnel via @p platforms) :
 * un personnage au sol sur une plateforme est **porté** avec elle (translation directe, avant que
 * sa propre physique ne s'applique), puis sa collision avec chaque plateforme est résolue en
 * continu (`sweepAabbVsAabb`, même principe que les blocs réduits, `EX-GP-005`) après le balayage
 * sur grille — la résolution la plus stricte des deux l'emporte, par construction. Un personnage
 * écrasé entre une plateforme montante et un plafond meurt (`Player::squished`), plutôt que de
 * bloquer la plateforme (qui resterait fonction pure du numéro de pas, `EX-NFR-002`).
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
     * @param platforms  Échantillons de position des plateformes mobiles pour ce pas
     *                   (`core::PlatformController::samples`) ; vide par défaut (comportement
     *                   inchangé pour tout niveau sans plateforme, `EX-GP-026`).
     */
    void update(World& world, const TileMap& tiles, const PlayerInput& input, float fixedDelta,
                const std::vector<PlatformSample>& platforms = {}) const;

private:
    /// Porte le personnage avec la plateforme sur laquelle il repose (s'il en repose une), avant
    /// que sa propre physique ne s'applique -- translation directe de `transform.position`. Marque
    /// `player.squished` si la translation l'embarque dans une tuile solide (écrasement plafond,
    /// `EX-GP-026`).
    void applyPlatformPortage(Player& player, Transform& transform, const Collider& collider,
                              const TileMap& tiles,
                              const std::vector<PlatformSample>& platforms) const;
    /**
     * @brief Résout la collision continue du personnage contre chaque plateforme
     *        (`sweepAabbVsAabb`), après le balayage sur grille -- même patron que
     *        `hmi::GameSession::resolveReducedBlockCollision` pour les blocs réduits
     *        (`EX-GP-005`), embarqué ici pour rester testable directement
     *        (`Source/Test/Integration/test_physique_personnage.cpp`).
     * @param player        Personnage dont l'état `grounded` est mis à jour en cas de contact.
     * @param transform     Position mutée en place si une plateforme bloque le déplacement.
     * @param velocity      Vitesse annulée sur les axes bloqués par une plateforme.
     * @param stepStartBox  Boîte du personnage au DÉBUT de ce pas (après portage, avant le
     *                      balayage sur grille) : sert de référence pour le déplacement réellement
     *                      obtenu par la grille, composé ici avec chaque plateforme.
     * @param platforms     Échantillons de position des plateformes mobiles pour ce pas.
     */
    void resolvePlatformCollision(Player& player, Transform& transform, Velocity& velocity,
                                  const Aabb& stepStartBox,
                                  const std::vector<PlatformSample>& platforms) const;

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
