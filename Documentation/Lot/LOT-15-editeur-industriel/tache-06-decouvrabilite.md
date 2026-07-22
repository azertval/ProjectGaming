# TACHE-06 — Découvrabilité : barre d'outils, aide, libellés, liaisons lisibles {#lot-15-tache-06-decouvrabilite}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Interface` · **Statut :** à faire

## Contexte
Toutes les commandes de l'éditeur (Ctrl+S, `P`, Ctrl+Z/Y, Maj+clic, flèches, et désormais `F2`,
molette, glisser droit, `1`/`2`/`3`, Ctrl+C/V) sont aujourd'hui invisibles à l'écran — seule la
documentation externe les liste. La palette n'a ni icône ni texte au-delà d'un aplat de couleur, et
plusieurs liaisons interrupteur↔porte simultanées se distinguent mal (une seule teinte cyan
partagée).

## Travail à réaliser
- **`ToolBar`** (nouveau, `Source/HMI/Editor/`) : classe de géométrie pure (même esprit que
  `TilePalette`) exposant les entrées cliquables des outils (`Pinceau`/`Rectangle`/`Selection`,
  TACHE-05) et l'outil sélectionné ; dessinée par `EditorScreen` sous la palette.
- **Aide des raccourcis** : `F1` bascule un aperçu compact (texte via `BitmapFont`, déjà utilisé
  pour le statut/menu) listant toutes les commandes de l'éditeur ; un indice discret et permanent
  (« F1 : aide ») s'affiche en bas d'écran quand l'aperçu est replié.
- **Libellés de palette** : un court texte (« Vide », « Plein », « Danger », « Entrée », « Sortie »,
  « Interrupteur », « Porte ») sous chaque entrée de `TilePalette`, via `BitmapFont`.
- **Liaisons lisibles** : dans `EditorScreen::renderGrid`, remplacer la teinte cyan unique
  (`LINK_TINT`) par une teinte choisie dans un petit ensemble fixe (~6 couleurs), assignée par
  interrupteur selon son ordre d'apparition dans `LevelDraft::mechanisms()` ; chaque porte reprend
  la teinte de son interrupteur.

## Fichiers impactés
- `Source/HMI/Editor/ToolBar.h`/`.cpp` (nouveau).
- `Source/HMI/Editor/TilePalette.h`/`.cpp` (libellés — géométrie uniquement, le texte reste dessiné
  par `EditorScreen` comme le reste du rendu de palette).
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (rendu barre d'outils/aide/libellés, teintes de
  liaison par interrupteur).
- Tests unitaires (`Source/Test/Unit/HMI/Editor/test_tool_bar.cpp`).

## Tests (obligatoires)
- `ToolBar::handleClick` sélectionne l'outil couvert par les coordonnées cliquées, sans effet en
  dehors de sa zone (même contrat que `TilePalette::handleClick`).
- Deux liaisons interrupteur↔porte simultanées reçoivent des teintes **différentes** ; une porte et
  son interrupteur partagent toujours la **même** teinte.
- `F1` bascule l'état d'affichage de l'aide (test au niveau de l'état, pas du rendu pixel).

## Points d'attention
- `ToolBar` et les libellés restent des ajouts purement additifs au rendu existant : aucune
  modification de la sémantique de peinture/liaison/redimensionnement déjà livrée.
- Rester cohérent avec le vocabulaire visuel déjà établi (teintes translucides sur quads, comme la
  surbrillance de survol et la liaison en attente, LOT-14) plutôt que d'introduire un nouveau
  langage graphique.
- Le cycle de couleurs de liaison doit rester lisible même si le nombre d'interrupteurs dépasse la
  taille de l'ensemble fixe (réutilisation cyclique documentée, pas un blocage).

## Définition de fait (DoD)
- Barre d'outils, aide, libellés et liaisons multicolores opérationnels et testés (`ctest` vert) ;
  vérifiés visuellement ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-015`, `EX-EDIT-016`.
