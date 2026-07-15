# External/

Dépendances **tierces** du projet.

## Stratégies
- **FetchContent** : la dépendance est récupérée automatiquement à la configuration CMake (cas de **GoogleTest**, voir `CMakeLists.txt`). Rien n'est versionné dans le dépôt.
- **Vendored** : une bibliothèque copiée ici, dans son propre sous-dossier, puis intégrée via `add_subdirectory()` dans `CMakeLists.txt`.

## Actuel
| Dépendance | Version | Mode |
|------------|---------|------|
| GoogleTest | v1.15.2 | FetchContent |

> DirectX ne se place pas ici : il provient du **Windows SDK** et est lié directement (`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`, …) au moment du lot de rendu.
