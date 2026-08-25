# HMI/Ai/

Le pont entre l'écran **Mode IA** (`../Interface/AiModeScreen`) et le solveur
(`../../AiSolver/`). Ce dossier ne contient **aucune** règle d'apprentissage : il fait tourner le
solveur ailleurs que sur le fil d'interface, et rapporte ce qu'il produit.

- `TrainingWorker` — exécute un entraînement sur un `QThread` séparé, pour que l'onglet
  Entraînement reste réactif pendant un run potentiellement long. L'arrêt demandé par
  l'utilisateur passe par un `std::atomic<bool>` relu entre deux générations/épisodes : on ne tue
  jamais le fil, on lui demande de s'arrêter à une frontière propre. Émet les statistiques au fur et
  à mesure, pour que le tableau se remplisse pendant le run et non à la fin.
- `EvaluationHelper` — évaluation d'un modèle déjà entraîné depuis l'onglet **Validation &
  sauvegarde** : compose `aisolver::eval::BenchmarkRunner` sans rien réimplémenter.

Réf. specs : `EX-IA-022`, lot [`LOT-ANNEXE-21`](Documentation/Lot-Annexe/LOT-ANNEXE-21-mode-ia-ihm/epic.md) ;
guide [`guide-annexe`](../../../Documentation/Guide-Annexe/guide-annexe.md).
