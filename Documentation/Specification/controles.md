# Contrôles & entrées {#spec-controles}

> Statut : **brouillon**. Dépend de [`gameplay.md`](gameplay.md).

## 1. Périphériques
- \anchor EX-CTRL-001 **EX-CTRL-001** — Le jeu doit être jouable **entièrement au clavier**.
- \anchor EX-CTRL-002 **EX-CTRL-002** — Le jeu doit supporter une **manette** (XInput).

## 2. Actions du jeu (mapping logique)
Les entrées sont traduites en **actions logiques** (pas de code métier lié à une touche physique), pour rendre le remappage et la manette possibles.

| Action logique | Clavier (défaut) | Manette (défaut) |
|----------------|------------------|------------------|
| Aller à gauche | ← (remappable, `LOT-29`) | Stick gauche / D-pad gauche (remappable, `LOT-30`) |
| Aller à droite | → (remappable, `LOT-29`) | Stick gauche / D-pad droite (remappable, `LOT-30`) |
| Sauter | Espace (remappable, `LOT-29`) | A (remappable, `LOT-30`) |
| Dash (8 directions) | Maj (remappable, `LOT-29`) | Épaule droite (remappable, `LOT-30`) |
| Interagir (⚠️ souhaité, non implémenté) | E | X |
| Quitter vers le menu (jeu) | Échap | B / Start |
| Valider (menu) | Entrée | A |
| Retour (menu) | Échap | B |

- \anchor EX-CTRL-010 **EX-CTRL-010** — Chaque action de gameplay doit être définie comme une action logique, dissociée de la touche physique.
- \anchor EX-CTRL-011 **EX-CTRL-011** — L'état d'une action doit distinguer **pressée**, **maintenue** et **relâchée** dans une frame (nécessaire au *jump buffering*).
- \anchor EX-CTRL-012 **EX-CTRL-012** — Le mapping doit être **reconfigurable** (au minimum via un fichier de configuration). Implémenté pour un sous-ensemble d'actions de jeu et d'éditeur, clavier (`LOT-29`) et manette (`LOT-30`) : Options → Touches de jeu / Touches de l'éditeur / Touches de la manette, persistance dans `Settings/keybindings.json`. Les deux sources (clavier, manette) sont remappables indépendamment l'une de l'autre — remapper l'une n'affecte jamais l'autre.
- \anchor EX-CTRL-013 **EX-CTRL-013** — Le **dash** doit être une action logique dédiée (touche par défaut : **Maj**), sa **direction** étant donnée par les touches directionnelles (8 directions), à défaut par l'**orientation** courante du personnage.

## 3. Réactivité
- \anchor EX-CTRL-020 **EX-CTRL-020** — La latence entrée → action ne doit pas dépasser **une frame** de simulation.
- \anchor EX-CTRL-021 **EX-CTRL-021** — La lecture des entrées doit être échantillonnée une fois par frame, en amont de la mise à jour de la logique.

## Traçabilité
Le module d'entrées relève de `Source/HMI` (acquisition) mais expose un état d'actions consommé par `Source/Core` (logique), sans dépendance inverse. Voir [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md) pour l'architecture.
