#include "Core/Ecs/Systems/ParticleSystem.h"

#include <cmath>

#include "Core/Ecs/World.h"
#include "Core/Math/DeterministicRandom.h"

namespace core {

ParticleSystem::ParticleSystem(std::uint64_t seed) noexcept : _baseSeed(seed) {}

Entity ParticleSystem::reserveSlot(World& world) {
    if (static_cast<int>(_order.size()) >= MAX_PARTICLES) {
        // Recyclage deterministe de la plus ANCIENNE (front de _order) : jamais dependant du
        // sparse set de l'ECS, qui ne garantit aucun ordre stable apres un swap-and-pop.
        const Entity recycled = _order.front();
        _order.erase(_order.begin());
        world.destroyEntity(recycled);
    }
    const Entity entity = world.createEntity();
    _order.push_back(entity);
    return entity;
}

void ParticleSystem::emit(World& world, const ParticleEffect& effect, Vector2 origin,
                          Vector2 direction, ParticleKind kind) {
    const float baseAngle = std::atan2(direction.y, direction.x);
    for (int i = 0; i < effect.count; ++i) {
        const Entity entity = reserveSlot(world);
        // Reseede POUR CETTE particule : le resultat ne depend que du triplet (graine de base,
        // pas courant, identifiant d'entite), jamais de l'ordre/nombre d'appels a emit() au sein
        // du meme pas (EX-NFR-002).
        DeterministicRandom rng{deriveSeed(_baseSeed, _stepIndex, entity.index)};
        const float speed = rng.nextRange(effect.speedMin, effect.speedMax);
        const float angle = baseAngle + rng.nextRange(-effect.spreadRadians, effect.spreadRadians);
        const float life = rng.nextRange(effect.lifeMin, effect.lifeMax);
        const Vector2 velocity{std::cos(angle) * speed, std::sin(angle) * speed};
        world.addComponent(entity, Particle{origin, velocity, life, life, kind});
    }
}

void ParticleSystem::update(World& world, float fixedDelta) {
    ++_stepIndex;
    // Compaction en place, dans l'ordre d'emission (_order reste la source de verite, jamais
    // world.view<Particle>() -- voir en-tete de la classe).
    std::size_t writeIndex = 0;
    for (std::size_t i = 0; i < _order.size(); ++i) {
        const Entity entity = _order[i];
        Particle& particle = world.getComponent<Particle>(entity);
        particle.life -= fixedDelta;
        if (particle.life <= 0.0f) {
            world.destroyEntity(entity);
            continue;  // disparait CE pas : pas recopiee dans le tableau compacte.
        }
        particle.position += particle.velocity * fixedDelta;
        _order[writeIndex++] = entity;
    }
    _order.resize(writeIndex);
}

void ParticleSystem::clear(World& world) {
    for (const Entity entity : _order) {
        world.destroyEntity(entity);
    }
    _order.clear();
}

}  // namespace core
