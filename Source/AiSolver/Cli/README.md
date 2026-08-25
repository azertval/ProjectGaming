# AiSolver/Cli/

L'exécutable **`aisolver-cli`** : entraîner, évaluer et exporter un rejeu depuis un terminal, sans
lancer le jeu.

- `Main.cpp` — aiguillage des trois sous-commandes.
- `Commands` — `train`, `evaluate`, `export-replay`. Ce sont des **habillages minces** autour des
  types déjà cadrés en amont (`LevelTrainingSession`, `BenchmarkRunner`, `writeReplay`) : aucune
  règle d'entraînement, d'évaluation ou d'export n'est réimplémentée ici. C'est la contrainte
  centrale du lot — deux chemins de décision divergeraient tôt ou tard.
- `TrainingConfig` — hyperparamètres **résolus** d'un run, traçables de bout en bout : un `train`
  lancé sans `--config` utilise des valeurs par défaut, mais ces valeurs sont écrites dans le CSV du
  run plutôt que laissées implicites.
- `ArgParsing` — analyse minimale d'arguments `--nom valeur`, sans dépendance tierce.

```
aisolver-cli train        --level <chemin> --algo <evo|pg|ac|avance> [--seed N] [--config <fichier>]
aisolver-cli evaluate     --model <chemin> --level <chemin> [--repetitions N]
aisolver-cli export-replay --model <chemin> --level <chemin> --output <chemin>
```

Réf. specs : `EX-IA-020`, lot [`LOT-ANNEXE-19`](Documentation/Lot-Annexe/LOT-ANNEXE-19-outillage-cli/epic.md).
