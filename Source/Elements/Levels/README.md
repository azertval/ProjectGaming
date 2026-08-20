# Elements/Levels/

Niveaux du jeu, un fichier **JSON** par niveau (`EX-LVL-001`, `EX-LVL-003`).

- Un niveau est un objet JSON : `name`, `width`, `height`, des budgets **optionnels**
  `jumpBudget`/`dashBudget` (`EX-GP-024`, absents = illimité), et une liste **`tiles`** d'objets
  `{ "x", "y", "type", … }`. Les cases **vides** ne sont pas listées (absence = vide). Une tuile
  peut porter un champ `"texture"` (nom d'asset, `EX-EDIT-043`) : texture assignée **par instance**,
  prioritaire sur le skin de son type, indépendante du type lui-même.
- Champs racine optionnels d'habillage (`LOT-65`, aucun n'affecte la géométrie/collision) :
  `"background"` (nom d'asset de `Assets/Backgrounds/`, `EX-REN-044`), `"skinSet"` (nom d'un jeu de
  `skins.json`, `EX-EDIT-024` — absent = jeu par défaut), `"decors"` (liste d'objets `{ "asset",
  "x", "y", "layer" }`, `EX-DEC-001`/`EX-DEC-002`, position en unités monde **flottantes**, jamais
  calée sur la grille), et `"cameraFraming"` (`{ "mode", "roomWidthTiles"?, "roomHeightTiles"?,
  "zones"? }`, `EX-LVL-006`, `LOT-64` — `mode` vaut `"wholeLevel"`/`"perRoom"`/`"follow"`, absent =
  règle de repli par dimensions, cf. `core::resolveCameraFraming`).
- Types de tuiles : `entry`, `exit`, `solid`, `danger`, `switch`, `pressurePlate`, `door`,
  `block`/`blockHalf`/`blockQuarter` (blocs poussables, tailles pleine/`×0.5`/`×0.25`,
  `EX-GP-022`/`EX-GP-005`), `slopeUpRight`/`slopeUpLeft` (pentes à 45°, `EX-GP-003`),
  `roundedUpRight`/`roundedUpLeft` (variante en quart de cercle, `EX-GP-004`),
  `slopeDownRight`/`slopeDownLeft`/`roundedDownRight`/`roundedDownLeft` (variantes de **plafond**
  des quatre précédentes — miroir vertical, jamais solides non plus, mais bloquent précisément un
  saut qui franchit leur profil incliné/courbe par en dessous ; leur face du haut, toujours plate,
  supporte un personnage qui tombe dessus par au-dessus, `EX-GP-006`),
  `concaveUpRight`/`concaveUpLeft`/`concaveDownRight`/`concaveDownLeft` (variante **concave** des
  arrondis, sol et plafond — même suivi/silhouette, courbure inversée : centre du cercle du côté
  plein plutôt que du côté creux, `EX-GP-007`),
  `dangerUp`/`dangerDown`/`dangerLeft`/`dangerRight` (danger **directionnel**, mortel sur une bande
  étroite du bord désigné, `EX-GP-050`, `LOT-31`), `dangerMover` (danger **mobile**, aller-retour
  déterministe autour de sa position de départ, champs optionnels `axis`/`range`, `EX-GP-051`),
  `dangerSwitched` (danger **commuté**, mortel quand son déclencheur lié est actif, champ
  `opensWith` comme `door`, `EX-GP-052`), `dangerBlink` (danger **temporisé**, alterne mortel/
  inoffensif selon une période fixe, champs optionnels `period`/`phase`/`activeDuration`,
  `EX-GP-053`), `movingPlatform` (plateforme **mobile**, `EX-GP-026`).
- **Route d'une plateforme mobile** (`EX-GP-054`, `LOT-67`) : la position de la tuile est le point
  de départ, `waypoints` liste les points **suivants** (`[{ "x": …, "y": … }, …]`), et `mode` vaut
  `"pingpong"` (défaut, omis) ou `"loop"` (circuit fermé, le dernier point rejoignant le premier).
  `speed` (cases/seconde) et `phase` (décalage initial, en pas fixes) restent optionnels. Le couple
  `endX`/`endY` d'avant le multi-points reste **lu** — un fichier qui l'emploie se charge et se joue
  à l'identique (`EX-LVL-008`) — mais l'éditeur ne le réécrit plus. `demo-plateforme.json` illustre
  volontairement les deux écritures.
- **Règles du tableau**, à la racine du fichier, tous optionnels : `jumpBudget`/`dashBudget`
  (`EX-GP-024`) plafonnent le nombre total de sauts/dashs **consommables sur tout le tableau**
  (`-1` = illimité, jamais rechargé) ; `airJumps`/`dashCharges` (`EX-GP-055`) redéfinissent les
  **capacités rechargées à chaque atterrissage**. Deux notions distinctes : un budget épuisé le
  reste jusqu'au rechargement du niveau, une capacité se recharge dès qu'on touche le sol.
- Coordonnées `x` = colonne, `y` = ligne, origine **haut-gauche** ; toute tuile doit rester dans
  les bornes `width × height`.
- **Mécanismes** : un `switch` (bascule au contact) ou une `pressurePlate` (activation continue,
  `EX-GP-025`) porte un `id`, une `door` le référence via `opensWith` (liaison déclencheur↔porte).
  Schéma extensible à d'autres champs par tuile.
- Contraintes de validité : exactement une `entry` et une `exit`, pas de deux tuiles à la même
  position, toute `door.opensWith` doit référencer un `switch`/`pressurePlate` existant.

Chargés à l'exécution par `core::LevelLoader` (copiés à côté de l'exécutable par CMake).

## Séquence démo (`LOT-25`, en donnée de contenu depuis `LOT-59` TACHE-04, refondue en `LOT-65`)

La séquence forme le contenu joué par le jeu (`ScreenId::Game`) : un niveau par mécanique (ou petit
groupe de mécaniques apparentées), ordre de difficulté croissante, terminée par des tableaux de
synthèse qui les combinent. L'**ordre joué** est celui de `sequence-demo.json`, à côté des niveaux
dans ce même dossier — pas un littéral dans `Source/HMI` (`EX-LVL-013` : le contenu ne s'écrit
jamais dans le code). `Source/Test/Systeme/test_parcours_complet.cpp` rejoue exactement la même
liste, dans le même ordre — un script CI, `scripts/check_demo_sequence.py`, échoue si les deux
divergent.

**Quatre garde-fous** protègent le contenu, et ils mesurent l'**usage** plutôt que la simple
présence — un contrôle par présence était vert alors que la moitié des tableaux ne demandaient rien
au joueur :

1. **Couverture et profondeur** (`test_couverture_mecaniques.cpp`, `EX-LVL-015`) : chaque type de
   tuile est posé au moins **trois** fois dans la séquence, chaque mode de cadrage apparaît, et
   chaque variante significative est employée — danger temporisé déphasé, danger mobile vertical,
   budget de sauts **et** budget de dashs comptés séparément, texture par instance, décor de premier
   plan, zones de caméra dessinées, taille de salle choisie par le niveau.
2. **Anti-couloir** (`test_parcours_complet.cpp`) : aucun tableau ne se franchit en maintenant
   « droite », hors exclusion nommée et justifiée (`demo-deplacement`, dont c'est le sujet).
3. **Proximité au trajet** (même fichier) : chaque tuile de mécanique passe à portée d'un saut du
   chemin réellement parcouru — une mécanique hors d'atteinte est « couverte » sans être jouée.
4. **Franchissabilité** (même fichier, `EX-NFR-021`) : chaque tableau est franchi par un scénario
   d'entrées déterministe qui **emploie** sa mécanique.

Voir `Documentation/Guide/guide-niveaux.md` et la doctrine de conception dans
`Documentation/Specification/niveaux.md`, Sec. 3.

### Inventaire des vingt-deux tableaux

Le `LOT-65` a refondu cette séquence en **deux temps**. Le premier (`TACHE-00` à `TACHE-04`) a
remplacé les dix-sept bancs d'essai nus hérités du `LOT-25` par des tableaux habillés couvrant
chaque type de tuile. Le second (`TACHE-05` à `TACHE-09`) les a redessinés pour que chaque mécanique
soit **exigée** et non décorative : une revue avait établi que onze tableaux sur vingt-deux ne
demandaient rien au joueur, que chaque mécanique n'existait qu'en un exemplaire, et que treize
tuiles étaient hors d'atteinte du personnage.

Quatre garde-fous automatiques tiennent désormais ces promesses — voir la doctrine de profondeur
dans `Documentation/Specification/niveaux.md`, Sec. 3.

| # | Tableau | Mécanique(s) exigée(s) | Cadrage | Budgets |
|---|---|---|---|---|
| 1 | `demo-deplacement.json` | Marche, chute, atterrissage ; aucune mort possible | Niveau entier | — |
| 2 | `demo-saut.json` | Saut (`EX-GP-014`) ; le `danger` de base introduit en fond de fosse | Niveau entier | — |
| 3 | `demo-double-saut.json` | Double saut (`EX-GP-015`), paliers hors de portée d'un saut simple | Niveau entier | 0 dash |
| 4 | `demo-wall-jump.json` | Wall slide/wall jump (`EX-GP-016`), deux puits | Niveau entier | 0 dash |
| 5 | `demo-dash.json` | Dash (`EX-GP-017`), couloir bas et fosses garnies de pics | Niveau entier | 0 saut |
| 6 | `demo-mouvement.json` | **Synthèse d'acte** : dash, double saut, wall jump enchaînés | **Suivi** | 14 sauts, 2 dashs |
| 7 | `demo-interrupteur.json` | Interrupteur ↔ porte (`EX-GP-020`), alcôves du plafond — skins `kenney` | Niveau entier | — |
| 8 | `demo-plaque-pression.json` | Plaque de pression (`EX-GP-025`) : un **bloc** y tient la porte ouverte | Niveau entier | — |
| 9 | `demo-cle.json` | Clé ↔ porte verrouillée (`EX-GP-023`), deux paires, invite « Interagir » | Niveau entier | — |
| 10 | `demo-bloc.json` | Bloc poussable (`EX-GP-022`) comblant une fosse | Niveau entier | 1 saut |
| 11 | `demo-bloc-reduit.json` | Bloc `×0.5` (`EX-GP-005`), deux fosses | Niveau entier | 2 sauts |
| 12 | `demo-bloc-quart.json` | Bloc `×0.25` (`EX-GP-005`) — texture par instance | Niveau entier | 1 saut |
| 13 | `demo-pente.json` | Pentes **et** arrondis de sol (`EX-GP-003`/`004`), montée et descente | Niveau entier | 0 dash |
| 14 | `demo-pente-gauche.json` | Variantes montant vers la **gauche** ; l'entrée est à droite | Niveau entier | 0 dash |
| 15 | `demo-concave.json` | Arrondis concaves (`EX-GP-007`), sol sur le chemin et plafond bas | Niveau entier | 0 dash |
| 16 | `demo-plafond.json` | Les quatre plafonds inclinés (`EX-GP-006`) bordant le couloir | Niveau entier | 0 dash |
| 17 | `demo-dangers-directionnels.json` | Les quatre orientations (`EX-GP-050`) **sur** le chemin | Niveau entier | — |
| 18 | `demo-dangers-avances.json` | Danger mobile, commuté, temporisé déphasé (`EX-GP-051`–`053`) | Niveau entier | — |
| 19 | `demo-plateforme.json` | Plateforme mobile (`EX-GP-026`), verticale, portant un bloc | Niveau entier | — |
| 20 | `demo-budget.json` | Budget de mouvements (`EX-GP-024`) : le trajet consomme tout | Niveau entier | 4 sauts, 2 dashs |
| 21 | `demo-synthese.json` | Mécanismes, terrain et dangers **entrelacés** | Niveau entier | 6 sauts, 1 dash |
| 22 | `demo-final.json` | **Final multi-salles** : une énigme par salle, zones de caméra dessinées | **Par salle** | 30 sauts, 4 dashs |

`demo-arrondi.json` (fusionné dans `demo-pente`) et `demo-salles.json` (absorbé par le final, qui
porte désormais le cadrage *par salle*) ont été supprimés au second temps.

Détail des défauts constatés en construisant cette séquence : voir `CHANGELOG.md`, section
*Non publié*.

### `sequence-demo.json`
Objet JSON : `version` (`EX-LVL-005`, absente = version initiale), `titleKey` (clé de traduction du
titre de la séquence, `EX-REN-033`), et `levels` — liste ordonnée de **noms de fichiers** (jamais
de chemin, jamais de `..`), résolus relativement à ce même dossier. Chargé par
`core::LevelSequenceLoader` (`Core/Levels/LevelSequence.h`), qui vérifie à la lecture que chaque
niveau référencé existe réellement à côté du fichier de séquence — une entrée fautive est une
erreur récupérable et nommée (`EX-NFR-040`), jamais un chargement hors bornes différé au premier
niveau joué.

**`sequence-` est un préfixe de nom de fichier réservé** dans ce dossier : un fichier de séquence
n'est pas un niveau (format différent), et `hmi::LevelFileOperations::list()` l'exclut
explicitement du panneau Niveaux de l'éditeur pour cette raison — l'ouvrir comme un niveau
échouerait. Ne pas nommer un niveau créé dans l'éditeur avec ce préfixe.

Réf. specs : `EX-LVL-001` (fichier externe), `EX-LVL-003` (format), `EX-LVL-004` (validation),
`EX-LVL-013` (séquence en donnée de contenu).
