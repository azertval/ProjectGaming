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
  `EX-GP-053`).
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
divergent. Un garde-fou distinct (`Source/Test/Systeme/test_couverture_mecaniques.cpp`,
`EX-LVL-015`) échoue si un type de tuile, un mode de cadrage ou une variante significative livrés
n'apparaît dans aucun tableau de cette séquence — voir `Documentation/Guide/guide-niveaux.md`.

### Inventaire des vingt-deux tableaux

Le `LOT-65` a entièrement refondu cette séquence : les dix-sept tableaux hérités du `LOT-25`
(bancs d'essai nus) ont été retirés (`TACHE-00`), habillés en les restaurant à l'identique côté
géométrie (`TACHE-02`), puis complétés de cinq tableaux couvrant les mécaniques encore absentes de
tout tableau (`TACHE-03`).

| # | Tableau | Mécanique(s) démontrée(s) | Cadrage |
|---|---|---|---|
| 1 | `demo-deplacement.json` | Déplacement, chute, sol ; premier tableau, tutoriel implicite | Niveau entier |
| 2 | `demo-saut.json` | Saut simple (`EX-GP-014`) | Niveau entier |
| 3 | `demo-double-saut.json` | Double saut (`EX-GP-015`) | Niveau entier |
| 4 | `demo-wall-jump.json` | Wall slide/wall jump (`EX-GP-016`) | Niveau entier |
| 5 | `demo-dash.json` | Dash (`EX-GP-017`) | Niveau entier |
| 6 | `demo-interrupteur.json` | Interrupteur ↔ porte (`EX-GP-020`) — skins `kenney` | Niveau entier |
| 7 | `demo-plaque-pression.json` | Plaque de pression ↔ porte (`EX-GP-025`) — skins `kenney` | Niveau entier |
| 8 | `demo-cle.json` | Clé ↔ porte verrouillée (`EX-GP-023`) — skins `kenney` | Niveau entier |
| 9 | `demo-bloc.json` | Bloc poussable, taille pleine (`EX-GP-022`) | Niveau entier |
| 10 | `demo-budget.json` | Budget de sauts borné (`EX-GP-024`) | Niveau entier |
| 11 | `demo-pente.json` | Pente/arrondi, sol, montant vers la droite (`EX-GP-003`/`004`) | Niveau entier |
| 12 | `demo-pente-gauche.json` | Pente/arrondi/concave, sol, montant vers la gauche (`LOT-65`) | Niveau entier |
| 13 | `demo-arrondi.json` | Arrondi convexe de sol (`EX-GP-004`) | Niveau entier |
| 14 | `demo-concave.json` | Arrondi concave, sol et plafond (`EX-GP-007`, `LOT-65`) | Niveau entier |
| 15 | `demo-plafond.json` | Pente/arrondi de **plafond**, quatre variantes (`EX-GP-006`, `LOT-65`) | Niveau entier |
| 16 | `demo-bloc-reduit.json` | Bloc poussable réduit `×0.5`, comble une fosse (`EX-GP-005`) | Niveau entier |
| 17 | `demo-bloc-quart.json` | Bloc poussable réduit `×0.25`, terrain plat (`LOT-65`) — texture par instance | Niveau entier |
| 18 | `demo-plateforme.json` | Plateforme mobile (`EX-GP-026`) | Niveau entier |
| 19 | `demo-dangers-avances.json` | Danger directionnel haut, mobile (vertical), commuté, temporisé déphasé (`EX-GP-050`–`053`) | Niveau entier |
| 20 | `demo-dangers-directionnels.json` | Danger directionnel bas/gauche/droite (`EX-GP-050`, `LOT-65`) | Niveau entier |
| 21 | `demo-final.json` | Synthèse : dash, pente, interrupteur/porte, double saut | **Suivi** (parcours continu le plus long) |
| 22 | `demo-salles.json` | Synthèse structurelle : deux salles, chute entre elles (`LOT-32`) | **Par salle** |

Détail des défauts constatés en construisant cette séquence (aucun n'affecte la franchissabilité
des tableaux livrés) : voir `CHANGELOG.md`, section *Non publié*.

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
