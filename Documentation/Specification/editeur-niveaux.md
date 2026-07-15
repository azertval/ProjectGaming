# Éditeur de niveaux {#spec-editeur}

> Statut : **brouillon**. Dépend de [`niveaux.md`](niveaux.md). Le point marqué ⚠️ est à valider.

## Objectif
Permettre la **création et la modification de niveaux sans écrire de code**, afin que des membres de l'équipe **non-développeurs** (game design, level design) contribuent directement au contenu du jeu.

## 1. Exigences fonctionnelles
- \anchor EX-EDIT-001 **EX-EDIT-001** — L'éditeur doit permettre de créer et modifier un niveau **sans compétence en programmation** ni ligne de commande.
- \anchor EX-EDIT-002 **EX-EDIT-002** — L'édition doit être **WYSIWYG** : une grille visuelle où l'on peint les tuiles (vide, solide, danger…) à la souris, depuis une **palette** de types.
- \anchor EX-EDIT-003 **EX-EDIT-003** — L'éditeur doit permettre de placer et **relier visuellement les mécanismes** (interrupteur ↔ porte, clé ↔ porte verrouillée, blocs poussables).
- \anchor EX-EDIT-004 **EX-EDIT-004** — L'éditeur doit permettre de définir l'**entrée** et la **sortie** du niveau.
- \anchor EX-EDIT-005 **EX-EDIT-005** — L'éditeur doit permettre de **redimensionner** la grille et de gérer **annuler/refaire** (undo/redo).
- \anchor EX-EDIT-006 **EX-EDIT-006** — L'éditeur doit **enregistrer et charger** au format hybride ASCII + JSON défini par `EX-LVL-003`, en produisant des fichiers **valides**.
- \anchor EX-EDIT-007 **EX-EDIT-007** — L'éditeur doit **valider** le niveau avant enregistrement (présence entrée/sortie, dimensions cohérentes, liaisons de mécanismes valides — `EX-LVL-004`) et signaler les erreurs de façon compréhensible par un non-codeur.
- \anchor EX-EDIT-008 **EX-EDIT-008** — L'éditeur doit permettre de **tester le niveau** immédiatement (le lancer dans le jeu depuis l'éditeur), pour un cycle création → essai rapide.

## 2. Réutilisation & cohérence
- \anchor EX-EDIT-010 **EX-EDIT-010** — L'éditeur doit **réutiliser le modèle de niveau et la validation de `Core`** — aucune duplication de la logique de niveau entre le jeu et l'éditeur (source unique de vérité).
- \anchor EX-EDIT-011 **EX-EDIT-011** — Un niveau enregistré par l'éditeur doit être **directement jouable** par le jeu sans conversion, et réciproquement (round-trip fiable).

## 3. Distribution & collaboration
- \anchor EX-EDIT-020 **EX-EDIT-020** — L'éditeur doit être fourni comme un **outil exécutable** que les non-codeurs lancent sans étape de build.
- \anchor EX-EDIT-021 **EX-EDIT-021** — Les niveaux sont des **fichiers** rangés dans `Source/Elements` et versionnés ; l'éditeur enregistre directement à cet emplacement.
- \anchor EX-EDIT-022 **EX-EDIT-022** — Le partage des niveaux passe par **Git via une interface graphique** (type GitHub Desktop) : les niveaux sont versionnés dans le dépôt au même titre que le reste du projet. Le level designer publie et récupère les niveaux en quelques clics, **sans ligne de commande**. Un court guide d'utilisation (installation + flux publier/mettre à jour) doit être fourni dans `Documentation/` à destination des non-codeurs.

## 4. Approche d'implémentation (décidée)
**Option retenue : éditeur intégré.** L'édition est un **mode de l'application** de jeu, réutilisant le rendu Direct3D 11 et le modèle de niveau de `Core`.

- \anchor EX-EDIT-030 **EX-EDIT-030** — L'éditeur est intégré à l'application (mode éditeur), et non un outil séparé.
- \anchor EX-EDIT-031 **EX-EDIT-031** — Le mode éditeur réutilise le **rendu D3D11** de `HMI` et le **modèle/validation de niveau** de `Core` (pas de duplication).

Justification : un seul codebase, un rendu identique au jeu, un cycle **création → essai** immédiat et un round-trip garanti avec le format de niveau. *(Repli documenté si le temps manque : l'éditeur libre Tiled avec une couche d'import vers notre format — non retenu par défaut.)*

## 4bis. Décors & pixel art (post-MVP, intégré à l'éditeur)
- \anchor EX-EDIT-040 **EX-EDIT-040** — L'éditeur doit permettre de **placer et transformer des décors** (position, échelle, superposition par couches) — cf. [`decors.md`](decors.md).
- \anchor EX-EDIT-041 **EX-EDIT-041** — L'éditeur doit intégrer la **conversion photo → pixel art** (chargement d'une photo, pixellisation, réduction de palette, paramètres ajustables) et enregistrer l'asset résultant dans `Source/Elements` — cf. `EX-DEC-030/031/032`.

Ces capacités sont livrées **après** l'édition de tuiles de base, mais l'architecture les accommode dès le départ (cf. [`architecture.md`](architecture.md)).

## 5. Non-objectifs (éditeur, MVP)
- Édition collaborative en temps réel (plusieurs personnes sur le même niveau simultanément).
- Édition des assets graphiques/sonores (l'éditeur agence des tuiles existantes, il ne dessine pas les sprites).

## Traçabilité
L'éditeur s'appuie sur `Core` (modèle et validation de niveau, `niveaux.md`) et sur le rendu de `HMI` (`rendu-technique.md`). Il fera l'objet d'un lot dédié, planifié **après** le chargement de niveaux dans le moteur.
