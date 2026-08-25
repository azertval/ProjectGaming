# TACHE-03 — Dash et pentes (suivi + glissade de sortie) {#lot-72-tache-03-dash-pentes}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems`, `Source/Core/Physics` ·
**Statut :** à faire

## Contexte
Pendant un dash, `CharacterPhysicsSystem` résout le déplacement par le seul balayage classique
(`sweepAabb`), **sans** passer par la passe de suivi de pente/plafond
(`resolveSlopeFollow`/`resolveCeilingSlopeFollow`) qu'utilise le déplacement normal. Une pente/arrondi
n'étant pas solide (`core::isSolid`), un dash qui la traverse ne la suit pas et peut clipper dedans.
`EX-GP-060` corrige ce trou et ajoute un effet de sortie : un dash qui se termine contre une pente
glisse le long d'elle au lieu de s'arrêter net.

## Travail à réaliser
- `CharacterPhysicsSystem` : pendant `dashTimer > 0`, appliquer aussi
  `resolveSlopeFollow`/`resolveCeilingSlopeFollow` en plus du balayage classique, exactement comme le
  fait le déplacement normal (mêmes fonctions, mêmes types de tuiles `SlopeUp*`/`RoundedUp*`/
  `ConcaveUp*` pour le sol, `SlopeDown*`/`RoundedDown*`/`ConcaveDown*` pour le plafond).
- À l'expiration du dash (`dashTimer` atteint zéro) ou à l'arrêt du balayage contre un obstacle, si la
  position finale repose sur une pente (`core::isFollowableSurface`), convertir la composante de
  vitesse horizontale résiduelle en vitesse **le long de la pente** (même principe que le déplacement
  normal en descente/montée de pente) plutôt que de la couper net.

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Dash en montée de pente** : la trajectoire suit la pente (pas de clip dans la matière, pas de
  saut artificiel au-dessus).
- **Dash en descente de pente** : idem, suit la pente vers le bas.
- **Dash sous un plafond incliné** : suit `resolveCeilingSlopeFollow` sans traverser.
- **Glissade de sortie** : un dash qui expire contre une pente convertit sa vitesse résiduelle en
  glissade le long de la pente, mesurable (le personnage continue à se déplacer selon l'inclinaison,
  pas un arrêt net).
- **Non-régression** : dash sur terrain plat inchangé ; dash contre un mur vertical toujours stoppé
  net (pas de « glissade » sur une surface non suivie).
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- Réutiliser telles quelles les fonctions de suivi de pente existantes (`core::resolveSlopeFollow`,
  `core::resolveCeilingSlopeFollow`, `core::slopeSurfaceHeight`) ; ne pas dupliquer la géométrie des
  pentes/arrondis.
- Les pentes de **plafond** ne sont jamais suivies latéralement par le personnage
  (`core::isFollowableSurface` reste `false` pour elles, voir `TileType.h`) : la glissade de sortie ne
  s'applique qu'aux pentes de **sol**.
- Composer proprement avec la suspension de gravité déjà en vigueur pendant un dash : la glissade de
  sortie prend le relais **après** l'expiration du dash, pas pendant.

## Définition de fait (DoD)
- Dash-pentes fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-060`, `EX-GP-017`, `EX-GP-003`, `EX-GP-004`, `EX-GP-006`, `EX-GP-007`, `EX-NFR-002`,
`EX-ARCH-011`.
