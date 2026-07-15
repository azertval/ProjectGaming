# Niveaux & contenu {#spec-niveaux}

> Statut : **brouillon**. Dépend de [`gameplay.md`](gameplay.md).

## 1. Représentation des niveaux
- \anchor EX-LVL-001 **EX-LVL-001** — Un niveau doit être décrit par un **fichier de données** externe (pas en dur dans le code), placé dans `Source/Elements`.
- \anchor EX-LVL-002 **EX-LVL-002** — Le format doit décrire au minimum : dimensions de la grille, type de chaque tuile, position d'entrée et de sortie, et les mécanismes (interrupteurs, portes, blocs) avec leurs liaisons.
- \anchor EX-LVL-003 **EX-LVL-003** — Le format retenu est **hybride** : une **grille ASCII** décrit le décor (tuiles) et une section **JSON** porte les métadonnées et les **mécanismes** (interrupteurs, portes, blocs) avec leurs liaisons. Le décor reste éditable à la main, le JSON gère les données riches et extensibles.
- \anchor EX-LVL-004 **EX-LVL-004** — Le chargement d'un niveau doit **valider** les données (dimensions cohérentes avec la grille, présence d'une entrée et d'une sortie, liaisons de mécanismes valides) et signaler une erreur exploitable en cas de fichier invalide (cf. politique d'erreurs des conventions).

### Format retenu (hybride ASCII + JSON)
Légende de la grille : `.` vide, `#` solide, `^` danger, `E` entrée, `S` sortie, `i` interrupteur, `D` porte.
```json
{
  "name": "Tutoriel 1",
  "width": 12,
  "height": 8,
  "grid": [
    "############",
    "#E.........#",
    "#....##....#",
    "#....##..i.#",
    "#........###",
    "#..^^^....D#",
    "#........S.#",
    "############"
  ],
  "mechanisms": [
    { "switch": [8, 3], "door": [10, 5] }
  ]
}
```
Les coordonnées des mécanismes sont exprimées en `[colonne, ligne]` dans le repère de la grille. Le tableau `grid` doit compter `height` lignes de `width` caractères.

## 2. Progression
- \anchor EX-LVL-010 **EX-LVL-010** — Le jeu doit charger les niveaux dans un **ordre défini** (liste ordonnée).
- \anchor EX-LVL-011 **EX-LVL-011** — À la réussite d'un niveau, le jeu doit charger automatiquement le suivant ; après le dernier, revenir au menu (ou écran de fin).
- \anchor EX-LVL-012 **EX-LVL-012** — Le MVP doit fournir **3 niveaux** de difficulté croissante illustrant : déplacement/saut, danger, puzzle interrupteur↔porte.

## 3. Conception (lignes directrices)
- Introduire une mécanique à la fois ; le premier niveau sert de tutoriel implicite (sans texte).
- Aucune situation sans issue (le joueur ne doit jamais être bloqué définitivement sans échec possible).
- Chaque niveau doit être **franchissable** — vérifié par un test système sur les niveaux du MVP.

## Traçabilité
Le chargement et la validation relèvent de `Source/Core` ; les fichiers de niveaux et l'atlas sont dans `Source/Elements`. Types de tuiles : [`gameplay.md`](gameplay.md).
