# Niveaux & contenu {#spec-niveaux}

> Statut : **brouillon**. Dépend de [`gameplay.md`](gameplay.md).

## 1. Représentation des niveaux
- \anchor EX-LVL-001 **EX-LVL-001** — Un niveau doit être décrit par un **fichier de données** externe (pas en dur dans le code), placé dans `Source/Elements`.
- \anchor EX-LVL-002 **EX-LVL-002** — Le format doit décrire au minimum : dimensions de la grille, type de chaque tuile, position d'entrée et de sortie, et les mécanismes (interrupteurs, portes, blocs) avec leurs liaisons.
- \anchor EX-LVL-003 **EX-LVL-003** — Le format retenu est un **JSON structuré orienté objets** : un niveau est un objet JSON portant ses **métadonnées** (nom, dimensions) et une **liste de tuiles**, chaque tuile étant un **objet** `{x, y, type, …}` (les cases vides sont omises) pouvant porter des **champs spécifiques** à son type (ex. liaison interrupteur↔porte par identifiant). Choisi pour un moteur **extensible et réutilisable** (données riches par tuile, sérialisation et *round-trip* d'éditeur directs), au prix d'une lisibilité « à l'œil » moindre qu'une grille ASCII — l'édition passe par l'**éditeur**, pas par le texte brut.
- \anchor EX-LVL-004 **EX-LVL-004** — Le chargement d'un niveau doit **valider** les données (positions des tuiles **dans les bornes** `width × height`, présence d'une entrée et d'une sortie, liaisons de mécanismes valides) et signaler une erreur exploitable en cas de fichier invalide (cf. politique d'erreurs des conventions).

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
`blockHalf` et `blockQuarter` (blocs poussables à taille **réduite** — `×0.5`/`×0.25` —
`EX-GP-005`, mêmes règles de poussée/chute que `block`, boîte de collision centrée et plus
petite). Une case **vide** n'est pas listée (absence = vide).
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

## 3. Conception (lignes directrices)
- Introduire une mécanique à la fois ; le premier niveau sert de tutoriel implicite (sans texte).
- Aucune situation sans issue (le joueur ne doit jamais être bloqué définitivement sans échec possible).
- Chaque niveau doit être **franchissable** — vérifié par un test système sur les niveaux du MVP.

## Traçabilité
Le chargement et la validation relèvent de `Source/Core` ; les fichiers de niveaux et l'atlas sont dans `Source/Elements`. Types de tuiles : [`gameplay.md`](gameplay.md).
