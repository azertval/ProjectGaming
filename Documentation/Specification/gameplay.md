# Gameplay {#spec-gameplay}

> Statut : **brouillon**. Dépend de [`vision.md`](vision.md).

## 1. Monde en tuiles
Le niveau est une **grille de tuiles** de taille fixe : **16 × 16 px** par tuile. Chaque cellule porte un type.

| Type de tuile | Comportement |
|---------------|--------------|
| Vide | Traversable. |
| Solide | Bloque le déplacement ; supporte le personnage. |
| Danger | Provoque l'échec au contact (pics, vide mortel). |
| Entrée | Position d'apparition du personnage. |
| Sortie | Déclenche la victoire au contact. |

- **EX-GP-001** — Le niveau doit être représenté par une grille de tuiles typées.
- **EX-GP-002** — Une tuile solide doit empêcher le personnage de la traverser.

## 2. Personnage & déplacement
- **EX-GP-010** — Le personnage doit se déplacer horizontalement à vitesse constante (⚠️ ~6 tuiles/s).
- **EX-GP-011** — Le personnage doit sauter : impulsion verticale puis retombée sous gravité constante.
- **EX-GP-012** — La gravité doit s'appliquer en continu tant que le personnage n'est pas au sol.
- **EX-GP-013** — Le personnage ne doit pouvoir sauter que lorsqu'il est **au sol** (pas de double saut au MVP).
- **EX-GP-014** — Les collisions personnage ↔ tuiles solides doivent être résolues sur les deux axes (pas de traversée à vitesse élevée — collision par balayage ou pas fixes).

### Ressenti (game feel) — ⚠️ à affiner par tests
- Hauteur de saut : ~2,5 tuiles ; apex atteint en ~0,35 s.
- Tolérances de confort recommandées : *coyote time* (~80 ms) et *jump buffering* (~120 ms).

## 3. Mécanismes de puzzle
- **EX-GP-020** — Un **interrupteur** doit changer d'état quand le personnage l'active (contact ou action dédiée).
- **EX-GP-021** — Une **porte** liée à un interrupteur doit s'ouvrir/se fermer selon l'état de celui-ci.
- **EX-GP-022** — Un **bloc poussable** doit pouvoir être déplacé horizontalement par le personnage et retomber sous gravité.
- **EX-GP-023** (⚠️ optionnel MVP) — Une **clé** collectée doit ouvrir une **porte verrouillée** correspondante.

Chaque mécanisme est déterministe : à état d'entrée identique, comportement identique (facilite tests et rejouabilité).

## 4. Conditions de fin de niveau
- **EX-GP-030** — Atteindre la tuile de **sortie** termine le niveau en **succès**.
- **EX-GP-031** — Le contact avec un **danger** ou la sortie des limites basses du niveau provoque l'**échec**.
- **EX-GP-032** — En cas d'échec, le niveau doit **redémarrer** à son état initial sans quitter le jeu.

## 5. États de jeu
- **EX-GP-040** — Le jeu doit gérer des états distincts : `Menu`, `EnJeu`, `Pause`, `NiveauTermine`.
- **EX-GP-041** — Les transitions entre états doivent être explicites et unidirectionnelles à chaque événement (machine à états).

## Traçabilité
Contrôles associés : [`controles.md`](controles.md). Format des niveaux : [`niveaux.md`](niveaux.md). Ces exigences seront couvertes par des tests unitaires (`Core`) et système.
