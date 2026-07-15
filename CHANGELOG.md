# Changelog

Toutes les évolutions notables du projet sont consignées ici.
Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) ;
le projet suit le [versionnage sémantique](https://semver.org/lang/fr/).

## [Non publié]

### Ajouté
- **LOT-04 — Documentation Doxygen (terminé)** : le site Doxygen est désormais une documentation **navigable** — page d'accueil (`index.md`), rubriques **Spécifications**, **Lots** et **Manuel utilisateur** (page « Télécharger et lancer »), en plus de la référence de code. Navigation hiérarchique (`@page`/`@subpage`) ; l'ordre des spécifications est porté par l'index (pas par des préfixes de fichiers). Garde-fou qualité `WARN_AS_ERROR` sur la génération. **Traçabilité des exigences** : chaque `EX-…` est une ancre Doxygen (`@ref`), avec un lint CI (`scripts/lint_exigences.py`) garantissant l'unicité des identifiants et l'absence de référence orpheline (mode `--next` pour le prochain numéro libre).

### Modifié
- **LOT-04 (TACHE-01)** : réorganisation de l'arborescence documentaire — `Specification/` et `Lot/` déplacés sous `Documentation/` ; le guide de conventions rejoint les spécifications (`Documentation/Specification/conventions.md`) ; les fichiers de spécification perdent leur préfixe numérique (`00-vision.md` → `vision.md`, …), l'ordre étant désormais porté par la navigation Doxygen. Références mises à jour dans tout le dépôt.

### Ajouté
- **Tests d'intégration** : première cible `IntegrationTests` (niveau *Integration*) — la démonstration de mouvement de l'ECS est reclassée depuis les tests unitaires (`test_ecs_mouvement.cpp`), et un test inter-lots vérifie que le cadenceur à pas fixe `FixedTimestep` (LOT-01) pilote correctement la simulation ECS (LOT-03) sans dérive et de façon déterministe (`test_boucle_simulation.cpp`).
- **LOT-03 (TACHE-06)** : premier composant réel et système de démonstration, bouclant l'ECS de bout en bout — composants données pures `Transform` (position, échelle, rotation, en unités monde) et `Velocity` (`Core/Ecs/Components`), et `MovementSystem` (`Core/Ecs/Systems`) qui applique `position += velocity * fixedDelta` aux entités possédant les deux composants. Déterministe, couvert par tests unitaires. **LOT-03 (fondation ECS & math `Core`) terminé.**
- **LOT-03 (TACHE-05)** : façade `World` et systèmes dans `Core/Ecs` — `World` possède les entités (`EntityManager`), les pools de composants (une par type, créée à la demande, via l'interface `IComponentPool`) et les systèmes ; API composants (`addComponent` / `getComponent` / `hasComponent` / `removeComponent`), vues (`view<...>()`) et cycle de vie (`destroyEntity` retire l'entité de **toutes** les pools). Interface `ISystem` (`update(World&, float)`) ; `World::update` exécute les systèmes **dans l'ordre d'enregistrement** au pas de temps fixe. Couvert par tests unitaires.
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
