# Source/Test/

Tests du projet, organisés par **niveau de test**.

| Sous-dossier | Portée |
|--------------|--------|
| `Unit/` | Tests unitaires : une fonction / une classe isolée (`../Core/`, `../HMI/`), sans dépendances externes. |
| `Integration/` | Tests d'intégration : interaction entre plusieurs modules (ex. Core ↔ Elements, Core ↔ HMI). |
| `Systeme/` | Tests système : le jeu complet, bout en bout (boucle, rendu DirectX, entrées, niveaux). |

## Convention
- Un fichier de test par module testé, nommé d'après la cible (ex. `test_physique.cpp`).
- Les tests référencent les specs (`../../Specification/`) et les critères d'acceptation des lots (`../../Lot/`).
