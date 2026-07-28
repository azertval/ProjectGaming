# HMI/Graphics/

Rendu Direct3D 11 (wrapper mince, pas de couche multi-backend).

## Pipeline
- `GraphicsDevice` — device, swap chain, cible de rendu, effacement + présentation V-Sync (modèle *flip*, `LOT-33`), redimensionnement ; expose `device()`/`context()`.
- `SpriteBatch` — pipeline 2D : quads texturés (`SpriteQuad`) et segments orientés (`LineQuad`, `LOT-37`), HLSL compilés à l'exécution, fusion alpha et échantillonnage *nearest* (pixel art) ; `begin`/`draw`/`end` avec *batching*. **Une seule texture liée par `begin`** — levé en `LOT-40`.
- `Camera2D` — projection monde → écran (16 px/unité, Y-bas, zoom) ; matrice de projection et conversions monde↔écran.
- `RoomGrid` — partition d'un niveau en **salles** (logique pure, `LOT-32`) : cadre la caméra salle par salle.

## Assets et textures
- `AssetPaths` — résolution pure d'un nom logique vers un chemin, dossier injecté (testable sans disque de production, `LOT-39`).
- `TextureLoader` — `decodeImageFile` (décodage `QImage` → RGBA non prémultiplié), `createTexture`, `loadTextureFromFile` : **point unique** de création de texture Direct3D 11. Jamais d'exception (`EX-NFR-040`).
- `TextureAtlas` — atlas de tuiles 16 px et d'images du personnage. Charge `Assets/atlas.png` depuis le disque, avec **repli sur la génération procédurale** si l'asset est absent ou illisible (`LOT-39`, `EX-REN-041`/`EX-REN-042`). `tile`/`playerFrameRegion` sont de la pure arithmétique de grille, `static` et testées sans GPU.
- `ProceduralAtlas` — génération CPU déterministe de l'atlas de repli (palette de tuiles, masques de silhouette des pentes, silhouette humanoïde du personnage). Aucune dépendance GPU ni Qt, entièrement testé.

## Pont vers l'ECS et l'éditeur
- `SpriteRenderer` — pont ECS → écran : lit `view<Transform, Sprite>`, trie par couche, dessine via `SpriteBatch` (**lecture seule** de l'ECS, `EX-ARCH-012`), avec interpolation (`PreviousPosition`, `EX-ARCH-031`).
- `PreviousPosition` — composant de **présentation** rangé dans le `core::World` mais écrit et lu par `HMI` seul ; porte la position au pas précédent pour l'interpolation.
- `DraftRenderer` — rendu du **brouillon d'édition** (`core::LevelDraft`) dans le viewport de l'éditeur : tuiles, grille, salles, liens de mécanismes, aperçu des outils, sans passer par l'ECS de jeu.
- `TileVisuals` — correspondance type de tuile → région d'atlas, partagée entre le jeu (`hmi::GameSession`) et l'éditeur (`hmi::DraftRenderer`, `hmi::PalettePanel`). **Point d'entrée unique** de l'apparence des tuiles.
- `GraphicsLog` — macros de journalisation du module.

## À venir
Le programme d'habillage `LOT-40` → `LOT-55` ajoute ici : registre de textures par nom logique et
invalidation, calques nommés, contrat d'asset, capture des primitives pour test sans GPU, culling
par salle (`LOT-40`) ; raccords automatiques de tuiles (`LOT-42`) ; rendu du fond (`LOT-44`) ;
animations pilotées par données (`LOT-46`) ; décors et parallaxe (`LOT-49`) ; police bitmap en scène
(`LOT-52`) ; ombres (`LOT-55`).

Réf. specs : `EX-REN-002`, `EX-REN-010`…`EX-REN-015`, `EX-REN-020`…`EX-REN-022`, `EX-REN-041`…`EX-REN-046`, `EX-REN-004`…`EX-REN-009`, `EX-ARCH-012`, `EX-ARCH-022`, `EX-ARCH-031`, `EX-ARCH-050`, `EX-NFR-004`, `EX-NFR-005`.
