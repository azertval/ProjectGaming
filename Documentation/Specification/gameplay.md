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

- \anchor EX-GP-001 **EX-GP-001** — Le niveau doit être représenté par une grille de tuiles typées.
- \anchor EX-GP-002 **EX-GP-002** — Une tuile solide doit empêcher le personnage de la traverser.

## 2. Personnage & déplacement
- \anchor EX-GP-010 **EX-GP-010** — Le personnage doit se déplacer horizontalement à vitesse constante (⚠️ ~6 tuiles/s).
- \anchor EX-GP-011 **EX-GP-011** — Le personnage doit sauter : impulsion verticale puis retombée sous gravité constante.
- \anchor EX-GP-012 **EX-GP-012** — La gravité doit s'appliquer en continu tant que le personnage n'est pas au sol.
- \anchor EX-GP-013 **EX-GP-013** — Le personnage ne doit pouvoir sauter que lorsqu'il est **au sol** (pas de double saut au MVP).
- \anchor EX-GP-014 **EX-GP-014** — Les collisions personnage ↔ tuiles solides doivent être résolues sur les deux axes (pas de traversée à vitesse élevée — collision par balayage ou pas fixes).

### Mécaniques aériennes avancées (au-delà du MVP)
> Ces exigences **étendent** le MVP et **assouplissent** `EX-GP-013` (qui interdit le double saut) : non requises au MVP, elles visent un platformer aux mécaniques riches.
- \anchor EX-GP-015 **EX-GP-015** — Le personnage doit pouvoir effectuer un nombre **paramétrable** de sauts **aériens** supplémentaires (double/multi saut), **rechargés au contact du sol**.
- \anchor EX-GP-016 **EX-GP-016** — Au contact d'un mur en l'air, le personnage doit **glisser** le long de celui-ci (wall slide) ; un saut le propulse alors **en diagonale opposée** au mur (wall jump).
- \anchor EX-GP-017 **EX-GP-017** — Le personnage doit pouvoir **dasher** : une ruée directionnelle (**8 directions**) à vitesse élevée sur une **courte durée**, disponible une fois puis **rechargée au contact du sol**.
- \anchor EX-GP-018 **EX-GP-018** — Le ressenti vertical doit être affiné : **gravité de chute renforcée** (chute plus rapide que la montée), **flottement à l'apex** (gravité réduite quand la vitesse verticale est faible) et **fast-fall** (chute accélérée en maintenant « bas »). La retombée reste sous gravité **constante** (à multiplicateur près), conformément à `EX-GP-011`.
- \anchor EX-GP-019 **EX-GP-019** — Le personnage doit avoir une **masse** ; la vitesse de chute doit résulter de l'équilibre entre le **poids** (masse × gravité effective) et une **traînée** proportionnelle à la vitesse, faisant émerger une **vitesse terminale** progressive plutôt qu'un plafond arbitraire. La montée du saut n'est pas concernée (gravité simple, `EX-GP-011`).

### Ressenti (game feel) — ⚠️ à affiner par tests
- Hauteur de saut : ~2,5 tuiles ; apex atteint en ~0,35 s.
- Tolérances de confort recommandées : *coyote time* (~80 ms) et *jump buffering* (~120 ms).

## 3. Mécanismes de puzzle
- \anchor EX-GP-020 **EX-GP-020** — Un **interrupteur** doit changer d'état quand le personnage l'active (contact ou action dédiée).
- \anchor EX-GP-021 **EX-GP-021** — Une **porte** liée à un interrupteur doit s'ouvrir/se fermer selon l'état de celui-ci.
- \anchor EX-GP-022 **EX-GP-022** — Un **bloc poussable** doit pouvoir être déplacé horizontalement par le personnage et retomber sous gravité.
- \anchor EX-GP-023 **EX-GP-023** (⚠️ optionnel MVP) — Une **clé** collectée doit ouvrir une **porte verrouillée** correspondante.
- \anchor EX-GP-024 **EX-GP-024** — Un **tableau** peut **limiter** le nombre de **sauts** et/ou de **dashs** disponibles (budget de mouvements, défini par le niveau) ; à budget épuisé, l'action est **refusée**. Le budget est **réinitialisé** au (re)chargement du niveau. Contrainte de **puzzle**.
- \anchor EX-GP-025 **EX-GP-025** — Une **plaque de pression** doit maintenir la porte liée **ouverte** tant qu'un poids suffisant y repose, et la **refermer** dès qu'il en repart — activation **continue**, à la différence de l'interrupteur à bascule (`EX-GP-020`), dont le comportement n'est pas affecté.

Chaque mécanisme est déterministe : à état d'entrée identique, comportement identique (facilite tests et rejouabilité).

## 4. Conditions de fin de niveau
- \anchor EX-GP-030 **EX-GP-030** — Atteindre la tuile de **sortie** termine le niveau en **succès**.
- \anchor EX-GP-031 **EX-GP-031** — Le contact avec un **danger** ou la sortie des limites basses du niveau provoque l'**échec**.
- \anchor EX-GP-032 **EX-GP-032** — En cas d'échec, le niveau doit **redémarrer** à son état initial sans quitter le jeu.

## 5. États de jeu
- \anchor EX-GP-040 **EX-GP-040** (⚠️ partiellement implémenté) — Le jeu doit gérer des états distincts : `Menu`, `EnJeu`, `Pause`, `NiveauTermine`. En l'état (`hmi::ScreenId`) : `Menu`, `Game`, `Editor`, `Options` — pas d'état `Pause` ni `NiveauTermine` dédiés (cf. `EX-REN-031`).
- \anchor EX-GP-041 **EX-GP-041** — Les transitions entre états doivent être explicites et unidirectionnelles à chaque événement (machine à états).

## Traçabilité
Contrôles associés : [`controles.md`](controles.md). Format des niveaux : [`niveaux.md`](niveaux.md). Ces exigences seront couvertes par des tests unitaires (`Core`) et système.
