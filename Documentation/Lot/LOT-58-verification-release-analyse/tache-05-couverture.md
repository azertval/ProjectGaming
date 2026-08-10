# TACHE-05 — Couverture étendue et seuil {#lot-58-tache-05-couverture}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `.github/workflows` · **Statut :** en cours

## Contexte
L'étape `Coverage` de `ci.yml` exécute OpenCppCoverage sur **`UnitTests.exe` uniquement**, exporte
un rapport Cobertura et HTML, et l'**envoie en artefact**. Rien ne le lit, rien ne le compare, rien
n'échoue jamais à cause de lui.

Deux conséquences. D'abord, la couverture affichée est fausse par construction : les 89 tests
d'intégration et les 3 tests système — c'est-à-dire toute la physique du personnage vérifiée de bout
en bout, et la franchissabilité des niveaux livrés — n'y figurent pas. Ensuite, `EX-NFR-022` demande
que la CI exécute « build, tests **et couverture** » ; produire un chiffre que personne ne regarde
n'est pas l'exécuter.

## Travail à réaliser
- **Instrumenter les trois exécutables** : `UnitTests`, `IntegrationTests` et `SystemTests`,
  agrégés en un rapport unique (OpenCppCoverage sait fusionner plusieurs exécutions).
- **Mesurer la valeur réelle** une fois les trois inclus — elle sera différente, probablement plus
  haute, de celle qu'on croyait.
- **Poser un seuil calé sous la valeur mesurée**, avec une marge explicite, et faire **échouer** la
  CI en dessous. Le seuil est une cliquet anti-régression, pas un objectif.
- **Consigner la valeur de référence et sa date** dans le workflow, en commentaire, pour que la
  prochaine personne sache d'où vient le nombre.
- **Exclure `Source/Test`** du calcul (déjà fait via `--excluded_sources`) et vérifier que
  `External/` en est bien absent.

## Fichiers impactés
- `.github/workflows/ci.yml` — étape de couverture.
- `Documentation/Specification/exigences-non-fonctionnelles.md` — `EX-NFR-022` précise ce qui est
  mesuré et que le seuil bloque.

## Tests (obligatoires)
- Le rapport agrégé inclut des lignes couvertes par `IntegrationTests` et par `SystemTests` que
  `UnitTests` seul ne couvrait pas — vérifié sur un fichier identifié (par exemple
  `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`), sinon l'agrégation ne fonctionne pas.
- **Test négatif** : un seuil temporairement placé au-dessus de la valeur mesurée fait échouer le
  job.
- Le job reste dans une durée acceptable une fois les trois exécutables instrumentés.

## Points d'attention
- **Ne pas viser haut.** Un seuil ambitieux posé le jour de la mise en place se contourne dès la
  première PR gênée. Le rôle du chiffre est d'interdire une chute, pas de forcer une hausse : c'est
  une décision de cadrage du lot, pas un arbitrage à refaire ici.
- L'instrumentation ralentit l'exécution : `SystemTests` rejoue les quinze niveaux livrés, mesurer
  l'impact avant de conclure.
- OpenCppCoverage est installé via Chocolatey sans version épinglée dans le workflow actuel —
  l'épingler, pour la même raison que Doxygen et `clang-format`.
- La couverture de `Source/HMI` restera structurellement basse : la moitié du dossier est du widget
  Qt et du Direct3D non testable sans machine graphique. C'est attendu et documenté par
  `EX-NFR-004`/`EX-NFR-010` ; le seuil doit en tenir compte plutôt que de pousser à écrire des tests
  de façade.

## État de la mesure (LOT-58) — ⚠️ incomplet, à finir avant de clore la tâche
- **Le mécanisme est câblé** : `ci.yml` exporte `UnitTests`, `IntegrationTests` et `SystemTests` en
  binaire intermédiaire (`--export_type binary:...`), puis les fusionne (`--input_coverage`) en un
  rapport Cobertura unique ; une étape dédiée compare `line-rate` au seuil et fait échouer le job en
  dessous. `OpenCppCoverage` est épinglé (`OPENCPPCOVERAGE_VERSION: 0.9.9.0`).
- **La mesure de référence n'a PAS pu être prise localement** : l'installeur `OpenCppCoverage`
  (Inno Setup) exige une élévation UAC indisponible dans l'environnement ayant câblé ce job (la
  demande d'élévation reste bloquée sur le bureau sécurisé, sans possibilité d'y répondre). Le seuil
  posé dans `ci.yml` (`COVERAGE_THRESHOLD_PERCENT: 20`) est donc une **valeur plancher provisoire**
  choisie pour ne détecter qu'une régression grossière (rapport vide, fusion cassée) — **pas** le
  cliquet calé sous la valeur réellement mesurée que ce lot doit livrer.
- **Reste à faire avant de considérer cette tâche terminée** : sur la première exécution réelle du
  job `build-test-coverage` en CI, relever le pourcentage agrégé affiché (`Write-Host`), l'écrire ici
  avec sa date, resserrer `COVERAGE_THRESHOLD_PERCENT` en conséquence (marge explicite sous la
  valeur mesurée), et confirmer sur le rapport HTML que `Source/Core/Ecs/Systems/
  CharacterPhysicsSystem.cpp` (ou un fichier équivalent) porte des lignes couvertes par
  `IntegrationTests`/`SystemTests` que `UnitTests` seul ne couvrait pas — sans quoi la fusion ne
  fonctionne pas. Faire aussi la démonstration du test négatif (seuil temporairement placé
  au-dessus de la valeur mesurée) directement en CI, faute d'avoir pu la faire en local.

## Définition de fait (DoD)
- La couverture agrège les trois exécutables, un seuil documenté et daté fait échouer la CI en cas
  de chute, la démonstration du blocage est faite, l'outil de couverture est épinglé.
- **Non atteinte en totalité** : voir « État de la mesure » ci-dessus — le mécanisme est en place et
  épinglé, mais la valeur de seuil réelle (mesurée, datée) et la démonstration du blocage restent à
  faire sur la première exécution CI, faute d'avoir pu installer `OpenCppCoverage` en local.

## Exigences
Réutilise `EX-NFR-022` (CI : build, tests et couverture), `EX-NFR-020` (tests unitaires `Core`),
`EX-NFR-021` (test système de franchissabilité), `EX-NFR-004` (rendu vérifiable sans GPU),
`EX-NFR-031` (outils épinglés).
