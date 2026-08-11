# Elements/Levels/

Niveaux du jeu, un fichier **JSON** par niveau (`EX-LVL-001`, `EX-LVL-003`).

- Un niveau est un objet JSON : `name`, `width`, `height`, des budgets **optionnels**
  `jumpBudget`/`dashBudget` (`EX-GP-024`, absents = illimité), et une liste **`tiles`** d'objets
  `{ "x", "y", "type", … }`. Les cases **vides** ne sont pas listées (absence = vide).
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

## Séquence démo (`LOT-25`, en donnée de contenu depuis `LOT-59` TACHE-04)

`demo-*.json` (15 fichiers) forme la séquence jouée par le jeu (`ScreenId::Game`) : un niveau par
mécanique (ou petit groupe cohérent), ordre de difficulté croissante, terminée par
`demo-final.json`/`demo-salles.json` qui les combinent. L'**ordre joué** est celui de
`sequence-demo.json`, à côté des niveaux dans ce même dossier — pas un littéral dans
`Source/HMI` (`EX-LVL-013` : le contenu ne s'écrit jamais dans le code). `Source/Test/Systeme/
test_parcours_complet.cpp` rejoue exactement la même liste, dans le même ordre — un script CI,
`scripts/check_demo_sequence.py`, échoue si les deux divergent. Détail du tableau mécanique →
niveau : `Documentation/Lot/LOT-25-niveaux-demo-exhaustifs/tache-01-inventaire-conception.md`.

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
