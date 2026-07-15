# ProjectGaming

Jeu 2D de plateforme / puzzle développé **from scratch** en **C++ / DirectX** (Windows).

## Documentation en ligne :

https://azertval.github.io/ProjectGaming/

## Organisation du dépôt

| Dossier | Rôle |
|---------|------|
| `Documentation/` | Documentation projet publiée en site **Doxygen** : `Specification/` (specs & conventions), `Lot/` (lots de travail), `Manuel/` (manuel utilisateur) et référence de code. |
| `Source/` | Code source, réparti par fonction. |

### Découpage de `Source/`

| Sous-dossier | Contenu |
|--------------|---------|
| `HMI/` | Code lié aux interfaces (rendu, menus, HUD, interactions utilisateur). |
| `Core/` | Fonctions back : logique de jeu, physique, boucle, gestion d'état, moteur. |
| `Elements/` | Assets et éléments statiques (sprites, tuiles, sons, niveaux, ressources). |

## Build

Le projet se construit **exclusivement via CMake**. Visual Studio est utilisé comme IDE grâce à son intégration CMake native (aucun `.vcxproj`/`.sln` versionné : ils sont générés dans `build/`).

### Prérequis
- Visual Studio 2022+ avec la charge de travail **« Développement Desktop en C++ »** (inclut CMake, Ninja et le compilateur MSVC).

### Depuis Visual Studio (recommandé)
1. `Fichier > Ouvrir > Dossier…` puis sélectionner la racine du dépôt.
2. VS détecte `CMakeLists.txt` et `CMakePresets.json`.
3. Choisir le preset `vs` (ou `ninja`) dans la barre d'outils, puis générer.

### En ligne de commande
```sh
cmake --preset vs      # configure (ou : ninja)
cmake --build --preset vs
ctest --preset vs      # lance les tests
```

> Reproductible sur plusieurs postes : tout est versionné sauf `build/` (local). GoogleTest est récupéré automatiquement par CMake (FetchContent).

## Statut

Projet en cours d'initialisation — arborescence, build CMake, tests et CI posés. Specs et premiers lots à venir.
