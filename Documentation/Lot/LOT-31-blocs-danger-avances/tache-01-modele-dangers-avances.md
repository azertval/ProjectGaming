# TACHE-01 — Modèle (types, format, généralisation des liaisons) {#lot-31-tache-01-modele-dangers-avances}

**Lot :** [LOT-31](epic.md) · **Emplacement :** `Core/Levels` · **Statut :** ⬜

## Contexte
Pose les fondations de données des quatre variantes, indépendamment de toute simulation (TACHE-02)
ou de tout rendu (TACHE-03) : nouveaux `TileType`, format JSON (`LevelLoader`/`LevelWriter`), et
généralisation de `core::Mechanism` pour qu'une liaison interrupteur/plaque puisse cibler un danger
commuté en plus d'une porte.

## Travail à réaliser
- **`Source/Core/Levels/TileType.h`** : sept nouvelles valeurs —
  `DangerUp`/`DangerDown`/`DangerLeft`/`DangerRight` (directionnel, `EX-GP-050` ; nom = bord
  **mortel** de la case, même convention que `SlopeUpRight` où le suffixe décrit la géométrie, pas
  un mouvement), `DangerMover` (mobile, `EX-GP-051`), `DangerSwitched` (commuté, `EX-GP-052`),
  `DangerBlink` (temporisé, `EX-GP-053`). Doc Doxygen de l'enum étendue, même style que
  l'existant (une phrase par famille, référence aux exigences).
- **`core::dangerHitbox(TileType type, int col, int row)`** (nouveau, `TileType.h` ou nouveau
  fichier `Core/Levels/DangerGeometry.h` si la fonction grossit) : retourne l'`Aabb` **mortelle**
  d'une case — case pleine pour `Danger`, bande étroite alignée sur le bord désigné pour les quatre
  variantes directionnelles, case pleine pour `DangerMover`/`DangerSwitched`/`DangerBlink` (leur
  variation est temporelle, pas géométrique). Seule source de vérité, consommée par TACHE-02
  (résolution de fin de niveau) **et** TACHE-03 (aperçu visuel des pics dans l'éditeur) — même
  garantie de non-divergence que `core::tileVisualScale` (`EX-GP-005`).
- **Format JSON** (`LevelLoader.cpp`/`LevelWriter.cpp`) : nouveaux types `dangerUp`/`dangerDown`/
  `dangerLeft`/`dangerRight`, `dangerMover` (champs optionnels `axis` : `"horizontal"`/`"vertical"`,
  `range` : entier en cases, défaut de conception si absent — cf. exclusion « pas d'édition
  numérique » de l'epic), `dangerSwitched` (champ `opensWith`, comme `door`), `dangerBlink` (champs
  optionnels `period`/`phase` en pas fixes, défaut de conception si absents).
- **`core::Mechanism`** (`Level.h`) : généralisé pour référencer une **cible** qui est soit une
  porte soit un danger commuté, plutôt que `doorPosition` nommé en dur — probablement un champ
  `targetPosition` + enum `TargetKind { Door, DangerSwitched }` résolu au chargement selon le type
  de tuile réellement présent à cette position (pas de nouveau champ dans le fichier : le type de
  la tuile cible suffit à distinguer les deux cas, comme aujourd'hui `switch` vs `pressurePlate` est
  distingué par le type de la tuile **origine**). Vérifier tous les appelants existants de
  `Mechanism::doorPosition` (`MechanismController`, éditeur) avant de renommer.
- **Validation** (`EX-LVL-004`) : une liaison `opensWith` doit référencer un `switch`/
  `pressurePlate` existant **et** pointer vers une `door` **ou** une `dangerSwitched` (pas un autre
  type) ; `dangerMover` avec une portée qui sortirait de la grille est invalide.

## Fichiers impactés
- `Source/Core/Levels/TileType.h` (sept types, `dangerHitbox`).
- `Source/Core/Levels/Level.h` (`Mechanism` généralisé).
- `Source/Core/Levels/LevelLoader.cpp`/`.h`, `LevelWriter.cpp`/`.h` (parsing/sérialisation, sept
  types + champs optionnels).
- `Source/Core/Levels/LevelDraft.h`/`.cpp` (peinture des nouveaux types depuis l'éditeur —
  `linkMechanism` généralisé si nécessaire).
- Tests : `Source/Test/Unit/Core/Levels/test_level_loader.cpp`, `test_level_writer.cpp`,
  `test_level_draft.cpp`, `test_level.cpp` (nouveaux cas par type).

## Tests (obligatoires)
- Round-trip JSON (écriture puis lecture) pour chacun des sept nouveaux types, avec et sans champs
  optionnels.
- `dangerHitbox` retourne le rectangle attendu pour chaque bord directionnel, et la case pleine pour
  `Danger`/`DangerMover`/`DangerSwitched`/`DangerBlink`.
- Une liaison `opensWith` peut cibler une `dangerSwitched` aussi bien qu'une `door` ; le
  comportement existant (`switch`/`pressurePlate` → `door`) reste inchangé bit à bit (tests
  existants toujours verts sans modification).
- Validation rejette une liaison vers un type qui n'est ni `door` ni `dangerSwitched`, et une
  portée de `dangerMover` hors bornes.

## Définition de fait (DoD)
- Compile, testé, **aucun** appelant en jeu ou en éditeur pour l'instant (simulation en TACHE-02,
  palette/rendu en TACHE-03) — tâche purement additive côté modèle, comme le veut l'ordre du
  processus de lot (modèle avant intégration).

## Exigences
`EX-GP-050`, `EX-GP-051`, `EX-GP-052`, `EX-GP-053`, `EX-LVL-002`, `EX-LVL-004`.
