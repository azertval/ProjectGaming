# TACHE-05 — Couverture étendue et seuil {#lot-58-tache-05-couverture}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `.github/workflows` · **Statut :** non commencé

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

## Définition de fait (DoD)
- La couverture agrège les trois exécutables, un seuil documenté et daté fait échouer la CI en cas
  de chute, la démonstration du blocage est faite, l'outil de couverture est épinglé.

## Exigences
Réutilise `EX-NFR-022` (CI : build, tests et couverture), `EX-NFR-020` (tests unitaires `Core`),
`EX-NFR-021` (test système de franchissabilité), `EX-NFR-004` (rendu vérifiable sans GPU),
`EX-NFR-031` (outils épinglés).
