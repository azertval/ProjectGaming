# TACHE-03 — Rendu de la sélection, poignées et aimantation {#lot-50-tache-03-selection-poignees}

**Lot :** [LOT-50](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/HMI/Editor` · **Statut :** fait

## Contexte
Le geste de TACHE-02 est aveugle sans retour visuel : l'auteur doit voir quel décor est sélectionné,
où sont les poignées, et si l'aimantation est active.

L'éditeur dispose déjà des primitives nécessaires : `hmi::DraftRenderer` dessine une grille, des
voiles de surbrillance et des flèches de liaison, avec un segment orienté (`hmi::LineQuad`, `LOT-37`)
et l'astuce consistant à teinter une région d'atlas opaque pour obtenir un rectangle uni. Rien de
nouveau n'est requis côté pipeline.

## Travail à réaliser
- **Cadre de sélection** autour du décor sélectionné, et **poignées** de redimensionnement aux coins
  plus une poignée de rotation, dessinés sur le calque *EditorOverlay* de *RenderLayer* (LOT-40).
- **Taille écran constante** : les poignées doivent conserver la même taille apparente quel que soit
  le zoom, sinon elles deviennent inutilisables aux extrêmes. Leur taille est donc calculée en
  pixels écran puis convertie en unités monde, pas l'inverse.
- **Aperçu pendant le geste** : pendant un déplacement ou un redimensionnement, afficher le décor à
  sa position en cours, avant validation.
- **Indication d'aimantation** : quand elle est active, matérialiser l'alignement (par exemple en
  accentuant la grille) — sinon l'auteur ne comprend pas pourquoi sa position « saute ».
- **Jamais en jeu** : ces aides n'apparaissent ni en mode jeu, ni en mode essai. C'est le rôle du
  calque *EditorOverlay*, qui n'est composé qu'en édition.

## Fichiers impactés
- `Source/HMI/Graphics/DraftRenderer.{h,cpp}`.
- `Source/HMI/Editor/DecorGesture.{h,cpp}` (géométrie des poignées, partie pure).
- `Source/Test/Unit/HMI/Editor/test_decor_handles.cpp` (nouveau).

## Tests (obligatoires)
- **Géométrie des poignées** : pour un décor donné et un zoom donné, les rectangles de poignées sont
  aux bonnes positions et de taille écran constante — fonction pure, plusieurs zooms testés.
- Cohérence entre la poignée **dessinée** et la poignée **détectée** par le geste (TACHE-02) : ce
  sont les mêmes rectangles, calculés une seule fois.
- Aucun quad d'aide d'édition en mode jeu ou essai — asserté via le *QuadRecorder*.

## Points d'attention
- **Une seule source pour la géométrie des poignées.** Si le rendu et la détection calculent
  chacun leur rectangle, ils divergeront au premier ajustement de taille, et les poignées ne
  répondront plus là où elles s'affichent.
- Les aides doivent rester lisibles **sur tous les fonds** : un cadre clair sur un décor clair
  disparaît. Prévoir un contraste suffisant (contour à double couleur, ou pointillés).
- Ne pas dessiner de poignées pour un décor situé sur une couche masquée (LOT-51).

## Définition de fait (DoD)
- Le décor sélectionné est encadré, ses poignées sont visibles et de taille constante à l'écran,
  l'aperçu suit le geste, l'aimantation est signalée, et rien n'apparaît en jeu ; la géométrie est
  partagée avec la détection et testée ; `/W4 /WX` propre.

## Exigences
`EX-DEC-010` (manipulation de décors), `EX-EDIT-040` (édition de décors) ; réutilise `EX-REN-043`
(calques), `EX-EDIT-030` (éditeur intégré), `EX-ARCH-022` (netteté), `EX-NFR-004` (vérification sans
GPU).
