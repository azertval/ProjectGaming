# HMI/Graphics/

Rendu Direct3D 11 (wrapper mince, pas de couche multi-backend).

- `GraphicsDevice` — device, swap chain, cible de rendu, effacement + présentation V-Sync, redimensionnement ; expose `device()`/`context()`.
- `SpriteBatch` — pipeline 2D : quads texturés (HLSL compilés à l'exécution), fusion alpha (transparence) et échantillonnage *nearest* (pixel art) ; `begin`/`draw`/`end` avec *batching*.
- `TextureAtlas` — atlas de textures généré en code (tuiles 16px, dont une transparente, plus une grille d'images 16×16 pour la silhouette animée du personnage — repos, course, saut) ; expose la vue de texture et les régions.
- `Camera2D` — projection monde → écran (16 px/unité, Y-bas, zoom) ; matrice de projection et conversions monde↔écran.
- `SpriteRenderer` — pont ECS → écran : lit `view<Transform, Sprite>`, trie par couche, dessine via `SpriteBatch` (lecture seule de l'ECS).
- `BitmapFont` — police bitmap monospace générée en code (glyphes 5×7, accents français composés) ; `drawText` dessine du texte UTF-8 en espace écran via `SpriteBatch`, et `screenProjection` fournit la projection pixels → clip de l'interface.
- `FlagIcons` — icônes de drapeaux générées en code (France, Royaume-Uni) pour le sélecteur de langue ; expose la région de texture par langue.
- `SaveIcon` — icône « enregistrer » (flèche de téléchargement) générée en code pour le bouton d'enregistrement des logs.

Réf. specs : `EX-REN-002`, `EX-REN-010`…`EX-REN-022`, `EX-REN-032`, `EX-REN-033`, `EX-ARCH-022`, `EX-ARCH-050`.
