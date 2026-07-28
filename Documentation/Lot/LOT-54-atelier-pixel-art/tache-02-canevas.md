# TACHE-02 — Canevas pixel art {#lot-54-tache-02-canevas}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** non commencé

## Contexte
Le cœur de l'atelier. Le périmètre est délibérément **modeste** — peindre, effacer, remplir,
prélever une couleur, zoomer, annuler — parce que c'est ce que demande le dessin de tuiles 16×16, et
parce qu'aucune bibliothèque permissive ne couvre ce besoin (les éditeurs pixel art matures sont de
lignée GPL, incompatible avec la licence du projet).

Tout repose sur des primitives déjà présentes : Qt Widgets depuis le `LOT-34`, `QImage` et
`QPainter`.

## Travail à réaliser
- **Canevas** (`QWidget`) affichant une `QImage` en cours d'édition, à un zoom entier, avec une
  grille de pixels visible au-delà d'un certain facteur.
- **Outils** : pinceau (un pixel), gomme (met à transparent), **pot de peinture** (remplissage par
  zone contiguë de même couleur), **pipette** (prélève la couleur sous le curseur).
- **Palette** de couleurs, avec sélection de la couleur courante et gestion de la transparence.
- **Zoom** et déplacement de la vue, sur le modèle du canevas de niveau (`EX-EDIT-013`).
- **Annuler/refaire local** : propre au canevas, **totalement indépendant** de l'historique de
  `core::LevelDraft`. Annuler un coup de pinceau ne doit jamais annuler une action d'édition de
  niveau, et réciproquement.
- **Logique pure séparée** : les opérations sur les pixels (remplissage par zone contiguë, tracé de
  ligne entre deux positions lors d'un glisser rapide) sont des fonctions sur un tampon, testables
  sans Qt.

## Fichiers impactés
- `Source/HMI/Editor/PixelCanvas.{h,cpp}` (nouveau) — widget.
- `Source/HMI/Editor/PixelOperations.{h,cpp}` (nouveau) — opérations pures sur un tampon.
- `Source/Test/Unit/HMI/Editor/test_pixel_operations.cpp` (nouveau).

## Tests (obligatoires)
- **Remplissage par zone contiguë** : zone fermée, zone ouverte débouchant sur un bord, remplissage
  sur la couleur déjà présente (aucun changement, et surtout pas de récursion infinie), image d'un
  seul pixel.
- **Tracé entre deux positions** : un glisser rapide ne doit pas laisser de trous — la ligne entre
  deux positions successives est remplie.
- **Annuler/refaire** : suite d'opérations, annulation partielle, nouvelle opération après annulation
  (la pile de rétablissement est vidée).
- Opérations **pures**, testées sans Qt ni GPU.

## Points d'attention
- **Le tracé entre positions n'est pas optionnel.** Sans lui, un glisser rapide produit des pixels
  isolés — c'est le défaut le plus visible d'un canevas pixel art fait naïvement.
- **Le remplissage doit être itératif**, pas récursif : une récursion sur une grande zone
  déborderait la pile.
- Deux historiques coexistent dans l'application (canevas et niveau). Vérifier explicitement que
  `Ctrl+Z` s'applique à celui qui a le focus, et jamais à l'autre.
- Le zoom doit rester **entier** pour que les pixels restent carrés et nets.

## Définition de fait (DoD)
- Le canevas permet de peindre, effacer, remplir, prélever une couleur, zoomer et annuler ; le
  glisser rapide ne laisse pas de trous ; l'historique est local et indépendant de celui du niveau ;
  les opérations sur pixels sont pures et testées ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-EDIT-013` (déplacement et zoom),
`EX-EDIT-030` (éditeur intégré), `EX-ARCH-022` (pixel art net), `EX-REN-033` (traduction).
