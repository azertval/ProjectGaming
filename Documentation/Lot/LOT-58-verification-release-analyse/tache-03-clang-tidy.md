# TACHE-03 — `clang-tidy` câblé, trié, puis bloquant {#lot-58-tache-03-clang-tidy}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `CMakeLists.txt`, `.clang-tidy`,
`.github/workflows` · **Statut :** non commencé

## Contexte
`.clang-tidy` est un fichier sérieux : `bugprone-*`, `cppcoreguidelines-*`, `modernize-*`,
`performance-*`, `readability-*`, et toute la table de nommage du projet (namespaces en
`lower_case`, membres privés préfixés `_`, `constexpr` en `UPPER_CASE`…). `conventions.md`
le présente comme de l'outillage « en place », et `exigences-non-fonctionnelles.md` s'appuie
dessus pour justifier que les exigences transverses sont tenues.

Il n'est exécuté **nulle part** : aucun `CXX_CLANG_TIDY` dans le CMake, aucun job CI, aucune
mention dans `CONTRIBUTING.md` parmi les étapes avant PR. Sur 61 000 lignes, il n'a jamais rien
signalé — pas parce que le code est parfait, mais parce qu'on ne le lui a jamais demandé.

## Travail à réaliser
- **Produire `compile_commands.json`** : `CMAKE_EXPORT_COMPILE_COMMANDS=ON` sur le preset Ninja
  (le générateur Visual Studio ne le produit pas).
- **Exécution ciblée sur le diff de la PR** plutôt que sur tout le dépôt : c'est ce qui rend
  l'outil utilisable dès le premier jour et empêche la dette existante de bloquer tout le monde.
- **Phase de triage, explicitement budgétée** : lancer une passe complète sur `Source/` hors
  `Source/Test`, classer les remontées par famille, corriger ce qui est trivial et sûr, et
  consigner le reste dans un état daté.
- **Politique par famille** : n'activer en **bloquant** que les familles ramenées à zéro
  (`bugprone-*` en priorité — c'est celle qui trouve des défauts, pas du style). Les autres restent
  en avertissement, avec la liste de ce qui les empêche d'être bloquantes.
- **Ajuster `.clang-tidy` si nécessaire**, en documentant chaque désactivation par sa raison — le
  fichier en donne déjà l'exemple pour `modernize-use-trailing-return-type` et les nombres magiques.
- **Ne pas analyser les dépendances** : `HeaderFilterRegex: 'Source/.*'` est déjà correct, vérifier
  qu'il tient face aux en-têtes générés par Qt (`moc_*`, `ui_*`), qui doivent être exclus.

## Fichiers impactés
- `CMakePresets.json` — export des commandes de compilation.
- `.github/workflows/ci.yml` — job d'analyse statique.
- `.clang-tidy` — ajustements documentés.
- `CONTRIBUTING.md` — l'étape existe dans la liste avant PR.
- Sources corrigées lors du triage.

## Tests (obligatoires)
- **Test négatif** : un code violant une règle d'une famille bloquante (un membre privé sans
  préfixe `_`, par exemple) fait échouer le job.
- Le job passe sur l'état de `main` après triage.
- Les en-têtes générés par Qt et le contenu de `External/` ne sont pas analysés — vérifié sur la
  sortie, pas supposé.

## Points d'attention
- **Attendre un mur au premier lancement.** `cppcoreguidelines-*` et `readability-*` sur un projet
  de cette taille produisent des centaines de remontées, dont beaucoup sont du bruit sur du code
  délibéré. Le rendre bloquant d'emblée conduit à le désactiver dans la semaine ; c'est le seul
  vrai risque de la tâche.
- **Ne pas « corriger » massivement pour faire taire l'outil.** Une correction automatique en masse
  sur du code non couvert par des tests d'IHM est un excellent moyen d'introduire une régression
  invisible juste avant une version.
- Le job tourne sur Linux (`clang-tidy` y est trivial à installer) mais le code est Windows/MSVC :
  les en-têtes Direct3D et Windows ne seront pas disponibles. Deux options — restreindre l'analyse
  aux sources sans dépendance système (celles déjà compilées dans `UnitTests`, qui sont nombreuses),
  ou exécuter sur `windows-2022` avec `clang-tidy` de l'installation LLVM. **Trancher à
  l'implémentation et documenter le choix.**
- Le triage produit des corrections mécaniques : les faire dans des commits séparés du câblage,
  pour qu'une régression reste bissectable.

## Définition de fait (DoD)
- `clang-tidy` s'exécute sur chaque PR, au moins `bugprone-*` est à zéro et bloquant, le reste est
  consigné avec sa raison, une violation est démontrée refusée, `CONTRIBUTING.md` dit la vérité.

## Exigences
`EX-NFR-024` (analyse statique vérifiée automatiquement) ; réutilise `EX-NFR-012` (conventions),
`EX-NFR-013` (sans avertissement), `EX-NFR-022` (CI).
