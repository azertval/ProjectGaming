# SPEC 08 — Décors & pipeline pixel art

> Statut : **brouillon**. Direction produit **« à terme »** : le socle (cf. [`architecture.md`](architecture.md)) doit l'**accommoder dès maintenant**, mais la livraison est **post-MVP**.

## Vision
Les décors sont issus de **photos réelles converties en pixel art**, plaçables et transformables. Ils sont manipulables par le **level designer** (dans l'éditeur) et, **à terme, par le joueur** (mécanique de gameplay).

## 1. Système de décors
- **EX-DEC-001** — Un décor est un **objet libre** (non calé sur la grille de tuiles) doté d'un **transform** (position, échelle, rotation optionnelle) en unités monde.
- **EX-DEC-002** — Les décors se superposent par **couches** (arrière-plan, décor, premier plan) — cf. `EX-REN-014`.
- **EX-DEC-003** — Rendu **pixel art net** (nearest-neighbor) — cf. `EX-ARCH-022`.
- **EX-DEC-004** — Les décors sont des **entités ECS de la simulation `Core`** (état sérialisable, manipulation déterministe) — cf. `EX-ARCH-100`.
- **EX-DEC-005** — Chaque décor porte une propriété **statique** ou **manipulable en jeu** (détermine s'il participe à la mécanique joueur).

## 2. Manipulation

### À la conception (éditeur)
- **EX-DEC-010** — L'éditeur permet de **placer, déplacer, redimensionner, superposer et supprimer** des décors.

### En jeu (mécanique, à terme)
- **EX-DEC-020** — Le joueur peut **manipuler en temps réel** les décors marqués manipulables (déplacer / redimensionner).
- **EX-DEC-021** — La manipulation en jeu reste **déterministe** (pas de temps fixe) et compatible avec une éventuelle sauvegarde/rejeu.

## 3. Pipeline photo → pixel art (intégré à l'éditeur)
- **EX-DEC-030** — L'éditeur permet de **charger une photo** et de la **convertir en pixel art** (pixellisation, réduction de palette) — traitement **intégré à l'outil**.
- **EX-DEC-031** — Les **paramètres de conversion** sont ajustables (taille de pixel / résolution cible, palette).
- **EX-DEC-032** — L'image pixel art résultante est enregistrée comme **asset** dans `Source/Elements` et référencée par le décor.

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
