# ProjectGaming

Jeu 2D de plateforme / puzzle développé **from scratch** en **C++20 / Direct3D 11**
(Windows), sans moteur tiers.

Cette documentation rassemble, en un seul endroit, les **spécifications** du
projet, les **lots de travail**, le **manuel utilisateur** et la **référence de
code** générée à partir des sources.

## En bref
- **Langage & rendu** : C++20, Direct3D 11, boucle de jeu à pas de temps fixe déterministe.
- **Architecture** : un cœur de simulation (`Core`) **indépendant** de la présentation
  (`HMI`), reposant sur un **ECS maison** ; les assets statiques vivent dans `Elements`.
- **Qualité** : build sans avertissement (`/W4 /WX`), tests unitaires et d'intégration
  (GoogleTest), documentation Doxygen et CI GitHub Actions.

## Avancement
- **LOT-01** — fenêtre Win32, init Direct3D 11 (RAII), boucle à pas fixe. *(terminé)*
- **LOT-02** — journalisation & diagnostics. *(terminé)*
- **LOT-03** — fondation ECS & mathématiques `Core`. *(terminé)*
- **LOT-04** — documentation Doxygen & réorganisation documentaire. *(en cours)*

Le détail de chaque lot (objectifs, tâches, avancement) est dans la rubrique
[Lots](@ref lots).

## Navigation
- @subpage guide — **Guide du développeur** : comprendre tout le moteur (concepts, code, maths).
- @subpage cahiertest — **Cahier de test** : tous les cas de test (catégorie, criticité, étapes).
- @subpage specifications — besoins, contraintes et exigences (`EX-…`), conventions de code.
- @subpage lots — plan de travail : un lot par incrément, découpé en tâches.
- @subpage manuel — manuel utilisateur (télécharger et lancer le jeu).
- **Référence de code** — classes, espaces de noms et fichiers de `Source/` : voir
  l'arbre de navigation (menu latéral) et les onglets *Namespaces* / *Classes*.
