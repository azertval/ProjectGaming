# HMI/Editor/

Périmètre **éditeur de niveau** de l'application Qt : les **panneaux** dockables et la **logique pure**
(testable hors Qt/GPU) qui les alimente. Le canevas d'édition lui-même est le viewport partagé
`hmi::GameViewport` (dossier `Game/`, mode édition : peinture, outils, grille `F10`) ; il opère sur
`core::LevelDraft` (modèle mutable/sérialisable de `Core`) et rend via `hmi::DraftRenderer`
(dossier `Graphics/`).

Panneaux Qt :

- **Palette** (`PalettePanel`) — `QTreeView` catégories → sous-groupes → tuiles, alimenté par la
  taxonomie pure `tileTaxonomy` (`TileTaxonomy.{h,cpp}`, tous les `core::TileType` couverts).
- **Outils** (`ToolPanel`) — Pinceau / Rectangle / Sélection / Lien (`EditorTool`), disposition
  décrite hors code dans `Elements/UI/ToolPanel.ui`.
- **Niveaux** (`LevelBrowserPanel`) — liste/recherche du dossier `Levels`, création / renommage /
  duplication / suppression, déléguant aux opérations fichiers pures.
- **Liens** (`LinkPanel`) — liaisons déclencheur → cible (interrupteur/plaque → porte, danger
  commuté) du niveau courant, en regard des flèches dessinées dans le viewport (`LOT-37`).

Logique pure (aucune dépendance Qt/GPU, couverte par `Source/Test/Unit`) :

- `tileTaxonomy` — arbre catégories/tuiles de la palette.
- `LevelFileOperations` — créer / renommer / dupliquer / supprimer un fichier de niveau.
- `LevelNameValidation` — validation d'un nom de niveau saisi.
- `EditorTool` — énumération de l'outil actif.
- `LinkGesture` — machine à états du geste de liaison (`resolveLinkClick`), indépendante de Qt.
- `LinkGeometry` — géométrie des flèches de liaison (segment, pointe, écartement anti-superposition).

## À venir

Le programme d'habillage `LOT-40` → `LOT-55` ajoute ici : un panneau **« Textures »** unique
(skins, fond, objets, animations, décors — `LOT-42` et suivants), une **bibliothèque d'assets** à
vignettes avec import et rechargement à chaud (`LOT-43`), un outil d'**assignation de texture** par
case (`LOT-45`), un outil de **placement de décors** (`LOT-50`), le contrôle de **visibilité par
calque** (`LOT-51`) et l'**atelier pixel art** (`LOT-54`).

Réf. specs : [`editeur-niveaux.md`](../../../Documentation/Specification/editeur-niveaux.md),
[`interface-ihm.md`](../../../Documentation/Specification/interface-ihm.md) (`EX-EDIT-*`, `EX-IHM-*`).
