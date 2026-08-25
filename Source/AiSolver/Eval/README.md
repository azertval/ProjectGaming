# AiSolver/Eval/

Mesurer ce qu'un modèle entraîné vaut **réellement** — une récompense qui monte pendant
l'entraînement ne dit pas qu'un niveau est franchi.

- `TrainedPolicy` — adaptateur fin **uniforme** vers un modèle déjà entraîné, quel que soit
  l'algorithme qui l'a produit. Les quatre implémentations (`EvolutionaryTrainedPolicy`,
  `ReinforceTrainedPolicy`, `ActorCriticTrainedPolicy`, `AdvancedAlgorithmTrainedPolicy`) sont ce
  qui permet au reste du dossier de ne connaître qu'un seul type.
- `ActionDecodingMode` — décodage au maximum (déterministe) ou par tirage, à l'évaluation.
- `BenchmarkRunner` / `BenchmarkConfig` / `BenchmarkResult` — exécution répétée d'une politique sur
  son niveau d'origine, à graines fixées ; taux de réussite, pas moyen, variance.
- `BenchmarkReport` — rapport comparatif CSV par niveau × algorithme.
- `NoisyObservation` — décorateur d'encodeur appliquant un bruit gaussien léger : un agent qui
  s'effondre au moindre bruit a mémorisé une trajectoire, il n'a pas appris une politique.
- `ConvergenceComparator` / `GenerationComparator` — comparaison chiffrée de convergence entre
  deux séries, puis entre *N* approches, à partir des CSV de `Stats/`. Génériques : ils vivent ici
  et non dans un dossier d'algorithme, dont ils ne dépendent pas.
- `CrossLevelBenchmark` — exécution d'une politique sur un niveau **différent** de celui qui l'a
  produite (mesure de transfert). Le lot qui l'a livrée annonçait d'emblée un transfert faible : elle
  **mesure** sans chercher à l'améliorer.

> **API de bibliothèque.** `CrossLevelBenchmark` et les deux comparateurs n'ont volontairement
> **aucun point d'entrée** `aisolver-cli`, qui n'expose que `train`, `evaluate` et
> `export-replay` : la campagne de transfert du `LOT-ANNEXE-16` a été exécutée une fois et ses
> résultats consignés ([résultats de transfert](Documentation/Lot-Annexe/LOT-ANNEXE-16-evaluation-hors-niveau/resultats-transfert.md)). Le code est appelé par ses tests, et
> reste disponible pour une nouvelle campagne — ce n'est pas du code oublié.

Réf. specs : `EX-IA-016`, `EX-IA-017` ; lots [`LOT-ANNEXE-15`](Documentation/Lot-Annexe/LOT-ANNEXE-15-benchmark-multi-algorithmes/epic.md) et [`LOT-ANNEXE-16`](Documentation/Lot-Annexe/LOT-ANNEXE-16-evaluation-hors-niveau/epic.md).
