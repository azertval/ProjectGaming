# Contrôles & entrées {#spec-controles}

> Statut : **brouillon**. Dépend de [`gameplay.md`](gameplay.md).

## 1. Périphériques
- \anchor EX-CTRL-001 **EX-CTRL-001** — Le jeu doit être jouable **entièrement au clavier**.
- \anchor EX-CTRL-002 **EX-CTRL-002** (⚠️ souhaité) — Le jeu devrait supporter une **manette** (XInput).

## 2. Actions du jeu (mapping logique)
Les entrées sont traduites en **actions logiques** (pas de code métier lié à une touche physique), pour rendre le remappage et la manette possibles.

| Action logique | Clavier (défaut) | Manette (défaut) |
|----------------|------------------|------------------|
| Aller à gauche | ← / Q | Stick gauche / D-pad gauche |
| Aller à droite | → / D | Stick gauche / D-pad droite |
| Sauter | Espace / W | A |
| Interagir | E | X |
| Pause | Échap | Start |
| Valider (menu) | Entrée | A |
| Retour (menu) | Échap | B |

- \anchor EX-CTRL-010 **EX-CTRL-010** — Chaque action de gameplay doit être définie comme une action logique, dissociée de la touche physique.
- \anchor EX-CTRL-011 **EX-CTRL-011** — L'état d'une action doit distinguer **pressée**, **maintenue** et **relâchée** dans une frame (nécessaire au *jump buffering*).
- \anchor EX-CTRL-012 **EX-CTRL-012** (⚠️ souhaité) — Le mapping devrait être **reconfigurable** (au minimum via un fichier de configuration).

## 3. Réactivité
- \anchor EX-CTRL-020 **EX-CTRL-020** — La latence entrée → action ne doit pas dépasser **une frame** de simulation.
- \anchor EX-CTRL-021 **EX-CTRL-021** — La lecture des entrées doit être échantillonnée une fois par frame, en amont de la mise à jour de la logique.

## Traçabilité
Le module d'entrées relève de `Source/HMI` (acquisition) mais expose un état d'actions consommé par `Source/Core` (logique), sans dépendance inverse. Voir [`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md) pour l'architecture.
