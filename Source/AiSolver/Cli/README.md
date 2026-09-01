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
- `ArgumentParsing` — analyse minimale d'arguments `--nom valeur`, sans dépendance tierce.

```
aisolver-cli train         --level <chemin> --algo <evo|pg|ac|avance> [options]
aisolver-cli evaluate      --model <chemin> --algo <...> --level <chemin> [options]
aisolver-cli export-replay --model <chemin> --algo <...> --level <chemin> --output <chemin> [--seed N]
```

Réf. specs : `EX-IA-020`, lot [`LOT-ANNEXE-19`](Documentation/Lot-Annexe/LOT-ANNEXE-19-outillage-cli/epic.md).

## Options de `train`

Priorité : défauts documentés → fichier `--config` → options individuelles ci-dessous. La
configuration résolue est écrite dans le `config.json` du run, donc un run reste reproductible sans
connaître les défauts du code de l'époque.

| Famille | Options |
|---|---|
| Commun | `--seed N`, `--config <fichier>`, `--runs-root <dossier>`, `--hidden-size N`, `--max-steps N`, `--stuck-threshold N` |
| Évolutif (`evo`) | `--population-size N`, `--mutation-rate X`, `--mutation-strength X`, `--tournament-size N`, `--max-generations N`, `--required-successes N`, `--crossover-rate X` |
| Gradient (`pg`/`ac`/`avance`) | `--episodes N`, `--learning-rate X`, `--critic-learning-rate X`, `--gamma X`, `--optimizer <sgd\|adam>`, `--batch-episodes N`, `--entropy X`, `--exploration-floor X`, `--grad-clip X`, `--action-repeat N` |
| DQN (`avance`) | `--dqn-replay-capacity N`, `--dqn-batch-size N`, `--dqn-warmup-size N`, `--dqn-update-period N`, `--dqn-target-sync-period N`, `--dqn-epsilon-start X`, `--dqn-epsilon-end X`, `--dqn-epsilon-decay N` |

`--hidden-size` mérite une mention à part : un modèle n'est rechargeable que sur la topologie qui
l'a produit. La valeur est relue dans le `config.json` déposé à côté du modèle — déplacer un
`model.bin` hors de son dossier de run le rend illisible dès qu'elle diffère du défaut.

`--max-steps` et `--stuck-threshold` valent `0` par défaut, ce qui signifie **dérivés du niveau**
(`../Env/StepBudget.h`) : la chaîne d'objectifs du niveau est mesurée case par case, mécanismes
compris, et convertie en budget. Une constante ne peut convenir aux deux bouts du catalogue —
`demo-wall-jump.json` se termine en quelques centaines de pas, `demo-final.json` en demande près de
`4 000`. Les fixer explicitement reste possible, et c'est ce que fait le harnais de benchmark quand
il veut comparer deux modèles sous exactement le même plafond.

`--critic-learning-rate` est séparé de `--learning-rate`, et bien plus élevé : la sortie du critique
doit couvrir l'amplitude des **retours** (une centaine de points), là où la politique n'a qu'à
déplacer des logits de l'ordre de l'unité.

Un run `avance` écrit, en plus du `stats.csv` commun, un `dqn_stats.csv`
(`index,replayBufferSize,epsilon`) : les deux grandeurs qui expliquent la forme d'une courbe DQN
n'ont pas de colonne dans le CSV commun aux quatre algorithmes.

## Options de `evaluate`

`--repetitions N`, `--report <fichier.csv>`, `--max-steps N`, `--seed N`,
`--decoding <argmax|stochastic>`. Les trois dernières couvrent le reste de
`eval::BenchmarkConfig` ; `argmax` reste le défaut de cette sous-commande (mesure déterministe), et
`--max-steps 0` — le défaut — dérive le budget du niveau, comme à l'entraînement. Mesurer un modèle
sous un budget plus court que celui qui l'a produit le déclarerait incapable de terminer un niveau
qu'il termine.

## Équivalence avec l'écran Mode IA

Tout ce qui est réglable ici l'est aussi dans l'écran **Mode IA** du jeu (`LOT-ANNEXE-22`), et
réciproquement. À réglages et graine identiques, les deux produisent des `config.json` identiques
champ pour champ — un désaccord de comportement entre les deux est un bug, jamais une variante
assumée.
