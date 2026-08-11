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
constexpr float MOVING_THRESHOLD = 0.01F;

// Noms des clips du personnage, migres tels quels depuis l'ancien enum (LOT-18), etendus par
// LOT-48 TACHE-02 : seule source de verite de ces chaines, partagee entre la construction du jeu
// (playerClipSet) et la projection (targetClipName) -- les dupliquer ailleurs risquerait de les
// laisser diverger silencieusement.
constexpr const char* PLAYER_CLIP_NAME_IDLE = "idle";
constexpr const char* PLAYER_CLIP_NAME_RUN = "run";
constexpr const char* PLAYER_CLIP_NAME_JUMP = "jump";
constexpr const char* PLAYER_CLIP_NAME_FALL = "fall";
constexpr const char* PLAYER_CLIP_NAME_LAND = "land";
constexpr const char* PLAYER_CLIP_NAME_WALLSLIDE = "wallslide";
constexpr const char* PLAYER_CLIP_NAME_DASH = "dash";

// Vrai si l'index designe l'un des clips AERIENS (saut, chute, glissade murale) : c'est depuis
// l'un de ces trois clips qu'un contact au sol declenche la transition d'atterrissage
// ci-dessous -- pas depuis idle/run/dash/land lui-meme (LOT-48 TACHE-02).
bool isAirborneClip(int clipIndex) {
    return clipIndex == PLAYER_CLIP_JUMP || clipIndex == PLAYER_CLIP_FALL ||
           clipIndex == PLAYER_CLIP_WALLSLIDE;
}

// Determine le nom du clip cible a partir de l'etat physique courant et du clip actuellement
// resolu (projection, cf. en-tete). Ordre de priorite EXPLICITE (LOT-48 TACHE-02), du plus fort
// au plus faible :
//   1. dash       -- dashTimer actif, domine tout le reste (y compris un atterrissage en cours) ;
//   2. land       -- transition d'atterrissage : soit deja en cours (clip courant == land, on la
//                    laisse se terminer), soit qui debute a l'instant (au sol ce pas-ci, alors
//                    qu'un clip AERIEN etait resolu au pas precedent -- comparaison avec le pas
//                    precedent, comme les transitions de mecanismes, LOT-47 TACHE-02) ;
//   3. wallslide  -- contact mural en l'air (wallDirection non nul, pas au sol) ;
//   4. fall/jump  -- en l'air, signe de la vitesse verticale (Y vers le bas : positive = chute) ;
//   5. run/idle   -- au sol, seuil de vitesse horizontale existant, inchange depuis LOT-18.
const char* targetClipName(const Player& player, const Velocity& velocity,
                           const Animation& animation) {
    if (player.dashTimer > 0.0F) {
        return PLAYER_CLIP_NAME_DASH;
    }
    if (animation.clipIndex == PLAYER_CLIP_LAND ||
        (player.grounded && isAirborneClip(animation.clipIndex))) {
        return PLAYER_CLIP_NAME_LAND;
    }
    if (!player.grounded) {
        if (player.wallDirection != 0.0F) {
            return PLAYER_CLIP_NAME_WALLSLIDE;
        }
        return (velocity.value.y > 0.0F) ? PLAYER_CLIP_NAME_FALL : PLAYER_CLIP_NAME_JUMP;
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
    animation.elapsed = 0.0F;
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
        idle.frameDuration = 0.5F;
        idle.endMode = ClipEndMode::Loop;
        clips->addClip(idle);

        AnimationClip run;
        run.name = PLAYER_CLIP_NAME_RUN;
        run.frames = {0, 1, 2, 3};
        run.frameDuration = 0.1F;
        run.endMode = ClipEndMode::Loop;
        clips->addClip(run);

        AnimationClip jump;
        jump.name = PLAYER_CLIP_NAME_JUMP;
        jump.frames = {0};  // pose unique : jamais animee (une seule image, cf. advanceAnimation).
        jump.frameDuration = 0.0F;
        jump.endMode = ClipEndMode::Loop;
        clips->addClip(jump);

        // Quatre clips LOT-48 (TACHE-02), sans pose procedurale dediee -- HMI les fait retomber
        // sur le clip le plus proche declare (hmi::resolveDeclaredPlayerClip) aussi bien pour
        // l'atlas procedural que pour une spritesheet externe partielle.
        AnimationClip fall;
        fall.name = PLAYER_CLIP_NAME_FALL;
        fall.frames = {0};  // pose unique, comme jump : distinguee par le NOM du clip, pas l'image.
        fall.frameDuration = 0.0F;
        fall.endMode = ClipEndMode::Loop;
        clips->addClip(fall);

        AnimationClip land;
        land.name = PLAYER_CLIP_NAME_LAND;
        // Transition jouee UNE fois (LOT-47 TACHE-02 pour le meme patron cote mecanismes), puis
        // bascule sur idle -- immediatement corrigee vers run par le pas suivant si le personnage
        // est deja en mouvement (targetClipName est reevalue a chaque pas, cf. en-tete).
        land.frames = {0, 1};
        land.frameDuration = 0.08F;
        land.endMode = ClipEndMode::OneShot;
        land.nextClip = PLAYER_CLIP_NAME_IDLE;
        clips->addClip(land);

        AnimationClip wallslide;
        wallslide.name = PLAYER_CLIP_NAME_WALLSLIDE;
        wallslide.frames = {0};  // pose unique : contact mural continu, pas de cycle d'images.
        wallslide.frameDuration = 0.0F;
        wallslide.endMode = ClipEndMode::Loop;
        clips->addClip(wallslide);

        AnimationClip dash;
        dash.name = PLAYER_CLIP_NAME_DASH;
        dash.frames = {0};  // pose unique : le dash est bref (Player::dashTimer), pas de cycle.
        dash.frameDuration = 0.0F;
        dash.endMode = ClipEndMode::Loop;
        clips->addClip(dash);

        // Ordre de construction = PLAYER_CLIP_IDLE/RUN/JUMP/FALL/LAND/WALLSLIDE/DASH
        // (AnimationSystem.h) : seule source de verite partagee avec la traduction en region
        // d'atlas cote HMI (LOT-46 TACHE-04, etendue LOT-48).
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
    if (clip->frames.size() <= 1 || clip->frameDuration <= 0.0F) {
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
            animation.elapsed = 0.0F;
            break;
        }
        animation.clipIndex = nextIndex;
        animation.frameIndex = 0;
        clip = &clips.clipAt(animation.clipIndex);
        if (clip->frames.size() <= 1 || clip->frameDuration <= 0.0F) {
            animation.elapsed = 0.0F;
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
            justChanged = setTargetClip(animation, *animation.clips,
                                        targetClipName(player, velocity, animation));
        }
        if (justChanged) {
            return;
        }
        advanceAnimation(animation, fixedDelta);
    });
}

}  // namespace core
