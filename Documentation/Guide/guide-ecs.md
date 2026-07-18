# ECS : entités, composants, systèmes {#guide-ecs}

Le moteur utilise une architecture **Entity-Component-System** maison
([ECS](https://en.wikipedia.org/wiki/Entity_component_system) ⧉), dans `Source/Core/Ecs`.

- **Entité** (`core::Entity`) : un simple **identifiant** (index + génération), sans données.
- **Composant** : une **donnée pure** (struct sans logique, `EX-ARCH-011`) — p. ex.
  `core::Transform` (position/échelle/rotation), `core::Velocity`, `core::Collider`, `core::Sprite`,
  `core::Player`.
- **Système** : la **logique**, qui parcourt les entités possédant un jeu de composants donné et les
  fait évoluer — p. ex. `core::CharacterPhysicsSystem`, `core::MovementSystem`.

La règle : **les données dans les composants, la logique dans les systèmes**. Un composant ne
contient jamais de comportement ; un système ne stocke pas d'état de jeu (il le lit/écrit dans les
composants).

## Le `World`

`core::World` détient les entités et leurs composants. API essentielle :

- `createEntity()` / `destroyEntity(e)` — cycle de vie ;
- `addComponent<T>(e, valeur)` / `getComponent<T>(e)` / `hasComponent<T>(e)` / `removeComponent<T>(e)` ;
- `view<A, B, …>()` — une **vue** sur les entités possédant **tous** les composants listés.

## Sparse sets et vues

Chaque type de composant est stocké dans un `core::ComponentPool` fondé sur un
[sparse set](https://research.swtch.com/sparse) ⧉ : un tableau **dense** des composants (itération
cache-friendly) + un index **creux** entité → position. Avantages : ajout/suppression/accès en
**O(1)** et itération rapide.

`core::View` (obtenue via `World::view<…>()`) parcourt le plus petit pool et ne retient que les
entités présentes dans **tous** les pools demandés. On l'utilise avec `.each([](Entity, A&, B&,
…){ … })`. Exemple type (le mouvement) :

```cpp
world.view<Transform, Velocity>().each(
    [dt](Entity, Transform& t, Velocity& v) { t.position += v.value * dt; });
```

C'est exactement le motif de tous les systèmes : une vue, une lambda, la logique.

## Voir aussi
- `core::World`, `core::Entity`, `core::EntityManager`, `core::ComponentPool`, `core::View`.
- `core::ISystem`, `core::MovementSystem`, `core::CharacterPhysicsSystem`.
- @ref guide-physique (le système de physique), @ref guide-maths.
