# Architecture (décisions dimensionnantes) {#spec-architecture}

> Statut : **brouillon**. Décisions **structurantes**, chères à modifier après coup : tout lot doit les respecter. Transverse à toutes les specs.

## 1. Modules & dépendances
- `Core` (simulation, indépendant du système), `HMI` (fenêtre, rendu, entrées, éditeur), `Elements` (assets), `Test`.
- **EX-ARCH-001** — Le sens des dépendances est `HMI → Core`, jamais l'inverse. `Core` est testable sans fenêtre ni GPU.

## 2. Modèle d'entités : ECS
Choix retenu : **ECS complet**, hébergé dans `Core`. Assumé plus lourd, justifié par le grand nombre de types d'objets manipulables visés (dont décors dynamiques, cf. §11).

- **EX-ARCH-010** — La simulation repose sur un **ECS** (entités = identifiants, composants = données pures, systèmes = logique).
- **EX-ARCH-011** — Les **composants ne contiennent pas de logique** ; la logique vit dans les **systèmes**.
- **EX-ARCH-012** — Le rendu (`HMI`) **lit** les composants (ex. `Transform`, `Sprite`) sans les muter.
- Composants pressentis (indicatif) : `Transform` (position/échelle/rotation en unités monde), `Velocity`, `Collider`, `Sprite`, `Animation`, `Layer`.
- Systèmes pressentis : mouvement/physique, collisions, animation (`Core`) ; rendu (lecture, `HMI`).

## 3. Coordonnées & unités — trois espaces distincts
| Espace | Unité | Rôle |
|--------|-------|------|
| **Monde** | tuile (float) | Position logique des objets et décors, indépendante de l'écran |
| **Art / texel** | pixel natif de l'asset | Résolution du pixel art |
| **Écran** | pixel d'affichage | Dépend de la résolution / du zoom |

- **EX-ARCH-020** — Unité monde = **1 tuile**, positions en **float**, origine **haut-gauche**, axe **Y vers le bas**.
- **EX-ARCH-021** — Un facteur **pixels-par-unité** (16) régit la conversion monde → écran ; les conversions sont centralisées (pas de constantes éparpillées).
- **EX-ARCH-022** — Rendu **pixel art** : échantillonnage *nearest-neighbor*, zoom caméra de préférence en **facteurs entiers** (netteté).
- La grille de tuiles (gameplay/collisions) et les décors libres **coexistent** dans le même espace monde ; les décors ne sont pas calés sur la grille.

## 4. Frontière simulation ↔ rendu
- **EX-ARCH-030** — `Core` met à jour la simulation à **pas de temps fixe** ; `HMI` produit l'image en **lisant** l'état.
- **EX-ARCH-031** — Un **facteur d'interpolation** `[0,1]` entre le pas précédent et le pas courant est fourni au rendu pour lisser le mouvement (prévu dès le départ).

## 5. Mathématiques dans `Core`
- **EX-ARCH-040** — `Core` définit **ses propres types** mathématiques (`Vector2`, `Rect`, …), **sans dépendance DirectX**. La conversion vers `DirectXMath` a lieu uniquement à la frontière de rendu (`HMI`).

## 6. Abstraction de rendu
- **EX-ARCH-050** — Le rendu est un **wrapper mince** au-dessus de Direct3D 11 (sprite batch). Pas de couche d'abstraction multi-backend (DirectX-only est acté).

## 7. Modèle de threading
- **EX-ARCH-060** — Boucle **mono-thread** au MVP. Un éventuel chargement asynchrone sera isolé plus tard, sans remettre en cause la simulation déterministe.

## 8. Communication inter-systèmes
- **EX-ARCH-070** — Communication par **appels directs / observateur simple**. Pas de bus d'événements tant que le couplage reste faible (réévalué si nécessaire).

## 9. Gestion des ressources
- **EX-ARCH-080** — Un `ResourceManager` simple gère les ressources (textures, niveaux, décors) par **handle**, chargement à la demande. Pas de hot-reload au MVP.

## 10. Contrainte « éditeur intégré »
- **EX-ARCH-090** — Le modèle de niveau **et les décors** constituent un **état ECS mutable et sérialisable** ; le rendu de `HMI` est utilisable **hors mode jeu** ; les états de jeu incluent un état **Éditeur**. (Respecté tôt = cheap ; rajouté tard = cher.)

## 11. Décors dynamiques (accommodation dimensionnante)
Les décors (cf. [`decors.md`](decors.md)) sont manipulables **à la conception (éditeur)** et **à terme en jeu par le joueur** (mécanique). Conséquence structurante :

- **EX-ARCH-100** — Les décors sont des **entités de la simulation** (`Core`), et non de simples éléments de rendu, afin d'être manipulables de façon **déterministe** et **sérialisable**.

## Traçabilité
Ces décisions conditionnent tous les lots. Détail des décors et du pipeline pixel art : [`decors.md`](decors.md). Exigences non fonctionnelles associées : [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md).
