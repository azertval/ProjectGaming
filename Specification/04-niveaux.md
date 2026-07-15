# SPEC 04 — Niveaux & contenu

> Statut : **brouillon**. Dépend de [`01-gameplay.md`](01-gameplay.md).

## 1. Représentation des niveaux
- **EX-LVL-001** — Un niveau doit être décrit par un **fichier de données** externe (pas en dur dans le code), placé dans `Source/Elements`.
- **EX-LVL-002** — Le format doit décrire au minimum : dimensions de la grille, type de chaque tuile, position d'entrée et de sortie, et les mécanismes (interrupteurs, portes, blocs) avec leurs liaisons.
- **EX-LVL-003** — Le format doit être **lisible et modifiable à la main** (⚠️ proposition : texte, par exemple une grille de caractères + une section de métadonnées, ou JSON).
- **EX-LVL-004** — Le chargement d'un niveau doit **valider** les données (dimensions cohérentes, présence d'une entrée et d'une sortie) et signaler une erreur exploitable en cas de fichier invalide (cf. politique d'erreurs des conventions).

### ⚠️ Exemple de format texte proposé
```
# metadata
name: Tutoriel 1
width: 12
height: 8
# legend: . vide, # solide, ^ danger, E entree, S sortie, i interrupteur, D porte
############
#E.........#
#....##....#
#....##..i.#
#........###
#..^^^....D#
#........S.#
############
```

## 2. Progression
- **EX-LVL-010** — Le jeu doit charger les niveaux dans un **ordre défini** (liste ordonnée).
- **EX-LVL-011** — À la réussite d'un niveau, le jeu doit charger automatiquement le suivant ; après le dernier, revenir au menu (ou écran de fin).
- **EX-LVL-012** — Le MVP doit fournir **3 niveaux** de difficulté croissante illustrant : déplacement/saut, danger, puzzle interrupteur↔porte.

## 3. Conception (lignes directrices)
- Introduire une mécanique à la fois ; le premier niveau sert de tutoriel implicite (sans texte).
- Aucune situation sans issue (le joueur ne doit jamais être bloqué définitivement sans échec possible).
- Chaque niveau doit être **franchissable** — vérifié par un test système sur les niveaux du MVP.

## Traçabilité
Le chargement et la validation relèvent de `Source/Core` ; les fichiers de niveaux et l'atlas sont dans `Source/Elements`. Types de tuiles : [`01-gameplay.md`](01-gameplay.md).
