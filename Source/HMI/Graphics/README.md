# HMI/Graphics/

Rendu Direct3D 11 (wrapper mince, pas de couche multi-backend).

- `GraphicsDevice` — device, swap chain, cible de rendu, effacement + présentation V-Sync, redimensionnement ; expose `device()`/`context()`.
- `SpriteBatch` — pipeline 2D : quads texturés (HLSL compilés à l'exécution), fusion alpha (transparence) et échantillonnage *nearest* (pixel art) ; `begin`/`draw`/`end` avec *batching*.
- `TextureAtlas` — atlas de textures généré en code (tuiles 16px, dont une transparente, plus une grille d'images 16×16 pour la silhouette animée du personnage — repos, course, saut) ; expose la vue de texture et les régions.
- `Camera2D` — projection monde → écran (16 px/unité, Y-bas, zoom) ; matrice de projection et conversions monde↔écran.
- `SpriteRenderer` — pont ECS → écran : lit `view<Transform, Sprite>`, trie par couche, dessine via `SpriteBatch` (lecture seule de l'ECS), avec interpolation (`PreviousPosition`, `EX-ARCH-031`).
- `DraftRenderer` — rendu du **brouillon d'édition** (`core::LevelDraft`) dans le viewport de l'éditeur : tuiles, grille, aperçu des outils, sans passer par l'ECS de jeu.
- `RoomGrid` — partition d'un niveau en **salles** (logique pure, `LOT-32`) : cadre la caméra salle par salle.
- `TileVisuals` — correspondance type de tuile → région d'atlas (couleur procédurale), partagée entre le jeu (`hmi::GameSession`) et l'éditeur (`hmi::DraftRenderer`, `hmi::PalettePanel`).

Réf. specs : `EX-REN-002`, `EX-REN-010`…`EX-REN-022`, `EX-REN-032`, `EX-REN-033`, `EX-ARCH-022`, `EX-ARCH-050`.
