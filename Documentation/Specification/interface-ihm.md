# Interface utilisateur (IHM) {#spec-interface-ihm}

> Statut : **brouillon**. Cadre la refonte de l'interface hors-jeu (programme `LOT-34` → `LOT-39`).
> Dépend de [`rendu-technique.md`](rendu-technique.md) et [`editeur-niveaux.md`](editeur-niveaux.md).

L'interface **hors-jeu** (menus, options, remappage, éditeur de niveaux) est distincte du **rendu
in-game**. Ce dernier reste en Direct3D 11 (`EX-REN-002`) ; l'interface, elle, repose sur un
**framework d'UI dédié (Qt)**, pour une application maintenable, à fenêtres réglables, remplaçant
l'UI « maison » dessinée quad par quad. `Core` demeure indépendant de la présentation
(`EX-NFR-010`).

## 1. Socle applicatif
- \anchor EX-IHM-001 **EX-IHM-001** — Toute l'**interface hors-jeu** (menus, options, remappage,
  éditeur) doit reposer sur le framework **Qt** ; seul le **rendu in-game** reste en Direct3D 11.
- \anchor EX-IHM-002 **EX-IHM-002** — Le **rendu Direct3D 11 du jeu** doit être **embarqué dans un
  viewport Qt** (surface native), sans processus séparé ni duplication du pipeline de rendu ; le
  déterminisme de la simulation (`EX-NFR-002`) et la latence d'entrée (`EX-CTRL-020`/`EX-CTRL-021`)
  sont préservés.

## 2. Éditeur
- \anchor EX-IHM-010 **EX-IHM-010** — L'éditeur de niveaux doit se présenter en **fenêtre à panneaux
  dockables** (palette, outils, niveaux, liens, viewport central) : panneaux **déplaçables,
  redimensionnables, détachables**.
- \anchor EX-IHM-011 **EX-IHM-011** — La **disposition** des panneaux doit être **persistée hors
  code** (sauvegardée et restaurée entre deux sessions), et réinitialisable à une disposition par
  défaut.

## 3. Gestion des niveaux
- \anchor EX-IHM-020 **EX-IHM-020** — L'éditeur doit offrir un **panneau de gestion des niveaux**
  listant les fichiers du dossier des niveaux, avec **recherche/filtre**, restant lisible quel que
  soit le nombre de niveaux.
- \anchor EX-IHM-021 **EX-IHM-021** — Ce panneau doit permettre de **créer, renommer, dupliquer et
  supprimer** un niveau, avec **validation de nom** (`EX-EDIT-006`) et **confirmation** des actions
  destructrices ; aucune modification non enregistrée ne doit être perdue silencieusement.

## 4. Liens de mécanismes
- \anchor EX-IHM-030 **EX-IHM-030** — Les **liaisons** déclencheur → cible (interrupteur/plaque →
  porte, déclencheur → danger commuté, `EX-EDIT-003`) doivent être rendues par des **traits/flèches
  explicites** dans le viewport, en remplacement de l'indication par teinte de case.
- \anchor EX-IHM-031 **EX-IHM-031** — Un **panneau « Liens »** doit **lister** les liaisons du niveau,
  mettre en **surbrillance** la liaison sélectionnée et permettre d'en **supprimer**.

## 5. Menus, options, unification
- \anchor EX-IHM-040 **EX-IHM-040** — Le **menu principal**, l'écran **Options** (V-Sync `EX-REN-022`,
  langue `EX-REN-033`) et les écrans de **remappage** (jeu, éditeur `EX-CTRL-012`, manette) doivent
  être fournis en Qt, fonctionnellement équivalents aux écrans historiques.
- \anchor EX-IHM-041 **EX-IHM-041** — L'interface hors-jeu doit reposer sur **une seule technologie
  d'UI** : la pile d'UI « maison » (écrans dessinés au `SpriteBatch`, gestion d'écrans dédiée, fenêtre
  Win32 propre) est **retirée** une fois la parité atteinte.

## Traçabilité
Tout ceci relève de `Source/HMI` (rendu de jeu réutilisé) et d'une couche applicative Qt
(`Source/Editor`). La logique testable (édition, validation, remappage) reste découplée de l'UI et
couverte par des tests (`EX-NFR-010`, `EX-NFR-020`). Détail du séquencement : lots
[`LOT-34`](@ref lot-34) à [`LOT-39`](@ref lot-39).
