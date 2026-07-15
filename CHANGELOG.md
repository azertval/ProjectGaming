# Changelog

Toutes les évolutions notables du projet sont consignées ici.
Format inspiré de [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) ;
le projet suit le [versionnage sémantique](https://semver.org/lang/fr/).

## [Non publié]

### Ajouté
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
