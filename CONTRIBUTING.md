# Contribuer à ProjectGaming

## Conventions de code
Voir [`Documentation/conventions.md`](Documentation/conventions.md). Le code doit être formaté (`clang-format`) et compiler sans avertissement avant tout commit.

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

## Stratégie de branches (trunk-based)
- `main` est **toujours dans un état compilable et testé** ; elle est protégée.
- Travailler sur des branches courtes préfixées par le type : `feat/...`, `fix/...`, `docs/...`.
- Ouvrir une **Pull Request** vers `main`. Le merge exige une **CI verte** (build + tests + zéro avertissement).
- Rattacher chaque branche/PR au lot et à la tâche concernés (`lot/LOT-XX-.../`).

## Avant d'ouvrir une PR
1. `cmake --build --preset vs` compile sans avertissement.
2. `ctest --preset vs` passe à 100 %.
3. Le code est formaté (`clang-format`) et les nouveaux comportements sont couverts par des tests.
4. Le `CHANGELOG.md` (section *Unreleased*) est mis à jour si pertinent.
