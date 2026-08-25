# TACHE-04 — Ground pound {#lot-72-tache-04-ground-pound}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/{Components,Systems}`,
`Source/HMI` · **Statut :** à faire

## Contexte
`EX-GP-058` ajoute un **ground pound** : en l'air uniquement, une chute accélérée dirigée
verticalement, qui se termine au contact du sol par un effet local (secousse caméra). Sur le modèle
des minuteries de *game feel* déjà portées par `Player` (coyote time, jump buffer), et des fronts déjà
exposés à l'IHM (`justJumped`, `squished`).

## Travail à réaliser
- `Player.h` : ajouter `groundPounding` (état actif) et un front d'atterrissage (ex.
  `justGroundPounded`), sur le modèle de `justJumped`.
- `PhysicsConfig` : `groundPoundSpeed` (vitesse de chute imposée pendant le pound).
- `CharacterPhysicsSystem` :
  - Déclenchement : en l'air (`!grounded`), direction verticale « bas » soutenue (`moveY > 0`) au-delà
    d'un court seuil (pour ne pas se déclencher sur un appui accidentel), et pas déjà en dash →
    `groundPounding = true`.
  - Pendant `groundPounding` : imposer `velocity.y = groundPoundSpeed` (gravité normale suspendue,
    comme le dash), ignorer l'entrée horizontale (comme le dash, pour un pound net).
  - À l'atterrissage (`grounded` devient vrai pendant que `groundPounding`) : couper
    `groundPounding`, lever `justGroundPounded` pour le pas courant (remis à faux au pas suivant, même
    patron que `justJumped`).
- Pas de règle spéciale pour les plaques de pression (`EX-GP-025`) : elles réagissent déjà à tout
  contact suffisamment massif, le ground pound n'a besoin d'aucun code dédié pour les activer.

## Fichiers impactés
- `Source/Core/Ecs/Components/Player.h`.
- `Source/Core/Physics/PhysicsConfig.h`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- `Source/HMI` : consommation de `justGroundPounded` pour l'effet cosmétique (secousse caméra), sur le
  modèle de la consommation existante de `justJumped`/`squished` (`hmi::GameEvents`, LOT-60).
- Tests d'intégration.

## Tests (obligatoires)
- **Déclenchement en l'air seulement** : au sol, l'appui « bas » ne déclenche pas de ground pound.
- **Vitesse de chute imposée** : pendant le pound, `velocity.y == groundPoundSpeed` en continu, entrée
  horizontale ignorée.
- **Front d'atterrissage** : `justGroundPounded` vrai exactement le pas où le contact au sol survient,
  faux au pas suivant.
- **Interaction plaque de pression** : un ground pound sur une plaque l'active exactement comme un
  atterrissage normal suffisamment massif.
- **Non-conflit avec le dash** : un dash en cours ne peut pas être interrompu par un ground pound (et
  réciproquement).
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- Ne pas dupliquer le patron `justJumped`/`squished` : même mécanique de front à usage externe, sans
  effet sur la simulation elle-même.
- La casse de blocs fragiles à l'impact est **explicitement exclue** du lot (aucun `TileType` fragile
  n'existe) : l'effet à l'atterrissage se limite au front consommé par l'IHM et à l'interaction
  normale, déjà existante, avec les mécanismes sensibles au poids.

## Définition de fait (DoD)
- Ground pound fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-058`, `EX-GP-025`, `EX-NFR-002`, `EX-ARCH-011`.
