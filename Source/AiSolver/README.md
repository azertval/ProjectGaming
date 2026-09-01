# Source/AiSolver/

Solveur **IA** maison : un agent qui apprend à terminer un niveau, écrit **de zéro**, sans framework
d'apprentissage automatique, sans dépendance Python et sans inférence en temps réel dans le jeu.

Ce module dépend de `Core` (pour rejouer la physique du jeu), **jamais l'inverse** : `Core` et `HMI`
ne gagnent aucune dépendance de calcul. Ce qu'un entraînement rapporte n'est pas un réseau branché au
moteur, mais une **séquence d'actions déterministe** ([`Replay/`](Replay/README.md)) que le jeu rejoue à l'identique.

Deux points d'entrée : l'exécutable `aisolver-cli` ([`Cli/`](Cli/README.md)) et l'écran **Mode IA** de
l'application (`../HMI/Ai/`).

## Découpage

| Sous-dossier | Rôle |
|---|---|
| [`Math/`](Math/README.md) | Tenseurs, générateur aléatoire à graine, autodiff en mode inverse. |
| [`Nn/`](Nn/README.md) | Couches denses, activations, initialisation, sérialisation des poids. |
| [`Optim/`](Optim/README.md) | Optimiseurs de descente de gradient (SGD à inertie, Adam). |
| [`Env/`](Env/README.md) | Le jeu rendu jouable par une machine : simulation sans fenêtre, encodage des observations, espace d'action, récompense. |
| [`Training/`](Training/README.md) | Les quatre algorithmes d'apprentissage et la boucle niveau-par-niveau. |
| [`Eval/`](Eval/README.md) | Mesurer ce qu'un modèle entraîné vaut réellement (benchmark, robustesse, transfert). |
| [`Stats/`](Stats/README.md) | Journalisation CSV d'un entraînement. |
| [`Replay/`](Replay/README.md) | Format de rejeu versionné et sa validation à la lecture. |
| [`Cli/`](Cli/README.md) | L'exécutable `aisolver-cli` (`train`, `evaluate`, `export-replay`). |

## Invariants du module

- **Tout l'aléatoire passe par `Math/Rng`**, à graine explicite : un entraînement est reproductible
  au bit près, et c'est éprouvé par des tests de reproductibilité intégrale.
- **La physique n'est jamais réimplémentée** : `Env/HeadlessLevelEnvironment` réplique pas à pas
  l'ordre de résolution du jeu réel, et un test système permanent compare les deux trajectoires.
- **Aucune exception ne remonte** d'une entrée malformée : les erreurs sont portées par des types
  résultat (`EX-NFR-040`).

Réf. specs : `EX-IA-*` ([`ia.md`](../../Documentation/Specification/ia.md)),
programme [programme annexe](Documentation/Lot-Annexe/lots-annexe.md), guides [`guide-annexe`](../../Documentation/Guide-Annexe/guide-annexe.md).
