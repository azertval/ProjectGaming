# HMI/Graphics/

Rendu Direct3D 11 (wrapper mince, pas de couche multi-backend).

- `GraphicsDevice` — device, swap chain, cible de rendu, effacement + présentation V-Sync, redimensionnement ; expose `device()`/`context()`.
- `SpriteBatch` — pipeline 2D : quads texturés (HLSL compilés à l'exécution), fusion alpha (transparence) et échantillonnage *nearest* (pixel art) ; `begin`/`draw`/`end` avec *batching*.
- À venir (LOT-05) : atlas de textures, caméra 2D, système de rendu des sprites.

Réf. specs : `EX-REN-002`, `EX-REN-010`…`EX-REN-022`, `EX-ARCH-022`, `EX-ARCH-050`.
