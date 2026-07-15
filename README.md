# ProjectGaming

[![CI](https://github.com/azertval/ProjectGaming/actions/workflows/ci.yml/badge.svg)](https://github.com/azertval/ProjectGaming/actions/workflows/ci.yml)
[![Documentation](https://github.com/azertval/ProjectGaming/actions/workflows/docs.yml/badge.svg)](https://github.com/azertval/ProjectGaming/actions/workflows/docs.yml)
[![Release Debug](https://github.com/azertval/ProjectGaming/actions/workflows/release.yml/badge.svg)](https://github.com/azertval/ProjectGaming/releases/tag/debug-latest)
![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C)
![Direct3D 11](https://img.shields.io/badge/Direct3D-11-8A2BE2)

Jeu 2D de plateforme / puzzle développé **from scratch** en **C++20 / Direct3D 11**
(Windows), sans moteur tiers.

- 📖 **Documentation en ligne** : <https://azertval.github.io/ProjectGaming/>
- ⬇️ **Télécharger (version Debug)** : <https://github.com/azertval/ProjectGaming/releases/tag/debug-latest>

## Description

ProjectGaming est un moteur de jeu 2D maison. Ses partis pris :

- **Séparation stricte** entre la logique (`Core`) et la présentation (`HMI`) :
  `Core` est indépendant de DirectX et de la fenêtre, donc **testable sans GPU**.
- **ECS maison** (sparse sets) : entités = identifiants, composants = données pures,
  systèmes = logique, exécutés à **pas de temps fixe déterministe**.
- **Rendu pixel art** en Direct3D 11 (tuiles 16 px, échantillonnage *nearest*).
- **Éditeur de niveaux** intégré (à venir) pour permettre à des non-développeurs de
  créer du contenu, et **décors issus de photos converties en pixel art** (post-MVP).

Le *quoi* et le *pourquoi* sont décrits dans les
[spécifications](https://azertval.github.io/ProjectGaming/) ; le *comment* dans la
référence de code Doxygen.

## Organisation du dépôt

| Dossier | Rôle |
|---------|------|
| `Documentation/` | Documentation projet publiée en site **Doxygen** : `Specification/` (specs & conventions), `Lot/` (lots de travail), `Manuel/` (manuel utilisateur) et référence de code. |
| `Source/` | Code source, réparti par fonction. |
| `.github/workflows/` | Intégration continue (voir plus bas). |

### Découpage de `Source/`

| Sous-dossier | Contenu |
|--------------|---------|
| `Core/` | Logique et moteur : ECS, mathématiques, boucle à pas fixe, diagnostics — **sans dépendance à DirectX**. |
| `HMI/` | Présentation : fenêtre Win32, rendu Direct3D 11, entrées, éditeur. Dépend de `Core`, jamais l'inverse. |
| `Elements/` | Assets et éléments statiques (sprites, tuiles, sons, niveaux). |
| `Test/` | Tests **unitaires** (`Unit/`) et **d'intégration** (`Integration/`) — GoogleTest. |

## Build

Le projet se construit **exclusivement via CMake**. Visual Studio est utilisé comme IDE
grâce à son intégration CMake native (aucun `.vcxproj`/`.sln` versionné : ils sont
générés dans `build/`).

### Prérequis
- Visual Studio 2022+ avec la charge de travail **« Développement Desktop en C++ »**
  (inclut CMake, Ninja et le compilateur MSVC).

### Depuis Visual Studio (recommandé)
1. `Fichier > Ouvrir > Dossier…` puis sélectionner la racine du dépôt.
2. VS détecte `CMakeLists.txt` et `CMakePresets.json`.
3. Choisir le preset `vs` (ou `ninja`) dans la barre d'outils, puis générer.

### En ligne de commande
```sh
cmake --preset vs        # configure (ou : ninja)
cmake --build --preset vs
ctest --preset vs        # lance les tests
```

> Reproductible sur plusieurs postes : tout est versionné sauf `build/` (local).
> GoogleTest est récupéré automatiquement par CMake (FetchContent).

## Process d'implémentation

Le travail avance par **lots** (un incrément livrable par lot), décrits dans
`Documentation/Lot/` (un `epic.md` + des `tache-NN.md`).

- **Branches** : `main` est **protégée** (aucun push direct). Une **branche par lot**
  (`lot/LOT-XX-nom`) ; correctifs isolés en `fix/…`, documentation seule en `docs/…`.
- **Pull Requests obligatoires** vers `main`, avec **CI verte** requise pour merger.
  `main` reste toujours compilable et testée.
- **Commits** : [Conventional Commits](https://www.conventionalcommits.org/) en français
  (`feat`, `fix`, `docs`, `refactor`, `test`, `build`, `ci`, `chore`).
- **Conventions de code** : nommage, RAII, documentation Doxygen (`.h` **et** `.cpp`),
  gestion d'erreurs — voir
  [`Documentation/Specification/conventions.md`](Documentation/Specification/conventions.md).
  Le code compile **sans avertissement** (`/W4 /WX`).
- **Tests** : toute logique de `Core` est couverte par des tests (unitaires et, au besoin,
  d'intégration). `CHANGELOG.md` est tenu à jour.
- **Traçabilité** : les exigences `EX-…` (spécifications) sont des identifiants stables,
  vérifiés en CI (`scripts/lint_exigences.py`).

Détails dans [`CONTRIBUTING.md`](CONTRIBUTING.md).

## Intégration continue

| Workflow | Déclencheur | Rôle |
|----------|-------------|------|
| **CI** (`ci.yml`) | Pull Request vers `main` | Configure, **build** et **tests** (CTest) sur `windows-2022`, **couverture** (OpenCppCoverage) et **lint des exigences**. Contrôle requis pour merger. |
| **Documentation** (`docs.yml`) | Push sur `main` | Génère la **Doxygen** (garde-fou `WARN_AS_ERROR`) et la publie sur la branche `gh-pages` (site en ligne). |
| **Release Debug** (`release.yml`) | Push sur `main` | Compile un exécutable **Debug autonome** (runtime statique) et publie la release roulante **`debug-latest`** pour les non-développeurs. |

## Statut

| Lot | Objet | État |
|-----|-------|:----:|
| LOT-01 | Fenêtre Win32, init Direct3D 11 (RAII), boucle à pas fixe | ✅ Terminé |
| LOT-02 | Journalisation & diagnostics | ✅ Terminé |
| LOT-03 | Fondation ECS & mathématiques `Core` | ✅ Terminé |
| LOT-04 | Documentation Doxygen & réorganisation documentaire | ✅ Terminé |

Prochaines étapes : rendu de tuiles et chargement de niveaux (les composants *données
pures* de l'ECS sont prêts pour la sérialisation).

## Licence

Propriétaire — tous droits réservés. Voir [`LICENSE`](LICENSE).
