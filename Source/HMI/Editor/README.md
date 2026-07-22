# HMI/Editor/

Mode **éditeur intégré** (réutilise le rendu `Graphics` et le modèle/validation de `Core`).

Livré (LOT-14, LOT-15) :

- Édition WYSIWYG des tuiles à la souris depuis une **palette** (`TilePalette`, libellée), placement
  et liaison des mécanismes interrupteur↔porte.
- Sélecteur de niveau nouveau/existant (`LevelPicker`), nommage/renommage du niveau
  (`TextInputField`, `LevelNameValidation`).
- Outils **Pinceau** / **Rectangle** / **Sélection** (copier/coller) via la barre d'outils
  (`ToolBar`, `EditorTool`).
- Panneau latéral fixe (`EditorLayout.h`, constantes de disposition partagées par `TilePalette`,
  `ToolBar` et `EditorScreen`) — la grille se cadre dans le canevas à droite, jamais dessous.

À venir (post-MVP, cf. `editeur-niveaux.md` §4bis) : placement/transform des **décors** (couches) et
pipeline **photo → pixel art** intégré — dépendent d'un lot dédié (`decors.md`, `EX-DEC-*`,
`EX-EDIT-040`/`041`), non commencés.

Réf. specs : `editeur-niveaux.md`.
