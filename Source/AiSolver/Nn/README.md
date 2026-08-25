# AiSolver/Nn/

Bibliothèque de **réseaux de neurones**, construite au-dessus de l'autodiff de
[`Math/Autodiff`](../Math/README.md) : chaque couche assemble un graphe différentiable, jamais un
calcul opaque.

- `Dense` — couche dense (transformation affine `matmul(poids, entrée) + biais`).
- `Network` — composition séquentielle de couches `Dense`.
- `Activations` — les activations différentiables absentes de `Ops` : `sigmoid`, `softmax`. Le
  `softmax` est calculé de façon **numériquement stable** (soustraction du maximum).
- `WeightInit` — schémas d'initialisation Xavier/Glorot et He, choisis pour préserver la variance du
  signal d'une couche à l'autre.
- `Serialization` — format binaire **versionné** des poids. Ne stocke aucune information de
  structure au-delà des formes : reconstruire un `Network` compatible (mêmes couches, mêmes
  activations, même ordre) reste la responsabilité de l'appelant avant `loadWeights`.

Réf. specs : `EX-IA-005`, lot [`LOT-ANNEXE-03`](Documentation/Lot-Annexe/LOT-ANNEXE-03-bibliotheque-reseaux-neurones/epic.md).
