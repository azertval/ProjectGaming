# HMI/Editor/

Périmètre **éditeur de niveau** de l'application Qt : les **panneaux** dockables et la **logique pure**
(testable hors Qt/GPU) qui les alimente. Le canevas d'édition lui-même est le viewport partagé
`hmi::GameViewport` (dossier `Game/`, mode édition : peinture, outils, grille `F10`) ; il opère sur
`core::LevelDraft` (modèle mutable/sérialisable de `Core`) et rend via `hmi::DraftRenderer`
(dossier `Graphics/`).

Panneaux Qt :

- **Palette** (`PalettePanel`) — `QTreeView` catégories → sous-groupes → tuiles, alimenté par la
  taxonomie pure `tileTaxonomy` (`TileTaxonomy.{h,cpp}`, tous les `core::TileType` couverts).
- **Outils** (`ToolPanel`) — Pinceau / Rectangle / Sélection (`EditorTool`), disposition décrite
  hors code dans `Elements/UI/ToolPanel.ui`.
- **Niveaux** (`LevelBrowserPanel`) — liste/recherche du dossier `Levels`, création / renommage /
  duplication / suppression, déléguant aux opérations fichiers pures.

Logique pure (aucune dépendance Qt/GPU, couverte par `Source/Test/Unit`) :

- `tileTaxonomy` — arbre catégories/tuiles de la palette.
- `LevelFileOperations` — créer / renommer / dupliquer / supprimer un fichier de niveau.
- `LevelNameValidation` — validation d'un nom de niveau saisi.
- `EditorTool` — énumération de l'outil actif.

Réf. specs : [`editeur-niveaux.md`](../../../Documentation/Specification/editeur-niveaux.md),
[`interface-ihm.md`](../../../Documentation/Specification/interface-ihm.md) (`EX-EDIT-*`, `EX-IHM-*`).
