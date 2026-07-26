# External/

Dépendances **tierces** du projet.

## Stratégies
- **FetchContent** : la dépendance est récupérée automatiquement à la configuration CMake (cas de **GoogleTest**, voir `CMakeLists.txt`). Rien n'est versionné dans le dépôt.
- **Vendored** : une bibliothèque copiée ici, dans son propre sous-dossier, puis intégrée via `add_subdirectory()` dans `CMakeLists.txt`.

- **Provisionné (hors dépôt)** : SDK trop volumineux pour FetchContent (ex. **Qt**), installé
  séparément (poste local, runner CI) et localisé par CMake via `find_package` / `CMAKE_PREFIX_PATH`.

## Actuel
| Dépendance | Version | Mode |
|------------|---------|------|
| GoogleTest | v1.15.2 | FetchContent |
| nlohmann/json | v3.11.3 | FetchContent |
| **Qt** | **6.8.1** (`win64_msvc2022_64`) | **Provisionné** (LOT-34) |

> DirectX ne se place pas ici : il provient du **Windows SDK** et est lié directement (`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`, …) au moment du lot de rendu.

## Qt (LOT-34, refonte IHM)

Qt porte l'IHM hors-jeu (éditeur `ProjectGamingEditor`, `Source/Editor`). **Qt 6.8+** est requis pour
l'archi `win64_msvc2022_64` (Qt 6.7 n'expose que `win64_msvc2019_64`). Licence **LGPLv3, lien
dynamique** — aucune obligation de publication du source du jeu ; les DLL Qt sont redistribuées à côté
de l'exécutable (`windeployqt`).

- **Poste local** (recommandé, scriptable) :
  ```
  pip install aqtinstall
  python -m aqt install-qt windows desktop 6.8.1 win64_msvc2022_64 --outputdir C:/Qt
  ```
  puis configurer en pointant CMake sur l'installation :
  ```
  cmake --preset vs -DCMAKE_PREFIX_PATH=C:/Qt/6.8.1/msvc2022_64
  ```
  (ou installeur officiel Qt). La cible éditeur est **optionnelle** : sans Qt, la configuration
  n'échoue pas (l'exécutable historique se construit) ; forcer l'exclusion avec `-DBUILD_EDITOR_QT=OFF`.
- **CI** (`.github/workflows/ci.yml`) : étape `jurplel/install-qt-action@v4` (avec cache) avant la
  configuration ; `CMAKE_PREFIX_PATH` est renseigné depuis `QT_ROOT_DIR`.
- **Release** (`.github/workflows/release.yml`) : tant que l'exécutable livré reste l'historique
  (jusqu'au LOT-38), Qt n'y est pas installé et l'éditeur est simplement ignoré. Quand la release
  basculera sur `ProjectGamingEditor`, ajouter **`windeployqt`** pour déployer les DLL Qt à côté du
  binaire packagé.
