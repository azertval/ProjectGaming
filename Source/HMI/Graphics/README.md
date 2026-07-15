# HMI/Graphics/

Rendu Direct3D 11 (wrapper mince, pas de couche multi-backend).

- `GraphicsDevice` — device, swap chain, cible de rendu, effacement + présentation V-Sync, redimensionnement ; expose `device()`/`context()`.
- `SpriteBatch` — pipeline 2D : quads texturés (HLSL compilés à l'exécution), fusion alpha (transparence) et échantillonnage *nearest* (pixel art) ; `begin`/`draw`/`end` avec *batching*.
- `TextureAtlas` — atlas de textures généré en code (tuiles 16px, dont une transparente) ; expose la vue de texture et les régions.
- `Camera2D` — projection monde → écran (16 px/unité, Y-bas, zoom) ; matrice de projection et conversions monde↔écran.
- À venir (LOT-05) : système de rendu des sprites.

Réf. specs : `EX-REN-002`, `EX-REN-010`…`EX-REN-022`, `EX-ARCH-022`, `EX-ARCH-050`.
