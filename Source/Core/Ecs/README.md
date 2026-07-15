# Core/Ecs/

Entity-Component-System (implémentation **maison**), cœur de la simulation.

- Entités : handles générationnels.
- Composants : données pures (sparse sets) — sous-dossier `Components/`.
- Systèmes : logique itérant sur les composants — sous-dossier `Systems/`.
- `World` : façade, orchestration au pas de temps fixe.

Implémenté (LOT-03) :
- `Entity` — handle générationnel `{ index, generation }`, `kInvalidEntity`.
- `EntityManager` — `create` / `destroy` / `isAlive`, recyclage des index par liste libre.

À venir (LOT-03) : stockage de composants (sparse sets), vues, systèmes & `World`.

Réf. specs : `EX-ARCH-010`, `EX-ARCH-011`, `EX-ARCH-012`, `EX-ARCH-100`.
