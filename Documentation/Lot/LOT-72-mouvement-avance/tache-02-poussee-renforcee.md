# TACHE-02 — Poussée renforcée pendant un dash {#lot-72-tache-02-poussee-renforcee}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Gameplay/BlockController.*`,
`Source/HMI/Game` · **Statut :** à faire

## Contexte
`BlockController` (`EX-GP-022`) pousse un bloc d'**une case** par contact, que le personnage marche
ou dashe contre lui. `EX-GP-057` distingue le second cas : un bloc percuté **pendant un dash** doit
être repoussé de **plusieurs cases en un seul pas fixe**, jusqu'au premier obstacle ou une distance
maximale, pour un ressenti d'impact cohérent avec la vitesse du dash.

## Travail à réaliser
- `BlockController::update`/`pushBlocks` : nouveau paramètre indiquant que le personnage est en dash
  (et sa direction), par exemple `bool isDashing` en plus de `moveIntentX` — ou dérivé directement de
  la vitesse du personnage transmise par l'appelant.
- Quand `isDashing` est vrai côté personnage poussant, au lieu de déplacer le bloc d'une case,
  avancer case par case (réutilisant `isFree`) tant que la case suivante est libre, jusqu'à un
  maximum configurable (`PhysicsConfig::dashPushMaxCells` ou équivalent) ou jusqu'au premier
  obstacle.
- Vérifier le point d'appel (`hmi::GameSession` ou équivalent) pour transmettre l'état de dash du
  personnage (`Player::dashTimer > 0`) à `BlockController::update`.

## Fichiers impactés
- `Source/Core/Gameplay/BlockController.h`/`.cpp`.
- Point d'appel dans `Source/HMI/Game` (transmission de l'état de dash).
- `Source/Core/Physics/PhysicsConfig.h` (distance maximale de poussée renforcée).
- Tests d'intégration.

## Tests (obligatoires)
- **Poussée renforcée** : un dash (normal ou boosté, TACHE-01) contre un bloc le déplace de plusieurs
  cases en un seul pas fixe.
- **Arrêt au premier obstacle** : la poussée renforcée ne traverse jamais un mur/bloc/mécanisme
  fermé.
- **Poussée simple inchangée** : un contact en marche normale (hors dash) pousse toujours d'une case
  par pas, comme avant le lot.
- **Blocs réduits** (`BlockHalf`/`BlockQuarter`, `EX-GP-005`) : même comportement de poussée
  renforcée que les blocs pleins.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- Réutiliser `isFree` de `BlockController` sans dupliquer sa logique de case libre (pentes/arrondis,
  autres blocs, mécanismes).
- Ne pas modifier le comportement de chute (`dropBlocks`) ni de portage sur plateforme mobile
  (`carryBlocksOnPlatforms`), hors de portée de cette tâche.
- Le bloc reste toujours **aligné sur la grille** (jamais de position infra-case), comme le reste de
  `BlockController`.

## Définition de fait (DoD)
- Poussée renforcée fonctionnelle et **testée** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-057`, `EX-GP-022`, `EX-GP-017`, `EX-NFR-002`, `EX-ARCH-011`.
