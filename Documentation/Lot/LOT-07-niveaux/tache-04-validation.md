# TACHE-04 — Validation du niveau {#lot-07-tache-04-validation}

**Lot :** [LOT-07](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** à faire

## Contexte
Un fichier de niveau syntaxiquement correct peut rester **incohérent** (grille de mauvaise
taille, sans sortie, mécanisme mal lié). Le chargement doit **valider** les données et signaler
une erreur **exploitable** (`EX-LVL-004`), sans planter (`EX-NFR-040`) — un level designer doit
comprendre ce qui ne va pas.

## Travail à réaliser
- Valider, après parsing (TACHE-03) et avant de rendre le `Level` :
  - **Dimensions cohérentes** : `grid` compte exactement `height` lignes de `width` caractères.
  - **Entrée et sortie présentes** : exactement une **entrée** (`E`) et une **sortie** (`S`)
    (unicité retenue pour ce lot).
  - **Liaisons de mécanismes valides** : chaque `switch`/`door` est **dans les bornes** et
    pointe sur la **bonne tuile** (interrupteur sur `i`, porte sur `D`).
- Renvoyer un **message d'erreur descriptif** (quel contrôle a échoué, où) via le résultat du
  chargeur.

## Fichiers impactés
- `Source/Core/Levels/LevelLoader.cpp` (ou `LevelValidator.h`/`.cpp` dédié), tests.
- `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- **Dimensions incohérentes** (ligne trop courte / trop de lignes) → rejet avec message.
- **Entrée ou sortie manquante** (ou en double) → rejet.
- **Liaison de mécanisme invalide** (hors bornes, ou pointant sur une tuile qui n'est pas
  `i`/`D`) → rejet.
- Un **niveau valide** est accepté.

## Points d'attention
- Séparer clairement **erreur de format** (TACHE-03 : JSON/caractère) et **erreur de validité**
  (cette tâche : cohérence métier) ; les deux sont récupérables et remontées de la même façon.
- Messages **compréhensibles par un non-codeur** (l'éditeur, plus tard, réutilisera cette
  validation — `EX-EDIT-007`/`EX-EDIT-010`, hors périmètre de livraison ici).
- Validation **pure** (aucune dépendance rendu) : testable sans fenêtre ni GPU.

## Définition de fait (DoD)
- Règles de validation implémentées, chaque échec testé avec un message exploitable
  (`ctest` vert) ; build `/W4 /WX`, documenté.

## Exigences
`EX-LVL-004`, `EX-NFR-040`, `EX-NFR-010`.
