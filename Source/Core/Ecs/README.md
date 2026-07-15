# Core/Ecs/

Entity-Component-System (implémentation **maison**), cœur de la simulation.

- Entités : handles générationnels.
- Composants : données pures (sparse sets) — sous-dossier `Components/`.
- Systèmes : logique itérant sur les composants — sous-dossier `Systems/`.
- `World` : façade, orchestration au pas de temps fixe.

Implémenté (LOT-03) :
- `Entity` — handle générationnel `{ index, generation }`, `INVALID_ENTITY`.
- `EntityManager` — `create` / `destroy` / `isAlive`, recyclage des index par liste libre.
- `ComponentPool<T>` — sparse set typé (`add` / `get` / `has` / `remove` / `removeIfPresent`), tableau dense contigu, suppression par swap-and-pop.

À venir (LOT-03) : vues multi-composants, systèmes & `World`.

Réf. specs : `EX-ARCH-010`, `EX-ARCH-011`, `EX-ARCH-012`, `EX-ARCH-100`.
