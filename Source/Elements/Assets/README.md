# Elements/Assets/

**Assets graphiques éditables hors code** (`LOT-39`, `EX-REN-042`) : atlas de tuiles au format PNG,
copié à côté de l'exécutable au build (patron `Levels`/`Localization`) et chargé au démarrage par
`hmi::TextureAtlas` (`hmi::AssetPaths`/`hmi::TextureLoader`).

## Contenu

- `atlas.png` — l'atlas de tuiles + images d'animation du personnage. Convention de grille
  (inchangée depuis la génération procédurale historique, `hmi::TextureAtlas::TILE_SIZE`/
  `TILES_PER_SIDE`/`PLAYER_FRAME_SIZE`/`PLAYER_FRAME_COLUMNS`) :
  - une grille de tuiles carrées de 16×16 pixels, `TILES_PER_SIDE` (5) tuiles par côté, dans le
    coin haut-gauche de l'image ;
  - sous cette grille, une ou plusieurs lignes d'images d'animation du personnage (16×16 chacune,
    même largeur de grille), dans l'ordre `Idle` puis `Run` puis `Jump`
    (`hmi::flatPlayerFrameIndex`).
  - `hmi::TextureAtlas::tile(colonne, ligne)`/`playerFrameRegion(clip, index)` calculent la région
    à partir de ces seules constantes — **remplacer le fichier suffit** à changer l'apparence, sans
    toucher au code, tant que la grille (dimensions, position des cases utilisées par
    `hmi::TileVisuals::regionForTile`) reste respectée.

## Éditer / régénérer

- **Remplacer une tuile** : éditer `atlas.png` directement (n'importe quel éditeur d'image), en
  respectant la grille ci-dessus. Relancer l'éditeur pour voir le résultat (pas de rechargement à
  chaud à ce stade).
- **Régénérer `atlas.png` à partir de la génération procédurale historique** (référence, en cas de
  divergence ou pour repartir d'une base propre) :
  `ProjectGaming.exe --export-atlas=Source/Elements/Assets/atlas.png` (option de développement,
  n'ouvre aucune fenêtre — voir `Documentation/Guide/guide-rendu.md`).
- **Asset absent ou illisible** : `hmi::TextureAtlas` retombe automatiquement sur la génération
  procédurale (`EX-NFR-040`), sans bloquer le rendu — utile pour développer sans art final.

> Anciennement `Textures/` (réservé, jamais peuplé) — ce dossier est le point d'entrée réel des
> assets graphiques depuis le LOT-39, nommé `Assets/` pour suivre le même patron que `Levels/` et
> `Localization/`.
