# Core/Levels/

Modèle de niveau, chargement/sérialisation JSON, et projection vers l'ECS.

- `TileType` — types de tuiles d'un niveau (`Empty`, `Solid`, `Danger`, `Entry`, `Exit`,
  `Switch`, `PressurePlate`, `Door`) et utilitaires associés (`isSolid`).
- `TileMap` — grille dense `width × height` de `TileType`, origine haut-gauche.
- `GridPosition` — position entière dans la grille (colonne, ligne).
- `Level` — niveau assemblé et immuable : `TileMap`, entrée/sortie, mécanismes
  (interrupteur/plaque ↔ porte), budgets de mouvements (saut/dash).
- `LevelLoader` — chargement d'un niveau depuis le format JSON (liste de tuiles-objets),
  avec validation (`EX-LVL-004`) et résultat récupérable (`LevelLoadResult`, jamais d'exception).
- `LevelWriter` — sérialisation d'un niveau vers le format JSON, symétrique à `LevelLoader`
  (régénère des identifiants d'interrupteur déterministes).
- `LevelDraft` — représentation **mutable** d'un niveau en cours d'édition (peinture, undo/redo,
  redimensionnement) ; `toLevel()` revalide via le même chemin que `LevelLoader`.
- `LevelOutcome` — issue d'un niveau (en cours / gagné / perdu) évaluée depuis la position du
  personnage (`EX-GP-030`…`EX-GP-032`).
- `LevelScene` (`buildLevelScene`) — projette une `TileMap` en entités ECS (une tuile non vide =
  une entité `Transform` + `Sprite`), pont vers le rendu.

Réf. specs : `EX-LVL-001`…`EX-LVL-012`, `EX-EDIT-001`…`EX-EDIT-017`.
