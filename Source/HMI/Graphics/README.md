# HMI/Graphics/

Rendu Direct3D 11 (wrapper mince, pas de couche multi-backend).

## Pipeline
- `GraphicsDevice` — device, swap chain, cible de rendu, effacement + présentation V-Sync (modèle *flip*, `LOT-33`), redimensionnement ; expose `device()`/`context()`.
- `SpriteBatch` — pipeline 2D : quads texturés (`SpriteQuad`) et segments orientés (`LineQuad`, `LOT-37`), HLSL compilés à l'exécution, fusion alpha et échantillonnage *nearest* (pixel art) ; `begin`/`draw`/`end` avec *batching*. **Une seule texture liée par `begin`** : depuis `LOT-40`, le rendu émet une passe par groupe de texture plutôt que de modifier ce contrat.
- `Quad` — primitives de dessin (`SpriteQuad`, `LineQuad`) **sans dépendance Direct3D** (`LOT-40`) : c'est ce qui permet à la composition d'être testée sans GPU.
- `Camera2D` — projection monde → écran (16 px/unité, Y-bas, zoom) ; matrice de projection, conversions monde↔écran et **cadrage visible** (`visibleBounds`, base du culling, `LOT-40`).
- `RoomGrid` — partition d'un niveau en **salles** (logique pure, `LOT-32`) : cadre la caméra salle par salle.

## Assets et textures
- `AssetPaths` — résolution pure d'un nom logique vers un chemin, dossier injecté (testable sans disque de production, `LOT-39`).
- `TextureLoader` — `decodeImageFile` (décodage `QImage` → RGBA non prémultiplié), `createTexture`, `loadTextureFromFile` : **point unique** de création de texture Direct3D 11. Jamais d'exception (`EX-NFR-040`).
- `TextureAtlas` — atlas de tuiles 16 px et d'images du personnage. Charge `Assets/atlas.png` depuis le disque, avec **repli sur la génération procédurale** si l'asset est absent ou illisible (`LOT-39`, `EX-REN-041`/`EX-REN-042`). `tile`/`playerFrameRegion` sont de la pure arithmétique de grille, `static` et testées sans GPU.
- `ProceduralAtlas` — génération CPU déterministe de l'atlas de repli (palette de tuiles, masques de silhouette des pentes, silhouette humanoïde du personnage). Aucune dépendance GPU ni Qt, entièrement testé.
- `TextureCache` — registre de textures chargées à la demande par **nom logique**, avec validation à l'entrée, `invalidate`/`invalidateAll` (rechargement à chaud) et mémorisation des échecs (`LOT-40`, `EX-REN-043`). Ne réimplémente ni le décodage ni l'upload : il compose `AssetPaths` et `TextureLoader`.
- `AssetContract` — **contrat de dimensions par famille d'asset** et validation **pure**, intercalée entre décodage et upload ; un asset non conforme est refusé avec un message nommant le fichier, le trouvé et l'attendu (`LOT-40`, `EX-REN-007`).
- `MissingTexture` — damier magenta opaque et déterministe, repli **visible** de toute texture attendue mais absente ; résolu par le point d'appel unique `hmi::resolveOrPlaceholder` (`LOT-40`, `EX-NFR-040`).

## Pont vers l'ECS et l'éditeur
- `RenderLayer` — ordonnancement de calques **unique et explicite** du projet (`EX-REN-014`) et composant de présentation `RenderLayerTag` ; aucun lot ne doit en inventer un concurrent. `TextureHandle` y désigne l'identité **opaque** d'une texture, seule notion dont la composition ait besoin.
- `RenderMode` — bascule **Physique** (couleur plate par type de tuile) / **Texture** (habillage), touche fixe `F8`, défaut `Texture` partout et persistance du choix (`LOT-41`, `EX-REN-046`).
- `TileAppearance` — **point de résolution unique** de l'apparence d'une entité selon le mode, appelé à la composition : basculer ne reconstruit jamais la scène ECS. C'est ici que `LOT-42` insérera « surcharge par case > skin > damier ».
- `ComposedScene` — **composition** du rendu : liste ordonnée des primitives d'une image (tri calque → texture → `Sprite::layer`, stable), culling par cadrage caméra et compteurs de volume. Logique pure, sans Direct3D (`LOT-40`, `EX-NFR-004`, `EX-NFR-005`).
- `QuadRecorder` — capture et **inspection** d'une scène composée pour les tests (ordre des calques, contiguïté des groupes de texture, dénombrement, présence d'une primitive). Outil de vérification, jamais un détour du rendu.
- `SpriteRenderer` — pont ECS → écran : **compose** (`composeWorldSprites`) puis **soumet** (`submitComposedScene`, une passe `begin/end` par groupe de texture) — **lecture seule** de l'ECS (`EX-ARCH-012`), avec interpolation (`PreviousPosition`, `EX-ARCH-031`).
- `PreviousPosition` — composant de **présentation** rangé dans le `core::World` mais écrit et lu par `HMI` seul ; porte la position au pas précédent pour l'interpolation.
- `DraftRenderer` — rendu du **brouillon d'édition** (`core::LevelDraft`) dans le viewport de l'éditeur : tuiles, grille, salles, liens de mécanismes, aperçu des outils, sans passer par l'ECS de jeu.
- `TileVisuals` — correspondance type de tuile → région d'atlas, partagée entre le jeu (`hmi::GameSession`) et l'éditeur (`hmi::DraftRenderer`, `hmi::PalettePanel`). **Point d'entrée unique** de l'apparence des tuiles.
- `GraphicsLog` — macros de journalisation du module.

## À venir
Le programme d'habillage `LOT-42` → `LOT-55` ajoute ici : raccords automatiques de tuiles (`LOT-42`) ; rendu du fond (`LOT-44`) ;
animations pilotées par données (`LOT-46`) ; décors et parallaxe (`LOT-49`) ; police bitmap en scène
(`LOT-52`) ; ombres (`LOT-55`).

Réf. specs : `EX-REN-002`, `EX-REN-010`…`EX-REN-015`, `EX-REN-020`…`EX-REN-022`, `EX-REN-041`…`EX-REN-046`, `EX-REN-004`…`EX-REN-009`, `EX-ARCH-012`, `EX-ARCH-022`, `EX-ARCH-031`, `EX-ARCH-050`, `EX-NFR-004`, `EX-NFR-005`.
