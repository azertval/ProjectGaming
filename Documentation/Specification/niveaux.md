# Niveaux & contenu {#spec-niveaux}

> Statut : **brouillon**. Dépend de [`gameplay.md`](gameplay.md).

## 1. Représentation des niveaux
- \anchor EX-LVL-001 **EX-LVL-001** — Un niveau doit être décrit par un **fichier de données** externe (pas en dur dans le code), placé dans `Source/Elements`.
- \anchor EX-LVL-002 **EX-LVL-002** — Le format doit décrire au minimum : dimensions de la grille, type de chaque tuile, position d'entrée et de sortie, et les mécanismes (interrupteurs, portes, blocs) avec leurs liaisons.
- \anchor EX-LVL-003 **EX-LVL-003** — Le format retenu est un **JSON structuré orienté objets** : un niveau est un objet JSON portant ses **métadonnées** (nom, dimensions) et une **liste de tuiles**, chaque tuile étant un **objet** `{x, y, type, …}` (les cases vides sont omises) pouvant porter des **champs spécifiques** à son type (ex. liaison interrupteur↔porte par identifiant). Choisi pour un moteur **extensible et réutilisable** (données riches par tuile, sérialisation et *round-trip* d'éditeur directs), au prix d'une lisibilité « à l'œil » moindre qu'une grille ASCII — l'édition passe par l'**éditeur**, pas par le texte brut.
- \anchor EX-LVL-005 **EX-LVL-005** — Le fichier de niveau doit porter un **numéro de version de
  format**, afin que l'ajout de nouveaux champs (fond, jeu de skins, texture par case, décors) reste
  traçable et qu'une évolution non rétrocompatible future soit **détectée** plutôt que subie. Un
  fichier **sans** numéro de version est lu comme la version initiale, sans erreur ni avertissement :
  la rétrocompatibilité des niveaux existants est un invariant. Concrétisé en `LOT-44`.
- \anchor EX-LVL-004 **EX-LVL-004** — Le chargement d'un niveau doit **valider** les données (positions des tuiles **dans les bornes** `width × height`, présence d'une entrée et d'une sortie, liaisons de mécanismes valides) et signaler une erreur exploitable en cas de fichier invalide (cf. politique d'erreurs des conventions).
- \anchor EX-LVL-006 **EX-LVL-006** — Un niveau doit porter son **mode de cadrage** de caméra
  (`EX-REN-016`) comme une **donnée**, au même titre que sa géométrie : le cadrage est une décision
  de **conception** — un tableau de puzzle se voit en entier, un tableau d'adresse suit le
  personnage — et non une règle déduite des dimensions. Un fichier **sans** mode déclaré conserve
  **exactement** le comportement historique (niveau entier s'il tient dans une salle, cadrage par
  salle sinon) : la rétrocompatibilité des niveaux existants reste un invariant (`EX-LVL-005`).
  Concrétisé en `LOT-64`.
- \anchor EX-LVL-007 **EX-LVL-007** — En **mode par salle** (`EX-REN-015`), un niveau doit pouvoir
  porter une **liste de zones de caméra** dessinées à la main (rectangles en tuiles) plutôt que de
  subir une grille uniforme unique : la caméra retient la **première** zone de la liste couvrant la
  position du personnage, avec repli sur le **niveau entier** si aucune zone ne le couvre — ce qui
  permet de mélanger plusieurs tailles de caméra dans un même niveau, sans transitions ni
  déclencheurs, la liste étant vide par défaut (comportement de grille automatique inchangé,
  `EX-LVL-006`). Concrétisé en `LOT-64`.

