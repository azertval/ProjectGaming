# TACHE-03 — Éditeur et rendu {#lot-24-tache-03-editeur-rendu}

**Lot :** [LOT-24](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics` · **Statut :** à faire

## Contexte
Rendre les blocs réduits utilisables : plaçables dans l'éditeur, visuellement plus petits qu'un
bloc plein (`LOT-21`) pour que leur taille de collision réduite (TACHE-02) soit lisible à l'écran.

## Travail à réaliser
- **`HMI/Editor/EditorLayout.h`** : `PALETTE_TYPE_COUNT` augmente de 2.
- **`HMI/Editor/TilePalette.cpp`** : `PALETTE_TYPES` gagne `BlockHalf`/`BlockQuarter` ; libellés
  courts (ex. `"Bloc 1/2"`/`"Bloc 1/4"`).
- **`HMI/Graphics/TileVisuals.cpp`** : couleurs d'atlas dédiées, ou variante teintée du bloc plein
  (`Block`) pour signaler visuellement la famille sans multiplier les couleurs disponibles.
- **Rendu à l'échelle** : le sprite du bloc doit visuellement occuper `facteur × 1` unité, **centré**
  dans sa case (cohérent avec la boîte de collision de TACHE-02) — sur le modèle de
  `refreshBlockVisuals` (`GameScreen.cpp`, `LOT-21`), qui devra lire le facteur de taille en plus
  de la position pour ajuster `Transform.scale` et recentrer `Transform.position`.

## Fichiers impactés
- `Source/HMI/Editor/EditorLayout.h`, `TilePalette.h`/`.cpp`.
- `Source/HMI/Graphics/TileVisuals.cpp`.
- `Source/HMI/Interface/GameScreen.cpp` (`refreshBlockVisuals`, mise à l'échelle et recentrage).

## Tests (obligatoires)
- `ctest` existant (`test_tile_palette.cpp`) reste vert.
- **Vérification visuelle obligatoire** : peindre un bloc `×0.5` et un `×0.25`, tester (`P`) — la
  taille affichée doit correspondre exactement à la boîte de collision (approcher le bord visuel
  du bloc doit correspondre au moment où le personnage est arrêté).

## Points d'attention
- **Cohérence visuel/collision stricte** : un décalage entre le sprite affiché et la boîte
  réellement testée (TACHE-02) serait perçu comme un bug de collision par le joueur, même si le
  code est correct — vérifier au pixel près en jeu, pas seulement par le calcul.

## Définition de fait (DoD)
- Blocs réduits plaçables, visuellement centrés et à l'échelle correcte ; aucune régression sur les
  autres entrées de la palette.

## Exigences
Aucune exigence propre — intégration de `EX-GP-005` (TACHE-01/TACHE-02) dans `HMI`.
