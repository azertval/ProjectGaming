# TACHE-03 — Parallaxe et arbitrage avec la caméra par salle {#lot-49-tache-03-parallaxe}

**Lot :** [LOT-49](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** non commencé

## Contexte
Un facteur de défilement par couche (`EX-DEC-006`) donne la profondeur : l'arrière-plan défile moins
vite que le niveau, le premier plan plus vite. C'est un multiplicateur appliqué au décalage de
caméra — presque gratuit, une fois les décors en place.

Presque, parce qu'il y a un conflit à résoudre : la caméra de ce jeu ne défile **pas**. Elle cadre
une salle et bascule **nettement** sur la suivante quand le personnage franchit la frontière
(`EX-REN-015`, `LOT-32`, parti pris assumé de style *Celeste*). Une parallaxe conçue pour un
défilement continu n'a, telle quelle, aucun sens dans ce cadre.

C'est le point que le cadrage du lot demande de **trancher et documenter**, pas de découvrir à
l'exécution.

## Travail à réaliser
- **Arbitrer entre deux comportements**, et implémenter celui retenu :
  - **décalage relatif au centre de la salle courante** — le décor se replace à chaque salle ; la
    parallaxe n'agit qu'à l'intérieur d'une salle, cohérente avec la coupure nette ;
  - **décalage absolu en espace niveau** — continu sur tout le niveau, mais visiblement discontinu
    au moment de la bascule de salle.
  La première option est cohérente avec le parti pris de cadrage existant ; la seconde le contredit.
  Quel que soit le choix, il doit être **écrit** dans la spécification des décors et dans le guide
  de rendu.
- **Calcul du décalage** : fonction **pure** prenant la position du décor, le facteur de sa couche et
  le rectangle cadré par la caméra, renvoyant la position de rendu. Aucun état.
- **Culling** : le rectangle de test de visibilité doit être calculé **après** application du
  décalage (piège signalé en LOT-40, TACHE-05) — une couche parallaxée n'occupe pas le même
  rectangle monde que le niveau.
- **Facteur par couche** : constantes nommées, pas de valeurs magiques. Le facteur de la couche
  `Decor` vaut 1 (elle est solidaire du niveau) ; c'est la valeur de référence.

## Fichiers impactés
- `Source/HMI/Graphics/Parallax.{h,cpp}` (nouveau).
- `Source/HMI/Graphics/DecorVisuals.{h,cpp}`, `SpriteRenderer.{h,cpp}`.
- `Documentation/Specification/decors.md`, `Documentation/Guide/guide-rendu.md`.
- `Source/Test/Unit/HMI/Graphics/test_parallax.cpp` (nouveau).

## Tests (obligatoires)
- Facteur 1 → position inchangée (la couche `Decor` est un cas de référence à asserter).
- Facteur inférieur et supérieur à 1 → décalages de sens opposés, proportionnels.
- **Comportement à la frontière de salle** conforme au choix documenté — c'est le test qui verrouille
  l'arbitrage.
- Culling : un décor parallaxé visible n'est pas écarté ; un décor parallaxé hors cadre l'est.
- Fonction pure, sans GPU.

## Points d'attention
- **La parallaxe est purement visuelle** (`EX-ARCH-012`) : la position simulée du décor ne change
  pas, seule sa position de rendu. Une future manipulation en jeu (`EX-DEC-020`, hors programme)
  devrait donc convertir entre les deux — le noter.
- Le zoom de la caméra est **entier** quand c'est possible (`EX-ARCH-022`) pour la netteté du pixel
  art. Un décalage de parallaxe fractionnaire produirait un décor flou ou tremblant : arrondir le
  décalage à un pixel écran entier.
- Ne pas appliquer la parallaxe au fond de niveau (LOT-44) : c'est une image unique étirée sur le
  niveau, pas une couche de décor.

## Définition de fait (DoD)
- Les couches défilent à des vitesses différenciées ; le comportement au changement de salle est
  celui documenté et testé ; le culling tient compte du décalage ; le rendu reste net ;
  `/W4 /WX` propre.

## Exigences
`EX-DEC-006` (facteur de défilement par couche) ; réutilise `EX-DEC-002` (couches), `EX-REN-015`
(caméra par salle), `EX-ARCH-022` (pixel art net), `EX-ARCH-012` (rendu sans effet sur la
simulation), `EX-NFR-005` (culling).
