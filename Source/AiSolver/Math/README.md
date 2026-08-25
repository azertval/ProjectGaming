# AiSolver/Math/

Fondations numériques du solveur : tout le reste du module est bâti dessus. Aucune dépendance
tierce, aucune dépendance à `Core` ni à Qt.

- `Tensor` — conteneur N-dimensionnel à formes et **pas** (*strides*) explicites, sur un tampon
  **partagé** : une vue (`view`) ne copie rien, `clone` copie. En-tête seul, générique.
- `TensorOps` — opérations élément par élément et réductions sur `Tensor<T>`.
- `Matmul` — produit matriciel et transposition, restreints au rang 2. C'est la boucle la plus
  chaude du module : elle est parcourue une fois par couche à l'avant et deux fois à l'arrière.
- `Rng` — générateur pseudo-aléatoire **déterministe** à graine explicite. Source unique de tout
  l'aléatoire du programme : c'est ce qui rend un entraînement reproductible.
- [`Autodiff/`](.) — `Node` et `Ops` : graphe de calcul construit à la volée, puis parcouru en
  **sens inverse** (tri topologique) pour accumuler les gradients. `Ops` fournit les opérations
  différentiables élémentaires au-dessus de `unaryOp`/`binaryOp`.

Réf. specs : `EX-IA-001` à `EX-IA-004`, lots [`LOT-ANNEXE-01`](Documentation/Lot-Annexe/LOT-ANNEXE-01-bibliotheque-tensorielle-rng/epic.md) et [`LOT-ANNEXE-02`](Documentation/Lot-Annexe/LOT-ANNEXE-02-moteur-autodiff/epic.md).
