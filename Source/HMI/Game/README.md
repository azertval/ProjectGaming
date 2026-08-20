# HMI/Game/

Le **jeu** dans l'application Qt : la simulation d'un niveau et le viewport qui l'affiche.

- `GameSession` — simule et rend **un seul niveau** (déjà validé, en mémoire), sans dépendance
  d'écran. `update()` avance d'un **pas fixe** et renvoie l'issue (`core::LevelOutcome`, rechargement
  interne sur échec) ; `render()` dessine via `hmi::SpriteRenderer` avec interpolation
  (`EX-ARCH-031`). Toute la logique de jeu d'un niveau (scène ECS, physique, mécanismes, blocs,
  dangers, caméra par salle, animation) vit ici, **une seule fois**.
- `GameViewport` — `QWindow` embarqué (`QWidget::createWindowContainer`) fournissant le `HWND` à
  `QRhiWidget`. Pilote la boucle à pas fixe (une image redemande la suivante, discipline d'entrées
  par pas — `LOT-33`) en **mode jeu** (enchaîne une séquence via `GameSession`) comme en **mode
  édition** (peinture/outils sur `core::LevelDraft`, rendu `hmi::DraftRenderer`, grille `F10`).

Réf. : guide [`guide-ihm-qt`](../../../Documentation/Guide/guide-ihm-qt.md),
[`guide-boucle`](../../../Documentation/Guide/guide-boucle.md).
