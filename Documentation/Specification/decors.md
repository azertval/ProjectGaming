# Décors & pipeline pixel art {#spec-decors}

> Statut : **partiellement planifié**. Le **système de décors** (section 1) et son **édition**
> (section 2, à la conception) sont concrétisés par `LOT-49` et `LOT-50` du programme
> d'habillage (`LOT-40` → `LOT-55`). La **manipulation en jeu** (section 2, `EX-DEC-020/021`) et le
> **pipeline photo → pixel art** (section 3) restent **post-programme** : le socle (cf.
> [`architecture.md`](architecture.md)) doit les accommoder, la livraison n'est pas planifiée.

## Vision
Les décors sont issus de **photos réelles converties en pixel art**, plaçables et transformables. Ils sont manipulables par le **level designer** (dans l'éditeur) et, **à terme, par le joueur** (mécanique de gameplay).

## 1. Système de décors
- \anchor EX-DEC-001 **EX-DEC-001** — Un décor est un **objet libre** (non calé sur la grille de tuiles) doté d'un **transform** (position, échelle, rotation optionnelle) en unités monde.
- \anchor EX-DEC-002 **EX-DEC-002** — Les décors se superposent par **couches** (arrière-plan, décor, premier plan) — cf. `EX-REN-014`.
- \anchor EX-DEC-003 **EX-DEC-003** — Rendu **pixel art net** (nearest-neighbor) — cf. `EX-ARCH-022`.
- \anchor EX-DEC-004 **EX-DEC-004** — Les décors sont des **entités ECS de la simulation `Core`** (état sérialisable, manipulation déterministe) — cf. `EX-ARCH-100`.
- \anchor EX-DEC-005 **EX-DEC-005** — Chaque décor porte une propriété **statique** ou **manipulable en jeu** (détermine s'il participe à la mécanique joueur).
- \anchor EX-DEC-006 **EX-DEC-006** — Chaque **couche** de décor porte un **facteur de défilement**
  (parallaxe) appliqué au rendu : une couche d'arrière-plan défile moins vite que le niveau, une
  couche de premier plan plus vite, ce qui donne la profondeur. Le facteur est purement visuel
  (`EX-ARCH-012`) et son comportement au franchissement d'une frontière de **salle**
  (`EX-REN-015`, caméra à coupure nette) doit être défini explicitement plutôt que subi.
  Concrétisé en `LOT-49`.

Les décors sont **traversables** : ils ne participent jamais aux collisions. Combiné à la couche de
**premier plan** dessinée au-dessus du personnage (`EX-REN-014`), c'est le moyen de lecture qui
permet au joueur de distinguer d'un coup d'œil le décor du physique.

## 2. Manipulation

### À la conception (éditeur)
- \anchor EX-DEC-010 **EX-DEC-010** — L'éditeur permet de **placer, déplacer, redimensionner, superposer et supprimer** des décors.

### En jeu (mécanique, à terme)
- \anchor EX-DEC-020 **EX-DEC-020** — Le joueur peut **manipuler en temps réel** les décors marqués manipulables (déplacer / redimensionner).
- \anchor EX-DEC-021 **EX-DEC-021** — La manipulation en jeu reste **déterministe** (pas de temps fixe) et compatible avec une éventuelle sauvegarde/rejeu.

## 3. Pipeline photo → pixel art (intégré à l'éditeur)
- \anchor EX-DEC-030 **EX-DEC-030** — L'éditeur permet de **charger une photo** et de la **convertir en pixel art** (pixellisation, réduction de palette) — traitement **intégré à l'outil**.
- \anchor EX-DEC-031 **EX-DEC-031** — Les **paramètres de conversion** sont ajustables (taille de pixel / résolution cible, palette).
- \anchor EX-DEC-032 **EX-DEC-032** — L'image pixel art résultante est enregistrée comme **asset** dans `Source/Elements` et référencée par le décor.

> Dépendance à prévoir : une capacité de **traitement d'image** côté éditeur (bibliothèque à ajouter dans `External/`, par ex. `stb_image` pour le chargement) — décision de dépendance prise au lot correspondant.

## 4. Périmètre & séquencement
- **MVP moteur** : ni décors photo ni manipulation en jeu. **Mais** l'architecture (ECS, trois espaces de coordonnées, décors = entités) est posée pour les accueillir sans refonte.
- Ordre de livraison visé :
  1. Socle moteur (fenêtre, boucle, ECS, rendu de tuiles).
  2. Éditeur de base (édition de tuiles).
  3. Décors : placement et transform dans l'éditeur.
  4. Pipeline photo → pixel art.
  5. Manipulation des décors en jeu.

## Traçabilité
S'appuie sur [`architecture.md`](architecture.md) (ECS, coordonnées, décors = simulation), [`editeur-niveaux.md`](editeur-niveaux.md) (édition intégrée) et [`vision.md`](vision.md).
