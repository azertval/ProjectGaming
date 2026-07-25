# TACHE-02 — Intégration jeu (contrôleurs, résolution de fin de niveau) {#lot-31-tache-02-integration-jeu}

**Lot :** [LOT-31](epic.md) · **Emplacement :** `Core/Gameplay`, `Core/Levels` · **Statut :** ⬜

## Contexte
Fait vivre les quatre variantes chaque pas fixe et branche leur résultat sur
`core::resolveOutcome` (`LevelOutcome.cpp`), qui ne teste aujourd'hui qu'un recouvrement statique
avec `TileType::Danger`. Dépend du modèle posé en TACHE-01.

## Travail à réaliser
- **`core::DangerController`** (nouveau, `Core/Gameplay/DangerController.h`/`.cpp`, même patron que
  `MechanismController`/`BlockController` : construit depuis le `Level`, `update()` par pas fixe,
  état interne + accesseurs) :
  - **Mobile** (`DangerMover`) : position courante par tuile, aller-retour linéaire déterministe en
    fonction du nombre de pas fixes écoulés depuis le chargement (pas d'accumulation flottante —
    même exigence de déterminisme que `MechanismController`, `EX-NFR-002`).
  - **Commuté** (`DangerSwitched`) : consulte l'état déclencheur déjà résolu par
    `MechanismController::isDoorOpen`-équivalent pour les liaisons dont la cible est un danger
    (généralisation de TACHE-01) — actif (mortel) quand le déclencheur l'est, inverse de la porte.
    Envisager de fusionner ce contrôleur avec `MechanismController` plutôt que de dupliquer la
    résolution déclencheur↔cible ; trancher en fonction de ce qui reste le plus lisible une fois le
    code sous les yeux.
  - **Temporisé** (`DangerBlink`) : actif selon `(pasCourant + phase) % period < activeDuration`
    (constantes de conception si non fournies par le fichier, cf. TACHE-01).
  - **Directionnel** : aucune donnée mutable (géométrie pure, `core::dangerHitbox`) — pas de
    contrôleur nécessaire, juste consommé directement par `resolveOutcome`.
- **`core::resolveOutcome`** (`LevelOutcome.cpp`) : généralise le test de recouvrement pour couvrir
  les huit nouveaux types via `core::dangerHitbox` (géométrie) et l'état courant du
  `DangerController` (position mobile, activation commutée/temporisée) au lieu du recouvrement de
  case entière actuel. Signature étendue (nouveau paramètre `const DangerController&`, ou fusionné
  dans un paramètre existant) — vérifier tous les appelants (`GameScreen`, tests).
- **Câblage** (`GameScreen`/boucle de jeu) : instancie et met à jour le `DangerController` comme le
  sont déjà `MechanismController`/`BlockController`, dans le même ordre de pas fixe.

## Fichiers impactés
- `Source/Core/Gameplay/DangerController.h`/`.cpp` (nouveau).
- `Source/Core/Levels/LevelOutcome.h`/`.cpp` (signature + logique étendues).
- `Source/HMI/Interface/GameScreen.h`/`.cpp` (câblage du nouveau contrôleur, essai immédiat depuis
  l'éditeur inclus).
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt` (nouveau fichier source/test).
- Tests : `Source/Test/Unit/Core/Gameplay/test_danger_controller.cpp` (nouveau, `DangerController`
  isolé), `Source/Test/Unit/Core/Levels/test_level_outcome.cpp` (nouveaux cas, `resolveOutcome`
  isolé avec un `DangerController` factice), `Source/Test/Integration/test_danger_avance.cpp`
  (nouveau — `DangerController` **assemblé** avec `LevelOutcome` et `CharacterPhysicsSystem` sur
  plusieurs pas fixes, un scénario par variante : personnage qui longe un danger directionnel côté
  sûr puis le traverse par le bord mortel, qui se fait rattraper par un danger mobile, qui déclenche
  un interrupteur lié à un danger commuté, qui traverse un danger temporisé aux deux phases de son
  cycle).

## Tests (obligatoires)
- Directionnel : recouvrement du bord mortel → `Lost` ; recouvrement du reste de la case sans le
  bord → pas d'échec.
- Mobile : position courante suit exactement la formule déterministe attendue à un pas donné ;
  contact à la position **courante** tue, contact avec la position de **départ** une fois le
  mobile parti n'y tue plus.
- Commuté : mortel seulement quand le déclencheur lié est actif ; comportement de la porte liée au
  même déclencheur (s'il y en a une) inchangé.
- Temporisé : séquence active/inactive déterministe et reproductible sur plusieurs relances avec le
  même niveau ; deux tuiles de phases différentes désynchronisées comme attendu.
- Aucune régression sur `TileType::Danger` classique ni sur les mécanismes interrupteur↔porte
  existants (tests existants de `test_level_outcome.cpp`/`test_mechanism_controller.cpp` toujours
  verts sans modification de leurs assertions).

- `test_danger_avance.cpp` (intégration) : chaque scénario assemblé reproduit fidèlement le résultat
  attendu (`Lost` au bon moment, jamais avant/après) sur plusieurs pas fixes consécutifs, pas
  seulement à un instant isolé.

## Définition de fait (DoD)
- Les quatre variantes sont fonctionnelles **en jeu** avec des niveaux de test ad hoc (pas encore
  peignables depuis l'éditeur — palette en TACHE-03), logique couverte par des tests **aux deux
  niveaux** Unit (brique isolée) et Integration (`DangerController` assemblé avec `LevelOutcome`/
  `CharacterPhysicsSystem`) — le niveau Système suit en TACHE-03, une fois les variantes peignables
  et intégrées à un niveau réel.

## Exigences
`EX-GP-050`, `EX-GP-051`, `EX-GP-052`, `EX-GP-053`, `EX-GP-031`, `EX-GP-032`, `EX-NFR-002`.
