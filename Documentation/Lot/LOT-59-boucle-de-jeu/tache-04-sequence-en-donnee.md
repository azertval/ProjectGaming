# TACHE-04 — Séquence de niveaux en donnée de contenu {#lot-59-tache-04-sequence-en-donnee}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/Elements/Levels`, `Source/Core/Levels` ·
**Statut :** non commencé

## Contexte
La séquence jouée est un **littéral C++** : quinze chemins énumérés dans `MainWindow::startGame`
(`Source/HMI/Interface/MainWindow.cpp`, l. 380-399). Réordonner deux tableaux, en retirer un ou en
ajouter un impose de recompiler l'application — alors que `EX-LVL-001` pose depuis le début que le
contenu d'un niveau est une **donnée externe**, jamais du code.

Cette duplication a déjà coûté un garde-fou : `scripts/check_demo_sequence.py` existe uniquement
pour vérifier que le littéral de `MainWindow.cpp` et celui de `test_parcours_complet.cpp` ne
divergent pas. Le script reste utile ; ce qu'il compare doit changer.

## Travail à réaliser
- **Fichier de séquence** dans `Source/Elements/Levels` (JSON, comme les niveaux) : liste ordonnée
  de noms de fichiers de niveaux, avec un titre de séquence traduisible par clé.
- **Lecture dans `Core`** — c'est du contenu, pas de la présentation : mêmes garanties que
  `core::LevelLoader` (parsing **non lançant**, validation, erreur exploitable). Un niveau
  référencé mais absent est une erreur **récupérable** et nommée (`EX-NFR-040`), pas un plantage.
- **Versionner le format** comme les niveaux (`EX-LVL-005`).
- **Retirer le littéral** de `MainWindow.cpp` : plus aucun nom de fichier de niveau dans
  `Source/HMI`.
- **Adapter `scripts/check_demo_sequence.py`** : il compare désormais le fichier de séquence au test
  système, au lieu de deux littéraux C++. Sa raison d'être — qu'un niveau jouable soit toujours
  couvert par le test système — est inchangée.

## Fichiers impactés
- `Source/Elements/Levels/sequence-demo.json` (nouveau).
- `Source/Core/Levels/LevelSequence.{h,cpp}` (nouveau).
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.
- `Source/HMI/Interface/MainWindow.cpp` (retrait du littéral).
- `Source/Test/Unit/Core/Levels/test_level_sequence.cpp` (nouveau).
- `scripts/check_demo_sequence.py`.
- `Source/Elements/Levels/README.md`.

## Tests (obligatoires)
- Une séquence valide se charge dans l'ordre exact du fichier.
- Fichier absent, JSON malformé, entrée vide, niveau référencé inexistant : chacun donne une erreur
  **exploitable** et nommant le fautif, sans exception qui remonte.
- La séquence **livrée** se charge et référence des fichiers qui existent tous — c'est le test qui
  attrape une faute de frappe dans le contenu.
- `scripts/check_demo_sequence.py` échoue si l'on retire un niveau du fichier sans toucher au test
  système (vérifié en l'exécutant sur une copie modifiée).

## Points d'attention
- **Le chemin doit rester relatif au dossier de niveaux** et ne jamais permettre d'échapper à
  celui-ci : un `..` dans une entrée est une entrée invalide, pas un chemin à suivre.
- Le déploiement copie déjà `Levels/` à côté de l'exécutable (`POST_BUILD`, `Source/HMI/CMakeLists.txt`) :
  vérifier que le nouveau fichier est bien embarqué dans le zip de release, sans quoi l'application
  publiée démarre sans aucune séquence.
- `nlohmann::json::parse(json, nullptr, false)` — mode non lançant, comme partout ailleurs dans le
  projet ; `core::LevelLoader` est le modèle exact à suivre.

## Définition de fait (DoD)
- La séquence jouée provient d'un fichier de contenu validé, aucun nom de niveau ne subsiste dans
  `Source/HMI`, les fichiers manquants ou malformés sont des erreurs récupérables, le garde-fou de
  synchronisation est adapté et vérifié ; `/W4 /WX` propre.

## Exigences
`EX-LVL-013` (séquence en donnée de contenu) ; réutilise `EX-LVL-001` (contenu externe),
`EX-LVL-004` (validation), `EX-LVL-005` (version de format), `EX-LVL-010` (ordre défini),
`EX-NFR-040` (erreur récupérable), `EX-NFR-010` (`Core` sans GPU).
