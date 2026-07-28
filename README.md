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
- **Éditeur de niveaux** intégré pour permettre à des non-développeurs de créer du
  contenu (peinture, mécanismes, undo/redo, essai immédiat), et **décors issus de
  photos converties en pixel art** (post-MVP).

Le *quoi* et le *pourquoi* sont décrits dans les
[spécifications](https://azertval.github.io/ProjectGaming/) ; le *comment* dans le
**Guide du développeur** et la référence de code Doxygen.

## Fonctionnalités du moteur (état actuel)

Le moteur physique est complet et **jouable** :

- **Personnage de plateforme** (silhouette humanoïde animée : repos, course, saut) :
  déplacement horizontal, **saut** avec *game feel* (hauteur variable, coyote time, jump
  buffering), **double saut**, **wall jump** + wall slide, **dash** 8 directions.
- **Gravité** asymétrique (chute plus lourde que la montée), flottement à l'apex, *fast-fall*,
  chute **newtonienne** (masse/traînée) au-delà du *game feel* de base.
- **Collisions** par **balayage continu** (swept AABB) : aucune traversée à vitesse élevée.
- **Niveaux** en tuiles typées, de taille arbitraire, chargés depuis des fichiers **JSON**, avec
  **validation**.
- **Mécanismes** interrupteur ↔ porte, **plaque de pression**, et **budget de mouvements**
  (sauts/dashs limités par tableau) pour des tableaux **puzzle**.
- **Éditeur de niveaux** intégré : peinture à la souris, outils rectangle/sélection, liaison de
  mécanismes, undo/redo, essai immédiat, guide non-codeur pour partager un niveau via Git.
- **Enchaînement de niveaux** en séquence (titre → niveaux → titre), **menu** multilingue (fr/en),
  **menu d'options** (V-Sync, langue), jouable/navigable au **clavier, à la souris et à la
  manette** (XInput).

Toute la simulation vit dans `Core` (pure, déterministe au pas fixe) et est **couverte par des
tests** (unitaires, intégration, système) — voir le **Cahier de test**.

## Organisation du dépôt

| Dossier | Rôle |
|---------|------|
| `Documentation/` | Documentation publiée en site **Doxygen** : `Specification/` (specs & conventions), `Lot/` (lots de travail), `Manuel/` (manuel utilisateur), **Guide du développeur**, **Cahier de test**, et référence de code. |
| `Source/` | Code source, réparti par fonction. |
| `.github/workflows/` | Intégration continue (voir plus bas). |

### Découpage de `Source/`

| Sous-dossier | Contenu |
|--------------|---------|
| `Core/` | Logique et moteur : ECS, mathématiques, boucle à pas fixe, diagnostics — **sans dépendance à DirectX**. |
| `HMI/` | Présentation : fenêtre Win32, rendu Direct3D 11, entrées, éditeur. Dépend de `Core`, jamais l'inverse. |
| `Elements/` | Assets et éléments statiques (sprites, tuiles, sons, niveaux). |
| `Test/` | Tests **unitaires** (`Unit/`), **d'intégration** (`Integration/`) et **système** (`Systeme/`) — GoogleTest. |

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

### Vérifications locales

Les mêmes contrôles qu'en intégration continue, tous lançables **depuis la racine du dépôt** :

```sh
python scripts/lint_exigences.py           # identifiants EX-… : ni doublon, ni orphelin
python scripts/lint_exigences.py --next    # prochain numéro libre, par catégorie
python scripts/generate_cahier_test.py --check   # cahier de test à jour
python scripts/check_demo_sequence.py      # séquence des niveaux démo cohérente
python scripts/build_docs.py               # documentation Doxygen (WARN_AS_ERROR)
```

> `build_docs.py` existe parce que Doxygen résout les chemins de son fichier de configuration
> relativement au **répertoire courant**, et non à l'emplacement du `Doxyfile` : lancer
> `doxygen Documentation/Doxyfile` depuis la racine échoue (`source '…' is not a readable file`).
> Le script se place dans `Documentation/` pour vous, quel que soit le répertoire d'appel.

**Version de Doxygen : `1.16.1`**, épinglée en CI (`EX-NFR-031`) et à installer à l'identique en
local. Les versions plus anciennes appliquent des règles de résolution de liens différentes : une
documentation générée sans avertissement avec une autre version peut échouer en CI, et la
vérification locale ne prédirait plus rien. Binaires officiels :
<https://github.com/doxygen/doxygen/releases/tag/Release_1_16_1>.

## Intégration continue

| Workflow | Déclencheur | Rôle |
|----------|-------------|------|
| **CI** (`ci.yml`) | Pull Request vers `main` | Configure, **build** et **tests** (CTest) sur `windows-2022`, **couverture** (OpenCppCoverage) et **lint des exigences**. Contrôle requis pour merger. |
| **Documentation** (`docs.yml`) | Push sur `main` | Génère la **Doxygen** (garde-fou `WARN_AS_ERROR`) et la publie sur la branche `gh-pages` (site en ligne). |
| **Release Debug** (`release.yml`) | Push sur `main` | Compile un exécutable **Debug autonome** (runtime statique) et publie la release roulante **`debug-latest`** pour les non-développeurs. |
