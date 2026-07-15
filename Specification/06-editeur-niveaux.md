# SPEC 06 — Éditeur de niveaux

> Statut : **brouillon**. Dépend de [`04-niveaux.md`](04-niveaux.md). Le point marqué ⚠️ est à valider.

## Objectif
Permettre la **création et la modification de niveaux sans écrire de code**, afin que des membres de l'équipe **non-développeurs** (game design, level design) contribuent directement au contenu du jeu.

## 1. Exigences fonctionnelles
- **EX-EDIT-001** — L'éditeur doit permettre de créer et modifier un niveau **sans compétence en programmation** ni ligne de commande.
- **EX-EDIT-002** — L'édition doit être **WYSIWYG** : une grille visuelle où l'on peint les tuiles (vide, solide, danger…) à la souris, depuis une **palette** de types.
- **EX-EDIT-003** — L'éditeur doit permettre de placer et **relier visuellement les mécanismes** (interrupteur ↔ porte, clé ↔ porte verrouillée, blocs poussables).
- **EX-EDIT-004** — L'éditeur doit permettre de définir l'**entrée** et la **sortie** du niveau.
- **EX-EDIT-005** — L'éditeur doit permettre de **redimensionner** la grille et de gérer **annuler/refaire** (undo/redo).
- **EX-EDIT-006** — L'éditeur doit **enregistrer et charger** au format hybride ASCII + JSON défini par `EX-LVL-003`, en produisant des fichiers **valides**.
- **EX-EDIT-007** — L'éditeur doit **valider** le niveau avant enregistrement (présence entrée/sortie, dimensions cohérentes, liaisons de mécanismes valides — `EX-LVL-004`) et signaler les erreurs de façon compréhensible par un non-codeur.
- **EX-EDIT-008** — L'éditeur doit permettre de **tester le niveau** immédiatement (le lancer dans le jeu depuis l'éditeur), pour un cycle création → essai rapide.

## 2. Réutilisation & cohérence
- **EX-EDIT-010** — L'éditeur doit **réutiliser le modèle de niveau et la validation de `Core`** — aucune duplication de la logique de niveau entre le jeu et l'éditeur (source unique de vérité).
- **EX-EDIT-011** — Un niveau enregistré par l'éditeur doit être **directement jouable** par le jeu sans conversion, et réciproquement (round-trip fiable).

## 3. Distribution & collaboration
- **EX-EDIT-020** — L'éditeur doit être fourni comme un **outil exécutable** que les non-codeurs lancent sans étape de build.
- **EX-EDIT-021** — Les niveaux sont des **fichiers** rangés dans `Source/Elements` et versionnés ; l'éditeur enregistre directement à cet emplacement.
- **EX-EDIT-022** — Le partage des niveaux passe par **Git via une interface graphique** (type GitHub Desktop) : les niveaux sont versionnés dans le dépôt au même titre que le reste du projet. Le level designer publie et récupère les niveaux en quelques clics, **sans ligne de commande**. Un court guide d'utilisation (installation + flux publier/mettre à jour) doit être fourni dans `Documentation/` à destination des non-codeurs.

## 4. Approche d'implémentation (décidée)
**Option retenue : éditeur intégré.** L'édition est un **mode de l'application** de jeu, réutilisant le rendu Direct3D 11 et le modèle de niveau de `Core`.

- **EX-EDIT-030** — L'éditeur est intégré à l'application (mode éditeur), et non un outil séparé.
- **EX-EDIT-031** — Le mode éditeur réutilise le **rendu D3D11** de `HMI` et le **modèle/validation de niveau** de `Core` (pas de duplication).

Justification : un seul codebase, un rendu identique au jeu, un cycle **création → essai** immédiat et un round-trip garanti avec le format de niveau. *(Repli documenté si le temps manque : l'éditeur libre Tiled avec une couche d'import vers notre format — non retenu par défaut.)*

## 5. Non-objectifs (éditeur, MVP)
- Édition collaborative en temps réel (plusieurs personnes sur le même niveau simultanément).
- Édition des assets graphiques/sonores (l'éditeur agence des tuiles existantes, il ne dessine pas les sprites).

## Traçabilité
L'éditeur s'appuie sur `Core` (modèle et validation de niveau, `04-niveaux.md`) et sur le rendu de `HMI` (`03-rendu-technique.md`). Il fera l'objet d'un lot dédié, planifié **après** le chargement de niveaux dans le moteur.
