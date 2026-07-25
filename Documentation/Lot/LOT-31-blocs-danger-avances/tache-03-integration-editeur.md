# TACHE-03 — Intégration éditeur (palette, rendu, liaison/portée) {#lot-31-tache-03-integration-editeur}

**Lot :** [LOT-31](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics`, `HMI/Interface` · **Statut :** ⬜

## Contexte
Rend les sept nouveaux types **peignables** depuis la palette (`EX-EDIT-002`), visuellement
distincts (`EX-EDIT-016`), et permet d'éditer leurs liaisons/portée sans taper de JSON à la main
(`EX-EDIT-001`) — dans la limite actée par l'epic (pas de nouveau widget numérique pour la
vitesse/période, cf. exclusions).

## Travail à réaliser
- **`TilePalette.cpp`** : la catégorie « Piège » (`Category`), aujourd'hui `pushStandalone` (une
  seule feuille), devient un **en-tête de catégorie** classique (comme « Tuile »/« Interactif ») :
  - `Danger` reste une feuille directe (« Classique »).
  - Nouveau sous-groupe « Directionnel » : quatre feuilles (`DangerUp`/`Down`/`Left`/`Right` — « Haut
    »/« Bas »/« Gauche »/« Droite »), même patron que le sous-groupe « Pente ».
  - Trois nouvelles feuilles directes : `DangerMover` (« Mobile »), `DangerSwitched` (« Commuté »),
    `DangerBlink` (« Clignotant »).
- **`TileVisuals.cpp`** : rendu des sept types. Directionnel = sprite/teinte du danger existant
  restreint à la bande retournée par `core::dangerHitbox` (cohérence visuel/hitbox garantie par la
  même fonction que TACHE-01/02, même principe que `tileVisualScale` pour les blocs réduits).
  Mobile/Commuté/Temporisé = variante de teinte ou icône distincte du danger classique (à
  trancher en fonction du jeu de sprites disponible ; cohérent avec la mention mémoire sur les
  couleurs assourdies déjà connues dans l'éditeur pour `Switch`/`Door`/`PressurePlate`).
- **`EditorScreen.cpp`** :
  - `handleLinkClick` généralisé pour accepter `DangerSwitched` comme cible valide au même titre que
    `Door` (clic déclencheur, clic cible) — réutilise `_pendingLink`/`linkMechanism` de TACHE-01
    sans nouveau mécanisme.
  - Nouveau geste pour `DangerMover` : après avoir peint la tuile, un second clic sur une case de la
    même ligne/colonne définit l'**axe** et la **portée** (distance jusqu'à la case cliquée),
    dessiné comme un aperçu de trajectoire (segment) pendant l'édition — même esprit que le clic de
    liaison, pas un widget numérique séparé (cohérent avec l'exclusion de l'epic).
  - Rendu des liaisons `DangerSwitched` distinguable des liaisons de porte (`EX-EDIT-016` déjà
    acquis pour plusieurs liaisons simultanées — étendre la palette de couleurs/motifs existante
    plutôt que la dupliquer).
- **Niveau de démonstration** : un nouveau `Source/Elements/Levels/demo-dangers-avances.json`
  intégré à la séquence jouée (`ScreenId::Game`, `Source/HMI/main.cpp`) et exerçant **les quatre**
  variantes (au moins une case de chacune), à la suite du niveau `demo-bloc`/avant `demo-final`
  (ordre de difficulté croissante, cf. `Source/Elements/Levels/README.md`).

## Fichiers impactés
- `Source/HMI/Editor/TilePalette.h`/`.cpp` (catégorie Piège restructurée).
- `Source/HMI/Graphics/TileVisuals.cpp`/`.h` (rendu des sept types).
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (liaison `DangerSwitched`, geste de portée
  `DangerMover`).
- `Source/Elements/Levels/demo-dangers-avances.json` (nouveau, séquence de démo).
- `Source/HMI/main.cpp` (insertion dans la séquence jouée).
- Tests : `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp` (nouvelles entrées/catégories),
  `Source/Test/Systeme/test_parcours_edition.cpp` (peinture/liaison/portée des nouveaux types,
  round-trip enregistrement/rechargement), `Source/Test/Systeme/test_parcours_complet.cpp`
  (`demo-dangers-avances` rejoué à sa place dans la séquence, avec un scénario d'entrées
  déterministe qui traverse les quatre variantes sans échec inattendu **et** un scénario qui
  provoque l'échec attendu sur chacune).

## Tests (obligatoires)
- Chaque nouveau type est sélectionnable depuis la palette et peint la bonne valeur de `TileType`.
- Une liaison déclencheur → `DangerSwitched` se crée/se défait comme une liaison déclencheur →
  `Door` (mêmes clics, même `_draft.mechanisms()`).
- Le geste de portée pour `DangerMover` produit l'axe/la distance attendus dans le fichier
  enregistré.
- `test_tile_palette.cpp` : la restructuration de la catégorie « Piège » ne casse aucun test
  existant sur `Danger` (toujours sélectionnable, toujours dans la catégorie « Piège »).
- `scripts/check_demo_sequence.py` retourne un code de sortie `0` (la liste rejouée par
  `test_parcours_complet.cpp` reste identique à celle chargée par `Source/HMI/main.cpp`).

## Définition de fait (DoD)
- Les sept types sont éditables de bout en bout (peinture, liaison/portée, enregistrement,
  rechargement, essai immédiat) sans édition manuelle de JSON.
- Couverture des **trois niveaux de test** du projet pour l'ensemble du lot : Unit (TACHE-01/02,
  brique isolée), Integration (TACHE-02, `DangerController` assemblé à `LevelOutcome`/
  `CharacterPhysicsSystem`), Système (cette tâche — `demo-dangers-avances` jouée bout en bout dans
  la séquence réelle, et round-trip d'édition).

## Exigences
`EX-EDIT-001`, `EX-EDIT-002`, `EX-EDIT-003`, `EX-EDIT-016`, `EX-GP-050`, `EX-GP-051`, `EX-GP-052`,
`EX-GP-053`.
