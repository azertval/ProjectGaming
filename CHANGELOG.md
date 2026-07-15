# Changelog

Toutes les évolutions notables du projet sont consignées ici.
Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) ;
le projet suit le [versionnage sémantique](https://semver.org/lang/fr/).

## [Non publié]

### Ajouté
- **LOT-03 (TACHE-04)** : vue multi-composants dans `Core/Ecs` — `View<Components...>` itère exactement les entités possédant **tous** les composants demandés, pilotée par la plus petite pool (filtrage sur les autres). Deux formes d'usage : `for (auto [entity, ...] : view)` et `view.each([](Entity, A&, B&){ ... })`, avec accès en référence aux composants. Couvert par tests unitaires.
- **LOT-03 (TACHE-03)** : stockage de composants dans `Core/Ecs` — `ComponentPool<T>`, sparse set typé (tableau dense contigu + tableau creux indexé par entité) avec `add` / `get` / `has` / `remove` / `removeIfPresent` ; suppression par swap-and-pop préservant la densité ; `get`/`remove` sur entité absente traités par assertion de précondition. Couvert par tests unitaires.
- **LOT-03 (TACHE-02)** : entités de l'ECS dans `Core/Ecs` — `Entity` (handle générationnel `index`+`generation`, `INVALID_ENTITY`) et `EntityManager` (`create` / `destroy` / `isAlive`, recyclage des index par liste libre avec incrément de génération pour invalider les handles périmés). Couverts par tests unitaires.
- **LOT-03 (TACHE-01)** : types mathématiques de `Core` dans `Core/Math`, sans dépendance DirectX — `Vector2` (opérateurs, produit scalaire, longueur, normalisation, égalité approchée), `Rect` (bords, `contains`, `intersects`, origine haut-gauche / Y-bas) et `MathUtils.h` (`approximatelyEqual`, `kEpsilon`). Couverts par tests unitaires.
- **LOT-02** implémenté : journalisation & diagnostics dans `Core/Diagnostics` — niveaux de log, `Logger` (filtrage + sinks), `ConsoleLogSink` / `MemoryLogSink`, macros `PROJECTGAMING_LOG_*` (horodatage + fichier/ligne) et assertions `PROJECTGAMING_ASSERT` (handler surchargeable, actives en Debug). `main` journalise désormais son démarrage et ses erreurs.
- **LOT-01** implémenté : fenêtre Win32 (`hmi::Window`), initialisation Direct3D 11 en RAII (`hmi::GraphicsDevice`, effacement + présentation V-Sync + redimensionnement) et boucle de jeu à pas de temps fixe déterministe (`core::FixedTimestep`, testée). L'exécutable ouvre une fenêtre stable et se ferme proprement (croix / Échap).
- Arborescence du projet (`Specification/`, `Lot/`, `Documentation/`, `Source/`, `External/`).
- Découpage `Source/` : `Core`, `HMI`, `Elements`, `Test` (`Unit`, `Integration`, `Systeme`).
- Build **CMake** (C++20) avec presets Visual Studio et Ninja ; Visual Studio utilisé via son intégration CMake native.
- **GoogleTest** (FetchContent) et un premier test unitaire.
- **CI GitHub Actions** : configure, build et tests sur `windows-latest`.
- Documentation **Doxygen** (`Doxyfile`) et **guide de conventions** de code.
- Outillage qualité : `.clang-format`, `.clang-tidy`, `.editorconfig`, avertissements `/W4 /WX`, option AddressSanitizer.
- En-têtes précompilés (`Source/pch.h`, option `ENABLE_PCH`).
- CI : couverture de code (OpenCppCoverage, artefact Cobertura + HTML) et génération de la documentation Doxygen (artefact HTML).
- Gouvernance : `CONTRIBUTING.md` (Conventional Commits, trunk-based), `CHANGELOG.md`, `LICENSE`.
