# TACHE-04 — Aperçu live et mode planche à raccords {#lot-54-tache-04-apercu-live-planche}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/Graphics` · **Statut :** non commencé

## Contexte
Sans cette tâche, l'atelier n'apporterait presque rien : un éditeur d'images externe fait déjà mieux
que ce que LOT-54 peut livrer en un lot. Ce qui le justifie, c'est d'être **intégré** — voir
immédiatement, dans le niveau, ce que l'on est en train de peindre.

S'y ajoute un besoin créé par LOT-42 : dessiner une **planche à raccords** de seize cases, où chaque
case correspond à une configuration de voisinage précise, est impossible sans repères. Un auteur
livré à lui-même se tromperait de case une fois sur deux.

## Travail à réaliser
- **Aperçu live** : à chaque modification significative (fin de coup de pinceau, remplissage), mettre
  à jour le rendu du niveau, via l'invalidation ciblée du *TextureCache* (`invalidate(name)`, API
  prévue depuis LOT-40, exploitée par LOT-43).
  - Ne pas invalider à chaque pixel : regrouper par geste, sinon le rechargement dominerait le coût.
  - L'aperçu doit fonctionner que l'asset soit utilisé comme skin, comme surcharge, ou comme décor.
- **Mode planche à raccords** : quand l'asset édité est utilisé en mode `bitmask16` (LOT-42),
  afficher sur le canevas les **repères des seize cases** et, pour chacune, l'indication de la
  configuration de voisinage qu'elle représente (bord haut, coin concave bas-droite, intérieur,
  tuile isolée…).
- **Aperçu de raccord** : optionnellement, une petite vue montrant un assemblage type (un bloc 3×3
  de tuiles) construit avec la planche en cours — c'est ce qui permet de voir si les bords se
  raccordent réellement.
- **Aperçu d'animation** : si l'asset porte une description d'animation (LOT-46), jouer le clip dans
  le canevas.

## Fichiers impactés
- `Source/HMI/Editor/PixelCanvas.{h,cpp}`.
- `Source/HMI/Graphics/TextureCache.{h,cpp}` (invalidation ciblée, déjà disponible).
- `Source/HMI/Graphics/TileAutotile.{h,cpp}` (libellés des seize configurations, réutilisés).
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- **Libellés des seize cases** : chaque index de la table de raccords a un libellé, et il correspond
  bien à la configuration de voisinage de la table (LOT-42, TACHE-02). Fonction pure.
- Regroupement des invalidations : un geste continu ne produit qu'une invalidation, pas une par
  pixel.
- L'aperçu de raccord assemble les cases selon la même table que le rendu — pas une seconde table.

## Points d'attention
- **Une seule table de raccords.** Les libellés et l'aperçu doivent lire la table de LOT-42 ; une
  copie dans l'atelier divergerait au premier ajustement et induirait l'auteur en erreur — le pire
  résultat possible pour un outil censé le guider.
- L'aperçu live ne doit pas rendre l'application saccadée : invalider et recharger une texture 16×16
  est peu coûteux, mais le faire à chaque mouvement de souris ne l'est pas.
- Si l'asset édité n'est utilisé par aucun niveau ouvert, l'aperçu live n'a rien à montrer : le
  signaler plutôt que de laisser croire à un dysfonctionnement.

## Définition de fait (DoD)
- Le niveau reflète les modifications en cours d'édition, par geste et non par pixel ; l'édition
  d'une planche affiche les repères et libellés des seize cases, issus de la table unique ; un asset
  animé se prévisualise ; chaînes traduites ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art, avec aperçu dans le niveau) ; réutilise `EX-EDIT-025`
(raccords automatiques), `EX-EDIT-026` (rechargement à chaud), `EX-REN-005` (animations par
données), `EX-REN-033` (traduction).
