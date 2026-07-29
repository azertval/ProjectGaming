# TACHE-01 — RNG déterministe {#lot-annexe-01-tache-01-rng-deterministe}

**Lot :** [LOT-ANNEXE-01](epic.md) · **Emplacement :** `Source/AiSolver/Math` · **Statut :** à faire

## Contexte
Première tâche du programme annexe : elle crée le module `Source/AiSolver` lui-même (aucun fichier
n'y existe encore). Avant même le conteneur tensoriel (TACHE-02), le programme a besoin d'une source
d'aléatoire déterministe partagée — initialisation des poids (`LOT-ANNEXE-03`), mutation
évolutionniste (génération 2), échantillonnage stochastique de politique (génération 3) — pour que
chaque exécution d'entraînement soit **rejouable à l'identique** à partir de sa seule graine, et
qu'aucun lot ultérieur ne réimplémente sa propre variante.

## Travail à réaliser
- **Squelette du module** : `Source/AiSolver/CMakeLists.txt`, calqué sur `Source/Core/CMakeLists.txt`
  (`add_library(AiSolver STATIC Math/Rng.cpp)`, `target_include_directories(AiSolver PUBLIC
  ${PROJECT_SOURCE_DIR}/Source)`, `target_link_libraries(AiSolver PRIVATE Core project_warnings
  project_options)`). Ajout de `add_subdirectory(AiSolver)` dans `Source/CMakeLists.txt`, après
  `add_subdirectory(HMI)`, avec une ligne de commentaire au même endroit que celles décrivant
  `Core`/`HMI`/`Elements`.
- **`aisolver::Rng`** (`Source/AiSolver/Math/Rng.h`) : classe non gabarit, RAII, membre privé
  `std::mt19937_64 _engine;`.
  - Constructeur `explicit Rng(std::uint64_t seed);` — pas de constructeur par défaut (une graine
    doit toujours être fournie explicitement, jamais dérivée de l'horloge).
  - `float nextFloat();` — flottant uniforme dans `[0, 1)`.
  - `float nextFloat(float min, float max);` — flottant uniforme dans `[min, max)`.
  - `float nextGaussian(float mean = 0.0f, float stddev = 1.0f);` — loi normale, via
    `std::normal_distribution<float>` piloté par `_engine`.
  - `int nextInt(int min, int max);` — entier uniforme dans `[min, max]` (bornes **incluses**), via
    `std::uniform_int_distribution<int>`.
- **`Source/AiSolver/Math/Rng.cpp`** : implémentation. Chaque méthode construit une distribution
  standard **locale** (pas de membre de distribution persistant) et l'applique à `_engine` — évite
  tout état de distribution à synchroniser entre appels de méthodes différentes sur le même moteur.

## Fichiers impactés
- `Source/AiSolver/CMakeLists.txt` (nouveau).
- `Source/CMakeLists.txt` (ajout de `add_subdirectory(AiSolver)`).
- `Source/AiSolver/Math/Rng.h` (nouveau).
- `Source/AiSolver/Math/Rng.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_rng.cpp` à `UnitTests`, ajout de
  `AiSolver` à `target_link_libraries(UnitTests PRIVATE …)`).
- `Source/Test/Unit/AiSolver/Math/test_rng.cpp` (nouveau, cf. TACHE-05).

## Tests (obligatoires)
- **Reproductibilité à seed fixée** : deux instances de `Rng` construites avec la **même** graine
  produisent exactement la **même** séquence de sorties, sur les quatre méthodes.
- **Deux graines différentes divergent** : deux instances à graines différentes produisent des
  séquences différentes (test de non-trivialité, pas de statistique poussée).
- **Bornes respectées** : `nextFloat(min, max)` reste dans `[min, max)` et `nextInt(min, max)` dans
  `[min, max]` sur un grand nombre de tirages.
- **`nextGaussian` moyenne/écart-type plausibles** : sur un grand nombre de tirages, la moyenne et
  l'écart-type empiriques restent proches (tolérance large) des paramètres demandés — test
  statistique **faible**, pas une preuve, juste un garde-fou contre une implémentation inversée.

## Points d'attention
- **Pas de graine par défaut basée sur l'horloge** : casserait la reproductibilité qui est la raison
  d'être de cette classe (rejouabilité d'un entraînement, comparaison de deux runs).
- **Ne pas garder de distribution comme membre persistant** : `std::uniform_real_distribution` et
  consorts n'ont pas d'état à préserver entre appels dans l'usage prévu ici (contrairement à
  `std::mt19937_64` lui-même, dont l'état **doit** persister) ; les recréer localement à chaque
  appel simplifie l'API sans coût significatif.
- **`std::mt19937_64` n'est pas cryptographiquement sûr** — sans importance ici (aucun usage
  sécuritaire), mais à ne pas réutiliser hors de ce contexte si un jour un besoin cryptographique
  apparaissait ailleurs dans le projet.

## Définition de fait (DoD)
- `Source/AiSolver` compile comme bibliothèque CMake indépendante, liée à `Core`. `aisolver::Rng`
  disponible et testé (`ctest` vert) ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  `EX-IA-001` déclarée (au niveau de l'épic, cette tâche n'introduit pas d'exigence propre).

## Exigences
Contribue à `EX-IA-001` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
