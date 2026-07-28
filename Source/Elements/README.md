# Source/Elements/

**Assets et éléments statiques** du jeu.

## Périmètre
- `Levels/` — définitions de niveaux (`.json`), copiées à côté de l'exécutable au build.
- `Localization/` — catalogues de traduction (`.lang`), copiés à côté de l'exécutable au build.
- `Assets/` — atlas de tuiles (`.png`), copié à côté de l'exécutable au build (`LOT-39`,
  `EX-REN-042`), chargé par `hmi::TextureAtlas` avec repli procédural si absent/illisible.
- `UI/` — assets Qt déclaratifs de l'IHM : mises en page Qt Designer (`.ui`) et ressource (`.qrc`).
- `Themes/` — feuilles de style Qt (`.qss`) de l'IHM.
- `Fonts/`, `Audio/` — réservés (polices, sons) ; procéduraux au MVP.

Ces éléments sont consommés par `../Core/` (données de niveau) et `../HMI/` (rendu, UI Qt).

> Note : les assets binaires volumineux peuvent être gérés à part (Git LFS) selon leur taille.
