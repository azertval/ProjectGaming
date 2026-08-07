#include "Core/Ecs/Systems/AnimationSystem.h"

#include <cmath>  // std::abs

#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Components/Player.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/World.h"

namespace core {

namespace {
// En dessous de ce seuil (unites monde/s), le personnage est considere immobile (bruit flottant
// tolere ; idle ne doit jamais se declencher a tort pour un personnage reellement en mouvement).
constexpr float MOVING_THRESHOLD = 0.01f;

// Noms des clips du personnage, migres tels quels depuis l'ancien enum (LOT-18) : seule source de
// verite de ces chaines, partagee entre la construction du jeu (playerClipSet) et la projection
// (targetClipName) -- les dupliquer ailleurs risquerait de les laisser diverger silencieusement.
constexpr const char* PLAYER_CLIP_NAME_IDLE = "idle";
constexpr const char* PLAYER_CLIP_NAME_RUN = "run";
constexpr const char* PLAYER_CLIP_NAME_JUMP = "jump";

// Determine le nom du clip cible a partir de l'etat physique courant (projection, cf. en-tete).
const char* targetClipName(const Player& player, const Velocity& velocity) {
    if (!player.grounded) {
        return PLAYER_CLIP_NAME_JUMP;
    }
    if (std::abs(velocity.value.x) > MOVING_THRESHOLD) {
        return PLAYER_CLIP_NAME_RUN;
    }
    return PLAYER_CLIP_NAME_IDLE;
}

// Bascule vers le clip nomme s'il differe du clip courant : reinitialise net l'image et le
// chronometre (EX-REN-012), comme l'ancien dispositif. Nom absent du jeu : repli deterministe sur
// le premier clip (ClipSet::indexOf/clipAt, EX-NFR-040), jamais un plantage.
// @return true si le clip courant a change (le pas qui detecte le changement ne doit alors pas
//         aussi faire progresser l'image : la transition consomme le pas sans accumulation).
bool setTargetClip(Animation& animation, const ClipSet& clips, const char* name) {
    const int found = clips.indexOf(name);
    const int resolved = found >= 0 ? found : 0;
    if (resolved == animation.clipIndex) {
        return false;
    }
    animation.clipIndex = resolved;
    animation.frameIndex = 0;
    animation.elapsed = 0.0f;
    return true;
}
}  // namespace

// Jeu de clips du personnage, migre tel quel depuis l'ancien dispositif fige (voir en-tete).
std::shared_ptr<const ClipSet> playerClipSet() {
    static const std::shared_ptr<const ClipSet> shared = [] {
        auto clips = std::make_shared<ClipSet>();

        AnimationClip idle;
        idle.name = PLAYER_CLIP_NAME_IDLE;
        idle.frames = {0, 1};
        idle.frameDuration = 0.5f;
        idle.endMode = ClipEndMode::Loop;
        clips->addClip(idle);

        AnimationClip run;
        run.name = PLAYER_CLIP_NAME_RUN;
        run.frames = {0, 1, 2, 3};
        run.frameDuration = 0.1f;
        run.endMode = ClipEndMode::Loop;
        clips->addClip(run);

        AnimationClip jump;
        jump.name = PLAYER_CLIP_NAME_JUMP;
        jump.frames = {0};  // pose unique : jamais animee (une seule image, cf. advanceAnimation).
        jump.frameDuration = 0.0f;
        jump.endMode = ClipEndMode::Loop;
        clips->addClip(jump);

        // Ordre de construction = PLAYER_CLIP_IDLE/RUN/JUMP (AnimationSystem.h) : seule source de
        // verite partagee avec la traduction en region d'atlas cote HMI (LOT-46 TACHE-04).
        return clips;
    }();
    return shared;
}

// Fait progresser une seule animation d'un pas fixe (voir en-tete).
void advanceAnimation(Animation& animation, float fixedDelta) {
    if (!animation.clips) {
        return;
    }
    const ClipSet& clips = *animation.clips;
    if (clips.clipCount() == 0) {
        return;
    }

    const AnimationClip* clip = &clips.clipAt(animation.clipIndex);
    if (clip->frames.size() <= 1 || clip->frameDuration <= 0.0f) {
        return;  // pose unique : jamais animee, aucun temps accumule (evite une derive d'elapsed).
    }

    animation.elapsed += fixedDelta;
    // Boucle unique (gere un pas fixe plus long qu'une duree d'image, sans image sautee ni
    // derive) : conservee du dispositif precedent, etendue a la bascule OneShot -> clip suivant,
    // pour ne jamais perdre de temps ecoule au changement de clip (determinisme, EX-NFR-002).
    while (animation.elapsed >= clip->frameDuration) {
        animation.elapsed -= clip->frameDuration;
        const int frameCount = static_cast<int>(clip->frames.size());
        if (animation.frameIndex + 1 < frameCount) {
            ++animation.frameIndex;
            continue;
        }
        if (clip->endMode == ClipEndMode::Loop) {
            animation.frameIndex = 0;
            continue;
        }
        // OneShot termine : bascule sur le clip suivant s'il existe, sinon reste net sur la
        // derniere image (pas de bouclage, pas de plantage sur un nom absent, EX-NFR-040).
        const int nextIndex = clips.indexOf(clip->nextClip);
        if (nextIndex < 0) {
            animation.elapsed = 0.0f;
            break;
        }
        animation.clipIndex = nextIndex;
        animation.frameIndex = 0;
        clip = &clips.clipAt(animation.clipIndex);
        if (clip->frames.size() <= 1 || clip->frameDuration <= 0.0f) {
            animation.elapsed = 0.0f;
            break;
        }
    }
}

// Applique un pas de simulation d'animation a toutes les entites animees (voir en-tete).
void AnimationSystem::update(World& world, float fixedDelta) {
    // Une seule traversee : la projection (personnage uniquement) precede la progression
    // (generale), entite par entite -- necessaire pour qu'un changement de clip detecte ce pas-ci
    // consomme le pas sans accumulation (EX-REN-012), exactement comme l'ancien dispositif.
    world.view<Animation>().each([&world, fixedDelta](Entity entity, Animation& animation) {
        if (!animation.clips) {
            return;  // securite : aucun jeu de clips assigne (EX-NFR-040).
        }

        bool justChanged = false;
        if (world.hasComponent<Player>(entity) && world.hasComponent<Velocity>(entity)) {
            const Player& player = world.getComponent<Player>(entity);
            const Velocity& velocity = world.getComponent<Velocity>(entity);
            justChanged =
                setTargetClip(animation, *animation.clips, targetClipName(player, velocity));
        }
        if (justChanged) {
            return;
        }
        advanceAnimation(animation, fixedDelta);
    });
}

}  // namespace core