### Format retenu (JSON, liste de tuiles-objets)
Types de tuiles : `entry` (entrée), `exit` (sortie), `solid` (solide), `danger`, `switch`
(interrupteur), `pressurePlate` (plaque de pression, activation continue tant qu'un poids y
repose), `door` (porte), `block` (bloc poussable, `EX-GP-022` — déplaçable par le personnage,
retombe sous gravité), `slopeUpRight` et `slopeUpLeft` (pentes à 45°, `EX-GP-003` — surface suivie,
jamais solide pour la grille classique ; « Up » désigne le côté qui monte, `Right`/`Left`),
`roundedUpRight` et `roundedUpLeft` (variante **courbe** — quart de cercle — des pentes, `EX-GP-004`,
même orientation et même principe de suivi, formule de hauteur différente), `slopeDownRight`,
`slopeDownLeft`, `roundedDownRight` et `roundedDownLeft` (variantes de **plafond** des quatre
tuiles précédentes, `EX-GP-006` — miroir vertical de la même silhouette ; comme leurs équivalents
de sol, jamais solides pour la grille classique, mais une passe de suivi dédiée bloque précisément
un saut qui franchit leur profil incliné/courbe par en dessous, sans jamais y faire « marcher » le
personnage — leur face du haut, toujours plate, supporte normalement un personnage qui tombe
dessus par au-dessus),
`concaveUpRight`, `concaveUpLeft`, `concaveDownRight` et `concaveDownLeft` (variante **concave**
des arrondis, `EX-GP-007` — même principe de suivi/silhouette, sol et plafond, mais courbure
**inversée** : centre du cercle du côté plein plutôt que du côté creux),
`blockHalf` et `blockQuarter` (blocs poussables à taille **réduite** — `×0.5`/`×0.25` —
`EX-GP-005`, mêmes règles de poussée/chute que `block`, boîte de collision centrée et plus
petite), `dangerUp`/`dangerDown`/`dangerLeft`/`dangerRight` (danger **directionnel**, `EX-GP-050` —
mortel uniquement sur une bande étroite du bord désigné par le suffixe, pas la case entière),
`dangerMover` (danger **mobile**, `EX-GP-051` — aller-retour linéaire déterministe autour de sa
position de départ ; champs optionnels `axis`, `"horizontal"` ou `"vertical"`, et `range`, en
cases, défauts respectifs `"horizontal"` et `2`), `dangerSwitched` (danger **commuté**, `EX-GP-052`
— mortel uniquement quand l'interrupteur/la plaque de pression qui lui est lié est actif ; champ
`opensWith` comme `door`, même résolution par identifiant), `dangerBlink` (danger **temporisé**,
`EX-GP-053` — alterne mortel/inoffensif selon une période fixe ; champs optionnels `period`,
`phase` et `activeDuration`, en pas fixes, défauts respectifs `120`, `0` et `60`). Une case
**vide** n'est pas listée (absence = vide).
```json
{
  "name": "Tutoriel 1",
  "width": 12,
  "height": 8,
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 9, "y": 6, "type": "exit" },
    { "x": 5, "y": 5, "type": "danger" },
    { "x": 8, "y": 3, "type": "switch", "id": "s1" },
    { "x": 10, "y": 5, "type": "door", "opensWith": "s1" }
  ]
}
```
Coordonnées `x` = colonne, `y` = ligne, origine **haut-gauche** ; toute tuile hors des bornes
`width × height` est invalide. Les **liaisons** interrupteur↔porte se font par **identifiant**
(un `switch` porte un `id`, une `door` le référence via `opensWith`), schéma extensible à
d'autres mécanismes. L'exemple omet les tuiles `solid` des bords pour rester lisible.

## 2. Progression
- \anchor EX-LVL-010 **EX-LVL-010** — Le jeu doit charger les niveaux dans un **ordre défini** (liste ordonnée).
- \anchor EX-LVL-011 **EX-LVL-011** — À la réussite d'un niveau, le jeu doit charger automatiquement le suivant ; après le dernier, revenir au menu (ou écran de fin).
- \anchor EX-LVL-012 **EX-LVL-012** — Le MVP doit fournir **3 niveaux** de difficulté croissante illustrant : déplacement/saut, danger, puzzle interrupteur↔porte.
- \anchor EX-LVL-013 **EX-LVL-013** — La **séquence** de niveaux jouée doit être une **donnée de
  contenu** (fichier de `Source/Elements/Levels`), jamais un littéral du code : réordonner, ajouter
  ou retirer un tableau ne doit demander aucune recompilation. Même exigence de validation et de
  version de format que les niveaux eux-mêmes (`EX-LVL-004`, `EX-LVL-005`) ; un niveau référencé mais
  absent est une **erreur récupérable** (`EX-NFR-040`). Concrétisé en `LOT-59`.
- \anchor EX-LVL-014 **EX-LVL-014** — La **progression** du joueur (tableau atteint, tableaux
  terminés) doit être **conservée entre deux lancements**, à la granularité du **tableau** et non de
  l'instant. Elle est stockée par **nom** de niveau — de sorte qu'un réordonnancement de la séquence
  (`EX-LVL-013`) ne la rende pas fausse — et se dégrade proprement : fichier absent, vide ou
  corrompu donne une partie neuve, sans erreur bloquante. Concrétisé en `LOT-59`.
- \anchor EX-LVL-015 **EX-LVL-015** — Le contenu livré doit **couvrir toutes les mécaniques** du
  moteur : chaque type de tuile et chaque mode de cadrage (`EX-LVL-006`) doit apparaître dans au
  moins un tableau de la séquence franchi par le test système (`EX-NFR-021`). La vérification est
  **automatique** et **dérivée des énumérations du code**, de sorte qu'ajouter une mécanique sans
  tableau qui l'emploie échoue sans qu'un inventaire ait à être tenu à la main ; les exclusions
  légitimes sont **nommées et justifiées**. Une mécanique absente de tout niveau n'est vérifiée
  qu'en isolation, jamais dans une partie réelle. Prévu en `LOT-65`.

## 3. Conception (lignes directrices)
- Introduire une mécanique à la fois ; le premier niveau sert de tutoriel implicite (sans texte).
- Aucune situation sans issue (le joueur ne doit jamais être bloqué définitivement sans échec possible).
- Chaque niveau doit être **franchissable** — vérifié par un test système sur les niveaux du MVP.

## Traçabilité
Le chargement et la validation relèvent de `Source/Core` ; les fichiers de niveaux et l'atlas sont dans `Source/Elements`. Types de tuiles : [`gameplay.md`](gameplay.md).

Le découpage d'un niveau en **salles** pour la caméra (`EX-REN-015`, `LOT-32`) est un comportement
de **cadrage**, entièrement porté par `Source/HMI` : le format de fichier (`EX-LVL-003`) et la
validation (`EX-LVL-004`, dont l'invariant « une entrée, une sortie ») n'en portent aucune trace —
un niveau à salles reste, du point de vue de `Core`, une grille de tuiles ordinaire.
