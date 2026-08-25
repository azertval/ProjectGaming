# TACHE-05 — Wall slide contrôlé {#lot-72-tache-05-wall-slide}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
`Player::wallDirection` (`EX-GP-016`) est déjà calculé par la physique pour le wall-jump, mais rien
n'exploite le contact mural avant le saut : le personnage tombe à vitesse normale le long d'un mur.
`EX-GP-059` ajoute un **wall slide contrôlé** : en l'air, au contact d'un mur, en poussant vers lui, la
chute est clampée à une vitesse plus lente.

## Travail à réaliser
- `PhysicsConfig` : `wallSlideSpeed` (vitesse de chute maximale en wall slide, inférieure à la vitesse
  terminale normale).
- `CharacterPhysicsSystem` : après résolution de la gravité et avant l'intégration de la position,
  si `!grounded && wallDirection != 0` et que l'entrée horizontale pousse **vers** le mur
  (`sign(moveX) == wallDirection`) et que `velocity.y > wallSlideSpeed` (chute), clamper
  `velocity.y = wallSlideSpeed`.
- Ne pas interférer avec le déclenchement du wall-jump existant : le wall slide ne fait que clamper la
  vitesse de chute, il ne modifie ni `wallDirection`, ni `wallJumpLockTimer`, ni la logique de saut.

## Fichiers impactés
- `Source/Core/Physics/PhysicsConfig.h`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Clamp actif** : en l'air, au contact d'un mur, en poussant vers lui, `velocity.y` ne dépasse
  jamais `wallSlideSpeed`.
- **Pas de clamp sans contact** : en l'air sans mur, chute libre normale inchangée.
- **Pas de clamp en poussant à l'opposé** : contact mural mais entrée horizontale vers l'autre côté →
  chute libre normale.
- **Wall-jump inchangé** : le déclenchement, la vitesse d'éjection et le verrouillage
  (`wallJumpLockTimer`) du wall-jump restent identiques à avant le lot, avec ou sans wall slide actif
  juste avant.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- Le clamp s'applique **après** le calcul normal de la gravité, pas en remplacement : une chute déjà
  plus lente que `wallSlideSpeed` (ex. juste après un saut) n'est pas accélérée jusqu'à ce plafond.
- Cohérent avec le **coyote time**/`jumpBufferTimer` existants : le wall slide ne les modifie pas.

## Définition de fait (DoD)
- Wall slide fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-059`, `EX-GP-016`, `EX-NFR-002`, `EX-ARCH-011`.
