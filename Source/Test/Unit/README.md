# Source/Test/Unit/

Tests **unitaires** : une fonction ou une classe isolée, sans dépendances externes (fichiers, rendu, fenêtre).

Cible principale : la logique de `../../Core/` (physique, règles de puzzle, gestion d'état).

## Arborescence

Les tests **reflètent l'arborescence des sources** : le test de `Source/<Module>/<X>` vit sous
`Unit/<Module>/test_<x>.cpp` (p. ex. `Source/Core/Ecs/World.h` → `Unit/Core/Ecs/test_world.cpp`).
Sous-dossiers actuels : `Core/` (`Math`, `Ecs`, `Levels`, `Time`, `Diagnostics`) et `HMI/`
(`Input`, `Graphics`, `Localization`, `Interface`).
