# Core/Resources/

**Dossier vide, conservé sans objet immédiat.**

Ce dossier était prévu pour un `ResourceManager` unique dans `Core`, gérant textures, niveaux et
décors par handle (`EX-ARCH-080` dans sa formulation d'origine). Ce module **ne sera pas écrit** :
la séparation `Core`/`HMI` telle qu'elle est appliquée aujourd'hui l'interdit, puisqu'un
gestionnaire de textures obligerait `Core` à connaître Direct3D 11 (`EX-NFR-010`, `EX-ARCH-010`).

La gestion des ressources vit désormais du côté qui les possède :

- **Textures** → `HMI/Graphics` : résolution de chemin (`hmi::AssetPaths`), décodage et upload
  (`hmi::TextureLoader`), registre par nom logique et mise en cache (`LOT-40`).
- **Niveaux** → `Core/Levels` : `core::LevelLoader`/`core::LevelWriter`, avec validation
  (`EX-LVL-004`).
- **Décors** → `Core/Levels` également, comme donnée annexe du niveau (`LOT-49`).

Réf. specs : `EX-ARCH-080` (amendée en conséquence).
