#pragma once

#include <memory>

#include "Core/Ecs/AnimationClip.h"
#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/ISystem.h"

/**
 * @file Core/Ecs/Systems/AnimationSystem.h
 * @brief Système d'animation générique, piloté par données (`LOT-46`).
 */

namespace core {

/**
 * @brief Jeu de clips du personnage, migré tel quel depuis l'ancien dispositif figé (`LOT-18`).
 *
 * Trois clips nommés (« idle », « run », « jump »), mêmes durées (repos 0,5 s ; course 0,1 s) et
 * mêmes nombres d'images (2, 4, 1) qu'avant `LOT-46` : ce lot est une refonte d'infrastructure,
 * l'animation à l'écran ne doit strictement pas changer (`LOT-46` TACHE-04).
 *
 * Instance **partagée** (même `shared_ptr` renvoyé à chaque appel) : tous les personnages d'une
 * session utilisent le même jeu de clips, immuable.
 * @return Le jeu de clips du personnage.
 */
[[nodiscard]] std::shared_ptr<const ClipSet> playerClipSet();

/// Index du clip « idle » dans `core::playerClipSet()` — ordre **stable**, seule source de vérité
/// partagée avec `hmi::TextureAtlas::playerFrameRegion` (traduction en région d'atlas, `LOT-46`
/// TACHE-04) : ne pas réordonner sans mettre à jour `HMI` en conséquence.
constexpr int PLAYER_CLIP_IDLE = 0;
/// Index du clip « run » dans `core::playerClipSet()` (voir `PLAYER_CLIP_IDLE`).
constexpr int PLAYER_CLIP_RUN = 1;
/// Index du clip « jump » dans `core::playerClipSet()` (voir `PLAYER_CLIP_IDLE`).
constexpr int PLAYER_CLIP_JUMP = 2;

/**
 * @brief Fait progresser l'animation de toute entité portant `core::Animation`, et projette
 *        l'état physique du personnage sur son clip cible.
 *
 * Deux responsabilités distinctes (`LOT-46` TACHE-02) :
 * - **projection**, spécifique au personnage : pour chaque entité `Player` + `Velocity` +
 *   `Animation`, détermine le clip actif à partir de `Player::grounded` et de la vitesse
 *   horizontale (en l'air → saut ; au sol et en mouvement → course ; au sol et immobile → repos),
 *   comme avant `LOT-46` (`targetClip`, désormais résolu par nom plutôt que par valeur d'`enum`) ;
 * - **progression**, générale : pour **toute** entité portant `core::Animation` (personnage,
 *   tuiles animées, `LOT-46` TACHE-05), avance l'image courante selon la durée du clip résolu,
 *   boucle ou bascule sur le clip suivant en fin de clip joué une fois (`core::ClipEndMode::
 *   OneShot`).
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
     * @brief Applique un pas de simulation d'animation à toutes les entités animées.
     * @param world      Monde ECS.
     * @param fixedDelta Durée du pas de simulation, en secondes.
     */
    void update(World& world, float fixedDelta) override;
};

/**
 * @brief Fait progresser une seule animation d'un pas fixe (image courante, bouclage/bascule).
 *
 * Extrait de `AnimationSystem::update` pour être réutilisable **hors ECS** : `LOT-46` TACHE-05
 * anime des tuiles partageant un même asset (eau, lave) via une horloge unique par asset plutôt
 * qu'une entité par case, précisément pour éviter de répéter cette progression une fois par case.
 * Sans effet si `animation.clips` est nul ou vide (`EX-NFR-040`).
 * @param animation  État d'animation à faire progresser.
 * @param fixedDelta Durée du pas de simulation, en secondes.
 */
void advanceAnimation(Animation& animation, float fixedDelta);

}  // namespace core
