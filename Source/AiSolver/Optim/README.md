# AiSolver/Optim/

Optimiseurs de **descente de gradient** : ils consomment les gradients accumulés par l'autodiff et
mettent à jour les paramètres d'un `nn::Network`.

- `IOptimizer` — interface commune (`step`, `zeroGrad`), pour que les boucles d'entraînement ne
  connaissent pas l'optimiseur qu'elles pilotent.
- `Sgd` — descente de gradient stochastique, avec inertie (*momentum*) optionnelle.
- `Adam` — moments d'ordre 1 et 2 avec **correction de biais** : les deux moments partent de zéro et
  sont donc biaisés vers zéro aux premiers pas, ce que la correction annule.
- `OptimizerUtils` — remise à zéro des gradients, partagée par les deux implémentations.

Réf. specs : `EX-IA-006`, lot [`LOT-ANNEXE-04`](Documentation/Lot-Annexe/LOT-ANNEXE-04-optimiseurs/epic.md).
