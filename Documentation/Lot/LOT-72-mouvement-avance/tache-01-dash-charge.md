# TACHE-01 — Charge de dash et dash boosté {#lot-72-tache-01-dash-charge}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/{Components,Systems}`,
`Source/Core/Physics` · **Statut :** à faire

## Contexte
Le dash (`EX-GP-017`) est aujourd'hui à vitesse et durée fixes. `EX-GP-056` ajoute un **dash chargé**
: maintenir la direction opposée à celle du prochain dash pendant un court délai, avant de dasher,
donne un **dash boosté** — plus rapide et/ou plus long qu'un dash normal.

## Travail à réaliser
- `Player.h` : ajouter `dashChargeTimer` (secondes accumulées de maintien de la direction opposée) et
  un indicateur de charge prête (ex. `dashBoostReady`).
- `PhysicsConfig` : `dashChargeHoldTime` (seuil pour considérer la charge prête),
  `dashBoostSpeedMultiplier`/`dashBoostDurationMultiplier` (ou un `dashBoostSpeed`/`dashBoostDuration`
  dédiés).
- `CharacterPhysicsSystem` :
  - Chaque pas fixe, si l'entrée horizontale est **opposée** à `Player::facing` (et non nulle),
    incrémenter `dashChargeTimer` ; sinon le remettre à zéro. Dès que `dashChargeTimer >=
    dashChargeHoldTime`, `dashBoostReady = true`.
  - Dans `applyDash`, si `dashBoostReady` au moment du déclenchement, appliquer `dashSpeed`/
    `dashDuration` majorés au lieu des valeurs normales, puis remettre `dashBoostReady` et
    `dashChargeTimer` à zéro (la charge est consommée qu'elle serve ou non à ce dash).
- Ne pas toucher à `dashChargesRemaining`/`dashesRemaining` (LOT-67) : le boost ne consomme et ne
  crée aucune charge supplémentaire, seulement le dash normalement déclenché.

## Fichiers impactés
- `Source/Core/Ecs/Components/Player.h`.
- `Source/Core/Physics/PhysicsConfig.h`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Charge puis dash** : maintenir la direction opposée au-delà du seuil, puis dasher → vitesse/durée
  supérieures à un dash normal, dans les 8 directions (y compris diagonales).
- **Charge insuffisante** : dasher avant le seuil → dash normal (pas de boost).
- **Charge annulée** : relâcher ou changer de direction avant le seuil → la charge suivante repart de
  zéro.
- **Non-consommation des charges/budget** : `dashChargesRemaining`/`dashesRemaining` se comportent
  exactement comme avant le lot, boost ou non.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- La détection de la direction opposée se fait sur `PlayerInput::moveX` comparé à `Player::facing`
  **avant** que le dash ne mette potentiellement à jour `facing` — ne pas inverser l'ordre.
- Le boost doit rester compatible avec un dash diagonal (direction normalisée comme aujourd'hui).
- Réutiliser le balayage existant (LOT-08) pour la résolution physique du dash boosté ; ne pas
  réimplémenter la collision.

## Définition de fait (DoD)
- Dash chargé fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-056`, `EX-GP-017`, `EX-GP-055`, `EX-NFR-002`, `EX-ARCH-011`.
