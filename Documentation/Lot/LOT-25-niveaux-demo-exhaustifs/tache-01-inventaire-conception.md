# TACHE-01 — Inventaire des mécaniques et conception {#lot-25-tache-01-inventaire-conception}

**Lot :** [LOT-25](epic.md) · **Emplacement :** `Documentation` · **Statut :** à faire

## Contexte
Avant d'écrire un seul niveau (`TACHE-02`), établir la liste complète de ce qui doit être exercé
et décider **combien** de niveaux couvrent cette liste — le nombre n'est volontairement pas fixé
avant cette tâche (voir décision de cadrage de l'épic).

## Travail à réaliser
- Dresser la liste des mécaniques jouables **par niveau** (exclut manette/menu d'options/
  localisation, transverses et déjà testées ailleurs) à partir des spécifications
  (`Documentation/Specification/gameplay.md`) et du CHANGELOG : mouvement, saut (simple, variable,
  coyote time, jump buffering), double saut, wall jump/wall slide, dash 8 directions, gravité
  asymétrique/apex/fast-fall, chute newtonienne, interrupteur↔porte, plaque de pression, bloc
  poussable, budgets de sauts/dashs, pentes (`LOT-22`), arrondi (`LOT-23`), blocs à taille
  fractionnaire (`LOT-24`).
- Pour chaque mécanique, décider si un niveau **existant** (`demo.json`…`demo5.json`) l'exerce déjà
  de façon **nécessaire**, ou si un niveau **nouveau** est requis — un tableau
  mécanique → niveau(s), consigné dans ce fichier ou dans `epic.md` une fois complété.
- Trancher le **nombre final** de niveaux et leur **ordre** (difficulté croissante, cohérent avec
  la progression déjà en place) : documenté ici avant que `TACHE-02` ne commence à produire des
  fichiers.
- Identifier les niveaux existants qui peuvent rester **inchangés**, ceux à **compléter** (ex.
  ajouter une plateforme nécessitant le double saut à un niveau déjà consacré au saut), et ceux à
  **créer** entièrement.

## Fichiers impactés
- `Documentation/Lot/LOT-25-niveaux-demo-exhaustifs/tache-01-inventaire-conception.md` (ce
  fichier, complété avec le tableau final) ou `epic.md`.

## Tests (obligatoires)
- Aucun test automatisé propre à cette tâche (travail de conception) — la validation est la
  **revue** du tableau mécanique → niveau avant de passer à `TACHE-02`.

## Points d'attention
- **Une mécanique par niveau reste un objectif, pas une règle rigide** : deux mécaniques très liées
  (ex. wall jump et wall slide, ou coyote time et jump buffering, déjà intriqués dans le
  *game feel* du saut) peuvent raisonnablement partager un seul niveau si les séparer produirait un
  niveau trivial ou artificiel.
- **Ne pas oublier `demo5.json`** dans l'inventaire — l'écart constaté avec le test système
  (absent de `test_parcours_complet.cpp`) est le symptôme déclencheur de ce lot ; s'assurer que le
  tableau final le couvre explicitement.

## Définition de fait (DoD)
- Tableau mécanique → niveau(x) complet, nombre de niveaux final tranché et documenté, revu avant
  le début de `TACHE-02`.

## Exigences
Aucune exigence propre — tâche de conception, préalable à l'implémentation.
