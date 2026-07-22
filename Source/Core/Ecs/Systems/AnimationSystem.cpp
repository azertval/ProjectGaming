#include "Core/Ecs/Systems/AnimationSystem.h"

#include <cmath>  // std::abs

#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/World.h"

namespace core {

namespace {
// En dessous de ce seuil (unités monde/s), le personnage est considéré immobile (bruit flottant
// toléré ; Idle ne doit jamais se déclencher à tort pour un personnage réellement en mouvement).
constexpr float MOVING_THRESHOLD = 0.01f;

// Durée (secondes) d'une image, par clip. Jump ne progresse jamais (une seule image).
constexpr float IDLE_FRAME_DURATION = 0.5f;
constexpr float RUN_FRAME_DURATION = 0.1f;

int frameCountFor(AnimationClip clip) {
    switch (clip) {
        case AnimationClip::Idle:
            return IDLE_FRAME_COUNT;
        case AnimationClip::Run:
            return RUN_FRAME_COUNT;
        case AnimationClip::Jump:
            return JUMP_FRAME_COUNT;
    }
    return 1;
}

float frameDurationFor(AnimationClip clip) {
    switch (clip) {
        case AnimationClip::Idle:
            return IDLE_FRAME_DURATION;
        case AnimationClip::Run:
            return RUN_FRAME_DURATION;
        case AnimationClip::Jump:
            return RUN_FRAME_DURATION;  // sans effet : un seul cadre, jamais atteint par la boucle
    }
    return IDLE_FRAME_DURATION;
}

// Détermine le clip cible à partir de l'état physique courant (projection, cf. en-tête).
AnimationClip targetClip(const Player& player, const Velocity& velocity) {
    if (!player.grounded) {
        return AnimationClip::Jump;
    }
    if (std::abs(velocity.value.x) > MOVING_THRESHOLD) {
        return AnimationClip::Run;
    }
    return AnimationClip::Idle;
}
}  // namespace

// Applique un pas de simulation d'animation à tous les personnages (voir en-tête).
void AnimationSystem::update(World& world, float fixedDelta) {
    world.view<Player, Velocity, Animation>().each(
        [fixedDelta](Entity, Player& player, Velocity& velocity, Animation& animation) {
            const AnimationClip clip = targetClip(player, velocity);
            if (clip != animation.clip) {
                // Changement de clip : redémarre net sur la première image (EX-REN-012).
                animation.clip = clip;
                animation.frameIndex = 0;
                animation.elapsed = 0.0f;
                return;
            }

            const int frameCount = frameCountFor(clip);
            if (frameCount <= 1) {
                return;  // Jump : pose unique, jamais animée.
            }

            const float frameDuration = frameDurationFor(clip);
            animation.elapsed += fixedDelta;
            while (animation.elapsed >= frameDuration) {
                animation.elapsed -= frameDuration;
                animation.frameIndex = (animation.frameIndex + 1) % frameCount;
            }
        });
}

}  // namespace core
