# Contribuer à ProjectGaming

## Conventions de code
Voir [`Documentation/Specification/conventions.md`](Documentation/Specification/conventions.md). Le code doit être formaté (`clang-format`) et compiler sans avertissement avant tout commit.

## Messages de commit — Conventional Commits
Format : `<type>(<portée facultative>): <description à l'impératif>`

Types :
| Type | Usage |
|------|-------|
| `feat` | Nouvelle fonctionnalité |
| `fix` | Correction de bug |
| `docs` | Documentation seule |
| `refactor` | Refonte sans changement de comportement |
| `test` | Ajout/modification de tests |
| `build` | Build, CMake, dépendances |
| `ci` | Intégration continue |
| `chore` | Tâche diverse (config, outillage) |

Exemples :
```
feat(core): ajouter la détection de collision AABB
fix(hmi): corriger le ratio d'aspect au redimensionnement
docs(conventions): préciser la politique de gestion d'erreurs
```
La portée correspond en général au module (`core`, `hmi`, `elements`, `test`, `build`…).

## Stratégie de branches
- `main` est **protégée** : **aucun push direct**. Toute évolution passe par une **Pull Request**.
- **Une branche par lot** : `lot/LOT-XX-nom-du-lot` (ex. `lot/LOT-03-fondation-ecs`).
  - Pour un correctif isolé hors lot : `fix/...` ; pour de la doc seule : `docs/...`.
- Ouvrir une **Pull Request** vers `main`. Le merge exige une **CI verte** (build + tests + couverture, zéro avertissement).
- `main` reste **toujours compilable et testée**.

## Automatisations sur `main` (après merge d'une PR)
- **CI** (`.github/workflows/ci.yml`) : s'exécute sur chaque PR ; contrôle requis pour merger.
- **Release** (`release.yml`) : à chaque push sur `main`, republie l'exécutable Debug autonome
  dans la **Release roulante `debug-latest`** (préversion, toujours à jour). À chaque tag
  `vX.Y.Z` poussé, publie une **Release versionnée** (non préversion) avec les exécutables
  **Debug et Release**, chacun autonome — destinés aux non-développeurs (télécharger,
  décompresser, lancer).
- **Documentation** (`docs.yml`) : génère la Doxygen et la publie sur la branche **`gh-pages`** (lisible en ligne via GitHub Pages).

## Avant d'ouvrir une PR
1. `cmake --build --preset vs` compile sans avertissement.
2. `ctest --preset vs` passe à 100 %.
3. Le code est formaté (`clang-format`) et les nouveaux comportements sont couverts par des tests.
4. Le `CHANGELOG.md` (section *Unreleased*) est mis à jour si pertinent.
